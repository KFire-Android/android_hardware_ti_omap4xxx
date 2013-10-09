#include "dsscomp_test.h"

#define PAGE_MASK (~4095)
#define PAGE_ALIGN(x) (((x) + ~PAGE_MASK) & PAGE_MASK)

int display_reset(struct dsscomp_setup_mgr_data *mgr_data,
			int disp_out)
{
	int fd, ret = 0;
	struct fb_fix_screeninfo fb_fix;
	struct fb_var_screeninfo fb_var;
	char *fbptr = NULL;

	printf("Reseting the display...\n");
	fd = open("/dev/fb0", O_RDWR);
	if (fd < 0)
		fd = open("/dev/graphics/fb0", O_RDWR);
	if (fd < 0) {
       	printf("Error: display_reset: open /dev/fb failed!\n");
		return -EINVAL;
	}

	if ((ret = ioctl(fd, FBIOGET_FSCREENINFO, &fb_fix)) != 0 ||
	    (fbptr = (char *) mmap(NULL, PAGE_ALIGN((fb_fix.smem_start & ~PAGE_MASK) +
				   fb_fix.smem_len), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == (char *) -1) {
       	printf("Error: display_reset: open fb mmap failed!\n");
		close(fd);
		return -EINVAL;
	}

	fbptr += (fb_fix.smem_start & ~PAGE_MASK);

	if (ioctl(fd, FBIOGET_VSCREENINFO, &fb_var) != 0) {
       	printf("Error: display_reset: failed to get screen info\n");
		close(fd);
		return -EINVAL;
	}

	mgr_data->mode = DSSCOMP_SETUP_DISPLAY;
	mgr_data->get_sync_obj = 0;
	mgr_data->mgr.default_color = 0;
	mgr_data->mgr.ix = disp_out;
	mgr_data->num_ovls = NO_OF_PIPELINES;
	mgr_data->ovls[1].cfg.ix = VID1_PIPELINE;
	mgr_data->ovls[2].cfg.ix = VID2_PIPELINE;
	mgr_data->ovls[3].cfg.ix = VID3_PIPELINE;
	mgr_data->ovls[0].cfg.ix = GFX_PIPELINE;
	mgr_data->ovls[1].cfg.enabled = false;
	mgr_data->ovls[2].cfg.enabled = false;
	mgr_data->ovls[3].cfg.enabled = false;
	mgr_data->ovls[0].cfg.enabled = false;

	ret = comp_update(mgr_data);
	if (ret)
		printf("Error: display_reset: Reset current display "
				"- mgr.id=%d composition update failed\n", mgr_data->mgr.ix);

	mgr_data->mode = DSSCOMP_SETUP_DISPLAY;
	mgr_data->get_sync_obj = 0;
	mgr_data->mgr.default_color = 0;
	mgr_data->mgr.ix = DEFAULT_DISPLAY;
	mgr_data->mgr.trans_key_type = OMAP_DSS_COLOR_KEY_VID_SRC;
	mgr_data->mgr.trans_key = 0;
	mgr_data->mgr.trans_enabled = false;
	mgr_data->mgr.interlaced = false;
	mgr_data->mgr.alpha_blending = 1;
	mgr_data->mgr.cpr_enabled = 0;
	mgr_data->mgr.swap_rb = 0;
	mgr_data->num_ovls = NO_OF_PIPELINES;

	mgr_data->ovls[0].cfg.ix = GFX_PIPELINE;
	mgr_data->ovls[0].cfg.enabled = true;
	mgr_data->ovls[0].address = fbptr;
	mgr_data->ovls[0].cfg.stride = (fb_var.xres << 1);
	mgr_data->ovls[0].cfg.pre_mult_alpha = 0;
	mgr_data->ovls[0].cfg.global_alpha = 255;
	mgr_data->ovls[0].cfg.rotation = 0;
	mgr_data->ovls[0].cfg.mirror = 0;
	mgr_data->ovls[0].cfg.ilace = OMAP_DSS_ILACE_NONE;
	mgr_data->ovls[0].cfg.zonly = 0;
	mgr_data->ovls[0].cfg.color_mode = OMAP_DSS_COLOR_ARGB32;
	mgr_data->ovls[0].cfg.zorder = 3;
	mgr_data->ovls[0].cfg.width = fb_var.xres;
	mgr_data->ovls[0].cfg.height = fb_var.yres;
	mgr_data->ovls[0].cfg.win.w = fb_var.xres;
	mgr_data->ovls[0].cfg.win.h = fb_var.yres;
	mgr_data->ovls[0].cfg.win.x = 0;
	mgr_data->ovls[0].cfg.win.y = 0;
	mgr_data->ovls[0].cfg.crop.x = 0;
	mgr_data->ovls[0].cfg.crop.y = 0;
	mgr_data->ovls[0].cfg.crop.w = fb_var.xres;
	mgr_data->ovls[0].cfg.crop.h = fb_var.yres;

	mgr_data->ovls[1].cfg.ix = VID1_PIPELINE;
	mgr_data->ovls[2].cfg.ix = VID2_PIPELINE;
	mgr_data->ovls[3].cfg.ix = VID3_PIPELINE;
	mgr_data->ovls[1].cfg.enabled = false;
	mgr_data->ovls[2].cfg.enabled = false;
	mgr_data->ovls[3].cfg.enabled = false;

	ret = comp_update(mgr_data);

	SLEEP_SEC(1);
	if (ret)
		printf("Error: display_reset: Display FB "
				"- mgr.id=%d composition update failed\n", mgr_data->mgr.ix);
	else
		printf("Reset display done!\n");

	close(fd);

	return ret;
}
