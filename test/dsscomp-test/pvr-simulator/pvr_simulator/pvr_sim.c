
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/file.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/anon_inodes.h>
#include <linux/list.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
#include <linux/types.h>
#include "../../../drivers/video/omap2/dsscomp/tiler-utils.h"
#include <asm/cacheflush.h>

#define MODULE_NAME_PVRSIM	"pvrsim"

#include <video/omapdss.h>
#include <video/dsscomp.h>
#include <plat/dsscomp.h>
#include "pvr_sim.h"

/* signal information */
static struct siginfo sinfo;
/* user program process id */
static pid_t  pid;
static struct task_struct *task;
static struct dsscomp_setup_dispc_data dispc;
static void *cookie;

static DEFINE_MUTEX(mtx);

/**
 * PVR Sim Device Driver
 *
 * @dev:   misc device base
 */
struct pvrsim_dev {
	struct miscdevice dev;
};

static void dsscomp_proxy_commandcomplete(void *cookie, int i)
{
	mutex_lock(&mtx);

	if (!pid) {
		pr_err("pvrsim: Other client to DSSCOMP has released the held Buffer\n");
		mutex_unlock(&mtx);
		return;
	}

	/* sending SIGPOLL signal to the user program */
	send_sig_info(SIGPOLL, &sinfo, task);
	mutex_unlock(&mtx);
}

static int simulate_post2(struct dsscomp_setup_dispc_data *dispc)
{
	/* physical address info for the overlays */
	struct tiler_pa_info *pas[5];
	u32 i, j;

	if (dispc->num_ovls == 0) {
		pr_err("pvrsim: must have at least one layer\n");
		return -1;
	}
	/* convert virtual addresses to physical and get tiler pa infos */
	for (i = 0; i < dispc->num_ovls; i++) {
		struct dss2_ovl_info *overlayinfo = dispc->ovls + i;
		u32 addr = (u32) overlayinfo->address;

		pas[i] = NULL;

		/* assume virtual NV12 for now */
		if (overlayinfo->cfg.color_mode == OMAP_DSS_COLOR_NV12) {
			if (overlayinfo->uv_address)
				overlayinfo->uv = tiler_virt2phys((u32) overlayinfo->uv_address);
			else
				overlayinfo->uv = tiler_virt2phys(addr + overlayinfo->cfg.height * overlayinfo->cfg.stride);
		} else {
			overlayinfo->uv = 0;
		}

		/* Get the physical address */
		overlayinfo->ba = tiler_virt2phys(addr);

		/* Get the physical pages for user-buffer*/
		if ((overlayinfo->ba < 0x60000000 || overlayinfo->ba >= 0x80000000) && overlayinfo->ba) {
			/* Flush Virtual address range --> Not needed mandotorily */
			dmac_flush_range((void *)addr, ((void *)addr + overlayinfo->cfg.height * overlayinfo->cfg.stride));
			pas[i] = user_block_to_pa(addr & PAGE_MASK, PAGE_ALIGN(overlayinfo->cfg.height * overlayinfo->cfg.stride + (addr & ~PAGE_MASK)) >> PAGE_SHIFT);
			/* Indicate to DSSCOMP the memory is non-contiguous */
			overlayinfo->uv = overlayinfo->ba;
			overlayinfo->ba = 0;
			/* Flush Physical pages */
			for (j = 0; j < pas[i]->num_pg; j++)
				outer_flush_range(pas[i]->mem[j], (pas[i]->mem[j] + (4 * 1024)));
		}
	}

	/* Get the pid of the calling process */
	if (pid == 0) {
		pid = current->pid;
		task = find_task_by_vpid(pid);
	}

	/* Queue the composition */
	dsscomp_gralloc_queue(dispc, pas, 0, dsscomp_proxy_commandcomplete, cookie);

	for (i = 0; i < dispc->num_ovls; i++)
		if (pas[i])
			tiler_pa_free(pas[i]);

	return 0;
}

static long pvrsim_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
	int r = 0;
	void __user *ptr = (void __user *)arg;

	switch (cmd) {
	case PVR_SIM_POST2:
		r = copy_from_user(&dispc, ptr, sizeof(dispc)) ? : simulate_post2(&dispc);
		break;
	case PVR_SIM_LAST_BUFFER:
		/* Resetting the pid number */
		pid = 0;
		break;
	default:
		r = -EINVAL;
		pr_err("pvrsim: undefined ioctl\n");
		break;
	}
	return r;
}

/* No private data to be filled */
static int pvrsim_open(struct inode *inode, struct file *filep)
{
	return 0;
}

static const struct file_operations pvrsim_fops = {
	.owner		= THIS_MODULE,
	.open		= pvrsim_open,
	.unlocked_ioctl = pvrsim_ioctl,
};

static int pvrsim_probe(struct platform_device *pdev)
{
	int ret;
	struct pvrsim_dev *cdev = kzalloc(sizeof(*cdev), GFP_KERNEL);
	if (!cdev) {
		pr_err("pvrsim: failed to allocate device.\n");
		return -ENOMEM;
	}
	cdev->dev.minor = MISC_DYNAMIC_MINOR;
	cdev->dev.name = "pvrsim";
	cdev->dev.mode = 0777;
	cdev->dev.fops = &pvrsim_fops;

	ret = misc_register(&cdev->dev);
	if (ret) {
		pr_err("pvrsim: failed to register misc device.\n");
		return ret;
	}

	platform_set_drvdata(pdev, cdev);

	/* init */
	memset(&sinfo, 0, sizeof(struct siginfo));
	sinfo.si_signo = SIGPOLL;
	sinfo.si_code  = POLL_MSG;

	pr_info("pvrsim: initializing.\n");

	return 0;
}

static int pvrsim_remove(struct platform_device *pdev)
{
	struct pvrsim_dev *cdev = platform_get_drvdata(pdev);
	misc_deregister(&cdev->dev);
	kfree(cdev);
	printk(KERN_INFO "Closing the pvrsim\n");
	return 0;
}

static struct platform_driver pvrsim_driver = {
	.probe = pvrsim_probe,
	.remove = pvrsim_remove,
	.driver = { .name = "pvrsim" }
};

static struct platform_device pvrsim_pdev = {
	.name = "pvrsim",
	.id = -1
};

static int __init pvrsim_init(void)
{
	int r = platform_driver_register(&pvrsim_driver);
	if (r)
		return r;

	r = platform_device_register(&pvrsim_pdev);
	if (r)
		platform_driver_unregister(&pvrsim_driver);
	return r;
}

static void __exit pvrsim_exit(void)
{
	platform_device_unregister(&pvrsim_pdev);
	platform_driver_unregister(&pvrsim_driver);
}

module_init(pvrsim_init);
module_exit(pvrsim_exit);
