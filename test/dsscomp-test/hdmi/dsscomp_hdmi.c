/*
 * dsscomp_lcd.c
 *
 * Test application for OMAP DSS Composition layer
 *
 * Copyright (C) 2011 Texas Instruments, Inc.
 *
 * This file is licensed under the terms of the GNU General Public License
 * version 2. This program is licensed "as is" without any warranty of any
 * kind, whether express or implied.
 *
 * Author : Charulatha Varadarajan
 * Date   : 25 Aug 2011
 */

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <linux/fb.h>

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <poll.h>
#include "ion/ion.h"
//#include "omapdss.h"
#include "../dsscomp.h"
#include <semaphore.h>
#include "uevent.h"

#define NO_OF_PIPELINES	4
#define NO_OF_MANAGER 2
#define GFX_PIPELINE	0
#define VID1_PIPELINE	1
#define VID2_PIPELINE	2
#define VID3_PIPELINE	3

#define BUF_SDMA	1
#define BUF_TILER2D	2
#define BUF_TILER1D	3
#define BUF_USER	4

#define LCD_OUT		0
#define TV_OUT		1
#define LCD2_OUT	2

#define DEFAULT_IN_WIDTH	640
#define DEFAULT_IN_HEIGHT	480

#if 0
#define DEFAULT_IN_WIDTH	864
#define DEFAULT_IN_HEIGHT	480
static char *file_name = "WVGA.RGBA";
#endif

static char *file_name = "butterfly.rgb32";
static unsigned int test = 4;
static unsigned int disp_out = TV_OUT;
static unsigned int buf_type = BUF_TILER2D;
static struct dsscomp_setup_mgr_data *mgr_data;
static unsigned int zorder = 3, gfx_zorder = 0;
static unsigned int vid1_zorder = 2, vid2_zorder = 1, vid3_zorder = 3;
static unsigned int sampling_factor = 0;
static unsigned int sampling_cordinate = 0;
int dsscomp_fd;
bool up;
static sem_t mutex;

static void fill_default_setup_mgr_data(unsigned int index,
		unsigned int pipeline)
{
	struct timeval tv;
	static __u32 sync_id;

	gettimeofday(&tv, NULL);
	sync_id = tv.tv_usec / 476 + tv.tv_sec * 2100840;

	mgr_data->sync_id = ++sync_id;
	mgr_data->mode = DSSCOMP_SETUP_DISPLAY;
	mgr_data->get_sync_obj = 0;

	mgr_data->mgr.ix = disp_out;

		mgr_data->mgr.default_color = 0xA000EE;

		mgr_data->mgr.trans_key_type = OMAP_DSS_COLOR_KEY_VID_SRC;
		mgr_data->mgr.trans_key = 0;
		mgr_data->mgr.trans_enabled = false;

	mgr_data->mgr.interlaced = false;
	mgr_data->mgr.alpha_blending = 1;
	mgr_data->mgr.cpr_enabled = 0;
	mgr_data->mgr.swap_rb = 0;

	mgr_data->ovls[index].cfg.enabled = true;
	mgr_data->ovls[index].cfg.stride = 4096;
		mgr_data->ovls[index].cfg.pre_mult_alpha = 0;

		mgr_data->ovls[index].cfg.global_alpha = 255;

		mgr_data->ovls[index].cfg.rotation = 0;
		mgr_data->ovls[index].cfg.mirror = 0;

	mgr_data->ovls[index].cfg.ilace = OMAP_DSS_ILACE_NONE;
	mgr_data->ovls[index].cfg.zonly = 0;

		mgr_data->ovls[index].cfg.zorder = zorder;
		mgr_data->ovls[index].cfg.ix = pipeline;
		mgr_data->ovls[index].cfg.color_mode =
						OMAP_DSS_COLOR_ARGB32;
		mgr_data->ovls[index].cfg.width = DEFAULT_IN_WIDTH;
		mgr_data->ovls[index].cfg.height = DEFAULT_IN_HEIGHT;

			mgr_data->ovls[index].cfg.win.w = DEFAULT_IN_WIDTH;
			mgr_data->ovls[index].cfg.win.h = DEFAULT_IN_HEIGHT;
			mgr_data->ovls[index].cfg.win.x = 0;
			mgr_data->ovls[index].cfg.win.y = 0;

			mgr_data->ovls[index].cfg.crop.x = 0;
			mgr_data->ovls[index].cfg.crop.y = 0;
			mgr_data->ovls[index].cfg.crop.w = DEFAULT_IN_WIDTH;
			mgr_data->ovls[index].cfg.crop.h = DEFAULT_IN_HEIGHT;
}
static inline int comp_check_ovl(int fd, unsigned int index)
{
	struct dsscomp_check_ovl_data *chk_ovl = NULL;

	chk_ovl = (typeof(chk_ovl)) calloc(1, sizeof(*chk_ovl));
	if (!chk_ovl) {
		printf("Error: Could not allocate memory for check_ovl data\n");
		return -EINVAL;
	}

	chk_ovl->mgr.ix = mgr_data->mgr.ix;
	chk_ovl->mgr.default_color = mgr_data->mgr.default_color;
	chk_ovl->mgr.trans_key_type = mgr_data->mgr.trans_key_type;
	chk_ovl->mgr.trans_key = mgr_data->mgr.trans_key;
	chk_ovl->mgr.trans_enabled = mgr_data->mgr.trans_enabled;
	chk_ovl->mgr.interlaced = mgr_data->mgr.interlaced;
	chk_ovl->mgr.alpha_blending = mgr_data->mgr.alpha_blending;
	chk_ovl->mgr.cpr_enabled = mgr_data->mgr.cpr_enabled;
	chk_ovl->mgr.swap_rb = mgr_data->mgr.swap_rb;

	chk_ovl->ovl.address = mgr_data->ovls[index].address;
	chk_ovl->ovl.cfg.stride = mgr_data->ovls[index].cfg.stride;
	chk_ovl->ovl.cfg.ix = mgr_data->ovls[index].cfg.ix;
	chk_ovl->ovl.cfg.enabled = mgr_data->ovls[index].cfg.enabled;
	chk_ovl->ovl.cfg.color_mode = mgr_data->ovls[index].cfg.color_mode;
	chk_ovl->ovl.cfg.pre_mult_alpha = mgr_data->ovls[index].cfg.pre_mult_alpha;
	chk_ovl->ovl.cfg.global_alpha = mgr_data->ovls[index].cfg.global_alpha;
	chk_ovl->ovl.cfg.rotation = mgr_data->ovls[index].cfg.rotation;
	chk_ovl->ovl.cfg.mirror = mgr_data->ovls[index].cfg.mirror;
	chk_ovl->ovl.cfg.ilace = mgr_data->ovls[index].cfg.ilace;
	chk_ovl->ovl.cfg.zorder = mgr_data->ovls[index].cfg.zorder;
	chk_ovl->ovl.cfg.zonly = mgr_data->ovls[index].cfg.zonly;
	chk_ovl->ovl.cfg.width = mgr_data->ovls[index].cfg.width;
	chk_ovl->ovl.cfg.height = mgr_data->ovls[index].cfg.height;
	chk_ovl->ovl.cfg.win.x = mgr_data->ovls[index].cfg.win.x;
	chk_ovl->ovl.cfg.win.y = mgr_data->ovls[index].cfg.win.y;
	chk_ovl->ovl.cfg.win.w = mgr_data->ovls[index].cfg.win.w;
	chk_ovl->ovl.cfg.win.h = mgr_data->ovls[index].cfg.win.h;
	chk_ovl->ovl.cfg.crop.x = mgr_data->ovls[index].cfg.crop.x;
	chk_ovl->ovl.cfg.crop.y = mgr_data->ovls[index].cfg.crop.y;
	chk_ovl->ovl.cfg.crop.w = mgr_data->ovls[index].cfg.crop.w;
	chk_ovl->ovl.cfg.crop.h = mgr_data->ovls[index].cfg.crop.h;
	chk_ovl->mode = mgr_data->mode;

	return ioctl(fd, DSSCIOC_CHECK_OVL, chk_ovl);
}

static inline int comp_update(int fd,  struct dsscomp_setup_mgr_data *s)
{
	return ioctl(fd, DSSCIOC_SETUP_MGR, s);
}

int test_mem_alloc(unsigned int w, unsigned int h,
		enum omap_color_mode color_mode, unsigned int index)
{
	int fd, ret = 0;
	int fmt = TILER_PIXEL_FMT_MIN;
	int map_fd, i, j, x, width;
	struct ion_handle *handle;
    	FILE *image_fp;
	unsigned char *ptr;
	size_t stride = 4096;

	switch (color_mode) {
		case OMAP_DSS_COLOR_CLUT1:
		case OMAP_DSS_COLOR_CLUT2:
		case OMAP_DSS_COLOR_CLUT4:
		case OMAP_DSS_COLOR_CLUT8:
			fmt = TILER_PIXEL_FMT_8BIT;
			break;

		case OMAP_DSS_COLOR_RGBA16:
		case OMAP_DSS_COLOR_RGBX16:
		case OMAP_DSS_COLOR_ARGB16_1555:
		case OMAP_DSS_COLOR_XRGB15:
		case OMAP_DSS_COLOR_RGB12U:
		case OMAP_DSS_COLOR_ARGB16:
		case OMAP_DSS_COLOR_RGB16:
			width = 2 * w;
			fmt = TILER_PIXEL_FMT_16BIT;
			break;

		case OMAP_DSS_COLOR_RGB24U:
		case OMAP_DSS_COLOR_RGB24P:
		case OMAP_DSS_COLOR_ARGB32:
		case OMAP_DSS_COLOR_RGBA32:
		case OMAP_DSS_COLOR_RGBX24:
			width = 4 * w;
			fmt = TILER_PIXEL_FMT_32BIT;
			break;

		case OMAP_DSS_COLOR_YUV2:
		case OMAP_DSS_COLOR_UYVY:
		case OMAP_DSS_COLOR_NV12:
			break;

		default:
			break;
	}

	if (buf_type == BUF_TILER1D) {
		fmt = TILER_PIXEL_FMT_PAGE;
		h = 1;
		w = w * h;
	}

	/* Open Image file */
    	image_fp = fopen(file_name, "r");
	if (!image_fp)
		printf("\ncannot open input file\n");

	/* Open Ion Device */
	fd = open("/dev/ion", O_RDWR);
        if (fd < 0)
       	        printf("open /dev/ion failed!\n");

	if ((test == 20) || (test == 10) || (test == 9)) {
		size_t len;

		fmt = TILER_PIXEL_FMT_32BIT;

		for (x = 0; x < NO_OF_PIPELINES; x++) {
			w = mgr_data->ovls[x].cfg.width;
			h = mgr_data->ovls[x].cfg.height;

			ret = ion_alloc_tiler(fd, w, h, fmt, 0, &handle,
					&stride);
			if (!ret) {
				len = h * stride;
				ret = ion_map(fd, handle, len,
						PROT_READ | PROT_WRITE,
						MAP_SHARED, 0, &ptr, &map_fd);
			}
			/* Read the Image and place it in buffer */
			if (!ret && ptr) {
				unsigned int *temp = (unsigned int *) ptr;
				w = mgr_data->ovls[x].cfg.width;

				for (i = 0; i < h; i ++) {
					unsigned int ch;
					for (j = 0; j < w; j ++) {
						if (x == GFX_PIPELINE)
							ch = 0xFFFFFFFF;
						else if (x == VID1_PIPELINE)
							ch = 0xFFFF0000;
						else if (x == VID2_PIPELINE)
							ch = 0xFF00FF00;
						else if (x == VID3_PIPELINE)
							ch = 0xFF000000;
	    					*temp++ = ch;
					}
					temp += ((stride >> 2) - w);
				}
			} else {
				printf("Error: memory allocation failed for "
						"overlay%d image buffer\n", x);
				ret = -EINVAL;
				goto done_alloc;
			}

			mgr_data->ovls[x].address = ptr;
			mgr_data->ovls[x].cfg.stride = stride;
		}
		goto done_alloc;
	}

	if ((buf_type == BUF_TILER1D) || (buf_type == BUF_TILER2D)) {
		size_t len;

		ret = ion_alloc_tiler(fd, w, h, fmt, 0, &handle, &stride);

		if (!ret) {
			len = h * stride;
			ret = ion_map(fd, handle, len, PROT_READ | PROT_WRITE,
					MAP_SHARED, 0, &ptr, &map_fd);
		}

		/* Read the Image and place it in buffer */
		if (!ret && ptr) {
			unsigned int *temp = (unsigned int *) ptr;

			for (i= 0; i < h; i++) {
				if (fread(temp, w, 4, image_fp) != 4)
					break;
				temp += (stride >> 2);
			}
		} else {
			printf("Error: memory allocation failed\n");
			ret = -EINVAL;
			goto done_alloc;
		}

		/* Fill the buffer address and stride info as provided by Ion */
		mgr_data->ovls[index].address = ptr;
		mgr_data->ovls[index].cfg.stride = stride;

#if 0
	} else if ((buf_type == BUF_USER) || (buf_type == BUF_SDMA)) {
		ret = ion_alloc(fd, h * stride, 0, 0, &handle);

		if (ret)
		printf("%s failed: %s\n", __func__, strerror(ret));

		close(fd);
#endif
	}

done_alloc:
	/* Close the ion device */
	close(fd);

	/* Close the Image file */
	fclose(image_fp);
	printf ("\n");

	return ret;
}

int reset_all(void)
{
	int ret =0, a;
	int i,j;	
        struct _qdis {
                struct dsscomp_display_info dis;
                struct dsscomp_videomode modedb[4];
        } d = { .dis = { .ix = 0 } };

	d.dis.modedb_len = sizeof(d.modedb) / sizeof(*d.modedb);


	for(i=0;i<2;i++)
	{
	d.dis.ix=i;
    	ret = ioctl(dsscomp_fd, DSSCIOC_QUERY_DISPLAY, &d);
	if (ret)
        	return ret;
	
	/* Reset all the pipelines before starting the test */
	mgr_data->mgr.default_color = 0xA000EE;
	mgr_data->mode = DSSCOMP_SETUP_DISPLAY;
	mgr_data->get_sync_obj = 0;
	mgr_data->mgr.ix = i;
	mgr_data->mgr.trans_key_type = OMAP_DSS_COLOR_KEY_VID_SRC;
	mgr_data->mgr.trans_key = 0;
	mgr_data->mgr.trans_enabled = false;
	mgr_data->mgr.interlaced = false;
	mgr_data->mgr.alpha_blending = 1;
	mgr_data->mgr.cpr_enabled = 0;
	mgr_data->mgr.swap_rb = 0;



	mgr_data->num_ovls = 0;
	for(j=0;j<NO_OF_PIPELINES;j++){
		if(d.dis.overlays_owned &(1<<j)){
			mgr_data->ovls[mgr_data->num_ovls].cfg.ix = j;
			mgr_data->ovls[mgr_data->num_ovls].cfg.enabled = false;
			printf("Overlay[%d] owned by mgr[%d]\n",j,i);
			mgr_data->num_ovls++;
		}
	}
	/*mgr_data->ovls[0].cfg.ix = GFX_PIPELINE;
	mgr_data->ovls[1].cfg.ix = VID1_PIPELINE;
	mgr_data->ovls[2].cfg.ix = VID2_PIPELINE;
	mgr_data->ovls[3].cfg.ix = VID3_PIPELINE;
	
	mgr_data->ovls[1].cfg.enabled = false;
	mgr_data->ovls[2].cfg.enabled = false;
	mgr_data->ovls[3].cfg.enabled = false;*/

	ret = comp_update(dsscomp_fd, mgr_data);
	if (ret)
		printf("Error: DSSCOMP_SETUP_MGR  ioctl failed: 0x%X\n", ret);
	}
	printf("Hit any key to continue\n");
	a = getchar();
	if (!a)
		a = getchar();
	a = 0;

	return ret;
}

#define PAGE_MASK (~4095)
#define PAGE_ALIGN(x) (((x) + ~PAGE_MASK) & PAGE_MASK)

int display_reset(void)
{
	int fd, ret = 0;
	struct fb_fix_screeninfo fb_fix;
	struct fb_var_screeninfo fb_var;
	char *fbptr = NULL;

	fd = open("/dev/fb0", O_RDWR);
	if (fd < 0)
		fd = open("/dev/graphics/fb0", O_RDWR);
	if (fd < 0) {
       	        printf("open /dev/fb failed!\n");
		return -EINVAL;
	}

	if ((ret = ioctl(fd, FBIOGET_FSCREENINFO, &fb_fix)) != 0 ||
	    (fbptr = (char *) mmap(NULL, PAGE_ALIGN((fb_fix.smem_start & ~PAGE_MASK) +
				   fb_fix.smem_len), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == (char *) -1) {
       	        printf("open fb mmap failed!\n");
		close(fd);
		return -EINVAL;
	}
	fbptr += (fb_fix.smem_start & ~PAGE_MASK);

	if (ioctl(fd, FBIOGET_VSCREENINFO, &fb_var) != 0) {
       	        printf("failed to get screen info\n");
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

	ret = comp_update(dsscomp_fd, mgr_data);
	if (ret)
		printf("Error: DSSCOMP_SETUP_MGR  ioctl failed: 0x%X\n", ret);

	usleep(10);

	mgr_data->mode = DSSCOMP_SETUP_DISPLAY;
	mgr_data->get_sync_obj = 0;
	mgr_data->mgr.default_color = 0;
	mgr_data->mgr.ix = disp_out;
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
	mgr_data->ovls[0].cfg.color_mode = OMAP_DSS_COLOR_RGB16;
	mgr_data->ovls[0].cfg.zorder = zorder;
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

	ret = comp_update(dsscomp_fd, mgr_data);
	if (ret)
		printf("Error: DSSCOMP_SETUP_MGR  ioctl failed: 0x%X\n", ret);

	usleep(10);

	close(fd);
	return ret;
}

#if 0	
static void *hdmi_hpd_thread(void *data)
{
	static char uevent_desc[4096];
	struct pollfd fds[1];
	int timeout;
	int err;
	char *s;
	uevent_init();
	fds[0].fd=uevent_get_fd();
#if 0	
	fd=open("/sys/class/switch/hdmi/uevent", O_RDONLY);
	if(fd<0){
		printf("Open failed\n");
		fd=open("/sys/class/switch/hdmi", O_RDONLY);
		if(fd<0){
			printf("Open failed for second case also\n");
			exit(0);
		}
	}
	fds[0].fd=fd;
#endif
	fds[0].events=POLLIN;

	timeout=-1;//1000;
	memset(uevent_desc, 0, sizeof(uevent_desc));
	do{
		err=poll(fds, 1, timeout);
		if(err==0){
			printf("POLL timeout\n");
			continue;
		}
		if(fds[0].revents & POLLIN){
			uevent_next_event(uevent_desc, sizeof(uevent_desc) - 2);
			s=uevent_desc;
			if (strcmp(s,"change@/devices/virtual/switch/hdmi")){
				continue;
			}
			s += strlen(s)+1;
			/*HDMI uevent*/
			while(*s){
				if (!strncmp(s, "SWITCH_STATE=", strlen("SWITCH_STATE="))) {
					int hdmi_state = atoi(s + strlen("SWITCH_STATE="));
					printf("HDMI HPD: Cable %s\n",hdmi_state==1?"CONNECTED":"DISCONNECTED");
					sem_post(&mutex);
				}
				s += strlen(s) + 1;
			}
		}		
	}while(1);
	return NULL;
}
#endif

int main(int argc, char *argv[])
{
	int ret = 0, a = 0;
	char *s1, *s2;
	unsigned int i;
	pthread_t hpd_thread;
	unsigned int pipeline, input_w, input_h, index = 0;
	bool disp_orig = 1;
	int hpd_count=8;
	int dis_mode;
	int deep_color=0;
	struct dsscomp_setup_display_data sdis = { .ix = 1, };
#if 0
	struct dsscomp_setup_display_interface_data idis = { .ix = 1, };
#endif
	struct _qdis {
		struct dsscomp_display_info dis;
		struct dsscomp_videomode modedb[64];
	} d = { .dis = { .ix = 1 } };

	printf("\n");

	/* Allocate manager data buffe r*/
	mgr_data = (typeof(mgr_data)) calloc(1, sizeof(*mgr_data) +
				NO_OF_PIPELINES * sizeof(*mgr_data->ovls));
	if (!mgr_data) {
		printf("Error: Could not allocate memory for manager data\n");
		return -EINVAL;
	}

	/* Open the dsscomp device */
	dsscomp_fd = open("/dev/dsscomp", O_RDWR);
	if (dsscomp_fd < 0) {
		printf("could not open DSSCOMP: %d\n", dsscomp_fd);
		return -EINVAL;
	}

	/* create, initialize semaphore */
	if( sem_init(&mutex,1,1) < 0)
	{
      		printf("semaphore initilization\n");
		exit(0);
	}
#if 0
	/*Create HPD thread*/
	if (pthread_create(&hpd_thread, NULL, hdmi_hpd_thread, NULL))
    	{
		printf("failed to create HDMI listening thread (%d): %m\n", errno);
		exit(0);
	}	
#endif
	/* reset_all */
#if 1	
	ret = reset_all();
	if (ret) {
		printf("Error: Reset all pipelines failed");
		goto error_exit;
	}
#endif
	/* Default values */
	mgr_data->num_ovls = 1;
	if (test == 15)
		pipeline = VID2_PIPELINE;
	else
		pipeline = VID1_PIPELINE;

	input_w = DEFAULT_IN_WIDTH;
	input_h = DEFAULT_IN_HEIGHT;
	fill_default_setup_mgr_data(index, pipeline);

	/* Parse the input arguments */
	for (i = 1; i < argc; i++) {
		char *a = argv[i];

		s1= strtok(a, "=");
		s2 = strtok(NULL, "=");

		if (strcmp("input_file", s1) ==  0) {
			file_name = s2;
			printf("input_file=%s\n", file_name);

		} else if (strcmp("test", s1) ==  0) {
			test = atoi(s2);
			printf("test number=%d\n", test);

		} else if (strcmp("disp_out", s1) ==  0) {
			disp_out = atoi(s2);
			printf("display device=%d\n", disp_out);

		} else if (strcmp("pipeline", s1) ==  0) {
			pipeline = atoi(s2);
			mgr_data->ovls[index].cfg.ix = pipeline;
			printf("pipeline number=%d\n", atoi(s2));

		} else if (strcmp("disp_id", s1) ==  0) {
			mgr_data->mgr.ix = (unsigned int)atoi(s2);
			printf("disp_id=%u\n", mgr_data->mgr.ix);

		} else if (strcmp("alpha_blending", s1) ==  0) {
			mgr_data->mgr.alpha_blending = (unsigned int)atoi(s2);
			printf("alpha_blending=%u\n",
					mgr_data->mgr.alpha_blending);

		} else if (strcmp("default_color", s1) ==  0) {
			mgr_data->mgr.default_color = atoi(s2);
			printf("default_color=%d\n",
					mgr_data->mgr.default_color);

		} else if (strcmp("trans_key_type", s1) ==  0) {
			mgr_data->mgr.trans_key_type = atoi(s2);
			printf("trans_key_type=%d\n",
					mgr_data->mgr.trans_key_type);

		} else if (strcmp("trans_key_val", s1) ==  0) {
			//sscanf(s3, "%x", &mgr_data->mgr.trans_key);
			mgr_data->mgr.trans_key = (unsigned int)atoi(s2);
			printf("trans_key_val=0x%X\n",
					mgr_data->mgr.trans_key);

		} else if (strcmp("trans_enabled", s1) ==  0) {
			mgr_data->mgr.trans_enabled = (unsigned int)atoi(s2);
			printf("trans_enabled=%u\n",
					mgr_data->mgr.trans_enabled);

		} else if (strcmp("cpr_enabled", s1) ==  0) {
			mgr_data->mgr.cpr_enabled = (unsigned int)atoi(s2);
			printf("cpr_enabled=%u\n",
					mgr_data->mgr.cpr_enabled);

		} else if (strcmp("swap_rb", s1) ==  0) {
			mgr_data->mgr.swap_rb = (unsigned int)atoi(s2);
			printf("swap_rb=%u\n",
					mgr_data->mgr.swap_rb);

		} else if (strcmp("format", s1) ==  0) {
			mgr_data->ovls[index].cfg.color_mode = 1 << (unsigned int)atoi(s2);
			printf("color_mode=%d\n", mgr_data->ovls[index].cfg.color_mode);

		} else if (strcmp("input_w", s1) ==  0) {
			input_w = 1 << (unsigned int)atoi(s2);
			mgr_data->ovls[index].cfg.width = input_w;
			printf("input_w=%d\n", input_w);

		} else if (strcmp("input_h", s1) ==  0) {
			input_h = (unsigned int)atoi(s2);
			mgr_data->ovls[index].cfg.height = input_h;
			printf("input_h=%d\n", input_h);

		} else if (strcmp("premult_alpha", s1) ==  0) {
			mgr_data->ovls[index].cfg.pre_mult_alpha = (unsigned int)atoi(s2);
			printf("pre_mult_alpha=%d\n", mgr_data->ovls[index].cfg.pre_mult_alpha);

		} else if (strcmp("global_alpha", s1) ==  0) {
			mgr_data->ovls[index].cfg.global_alpha = (unsigned int)atoi(s2);
			printf("global_alpha=%d\n", mgr_data->ovls[index].cfg.global_alpha);

		} else if (strcmp("rotation", s1) ==  0) {
			mgr_data->ovls[index].cfg.rotation = (unsigned int)atoi(s2);
			printf("rotation=%d\n", mgr_data->ovls[index].cfg.rotation);

		} else if (strcmp("mirror", s1) ==  0) {
			mgr_data->ovls[index].cfg.mirror = (unsigned int)atoi(s2);
			printf("mirror=%d\n", mgr_data->ovls[index].cfg.mirror);

		} else if (strcmp("ilace", s1) ==  0) {
			mgr_data->ovls[index].cfg.ilace = (unsigned int)atoi(s2);
			printf("mirror=%d\n", mgr_data->ovls[index].cfg.ilace);

		} else if (strcmp("user_buf", s1) ==  0) {
			if ((unsigned int)atoi(s2) == 1)
				buf_type = BUF_USER;
			printf("mirror=%d\n", buf_type);

		} else if (strcmp("zorder", s1) ==  0) {
			zorder = (unsigned int)atoi(s2);
			printf("zorder=%d\n", zorder);

		} else if (strcmp("z0order", s1) ==  0) {
			gfx_zorder = (unsigned int)atoi(s2);
			printf("graphics zorder=%d\n", gfx_zorder);

		} else if (strcmp("z1order", s1) ==  0) {
			vid1_zorder = (unsigned int)atoi(s2);
			printf("video1 zorder=%d\n", vid1_zorder);

		} else if (strcmp("z2order", s1) ==  0) {
			vid2_zorder = (unsigned int)atoi(s2);
			printf("video2 zorder=%d\n", vid2_zorder);

		} else if (strcmp("z3order", s1) ==  0) {
			vid3_zorder = (unsigned int)atoi(s2);
			printf("video3 zorder=%d\n", vid3_zorder);

		} else if (strcmp("up", s1) ==  0) {
			up = (unsigned int)atoi(s2);
			printf("up=%d\n", up);

		} else if (strcmp("factor", s1) ==  0) {
			sampling_factor = (unsigned int)atoi(s2);
			printf("sampling_factor=%d\n", sampling_factor);

		} else if (strcmp("sampling_cordinate", s1) ==  0) {
			sampling_cordinate = (unsigned int)atoi(s2);
			printf("sampling_cordinate=%d\n", sampling_cordinate);

		} else if (strcmp("crop_x", s1) ==  0) {
			mgr_data->ovls[index].cfg.crop.x = (unsigned int)atoi(s2);
			printf("crop_x=%d\n", mgr_data->ovls[index].cfg.crop.x);

		} else if (strcmp("crop_y", s1) ==  0) {
			mgr_data->ovls[index].cfg.crop.y = (unsigned int)atoi(s2);
			printf("crop_y=%d\n", mgr_data->ovls[index].cfg.crop.y);

		} else if (strcmp("crop_w", s1) ==  0) {
			mgr_data->ovls[index].cfg.crop.w = (unsigned int)atoi(s2);
			printf("crop_w=%d\n", mgr_data->ovls[index].cfg.crop.w);

		} else if (strcmp("crop_h", s1) ==  0) {
			mgr_data->ovls[index].cfg.crop.h = (unsigned int)atoi(s2);
			printf("crop_h=%d\n", mgr_data->ovls[index].cfg.crop.h);

		} else {
			printf("Error: invalid input parameter: %s\n\n", s1);
			goto error_exit;
		}
	}

	mgr_data->num_ovls = 1;
	mgr_data->ovls[0].cfg.ix = pipeline;
	mgr_data->ovls[0].cfg.enabled = true;
	mgr_data->ovls[1].cfg.enabled = false;
	mgr_data->ovls[2].cfg.enabled = false;
	mgr_data->ovls[3].cfg.enabled = false;

	switch(pipeline) {
	case GFX_PIPELINE:
		mgr_data->ovls[1].cfg.ix = VID1_PIPELINE;
		mgr_data->ovls[2].cfg.ix = VID2_PIPELINE;
		mgr_data->ovls[3].cfg.ix = VID3_PIPELINE;
		break;

	case VID1_PIPELINE:
		mgr_data->ovls[1].cfg.ix = GFX_PIPELINE;
		mgr_data->ovls[2].cfg.ix = VID2_PIPELINE;
		mgr_data->ovls[3].cfg.ix = VID3_PIPELINE;
		break;

	case VID2_PIPELINE:
		mgr_data->ovls[1].cfg.ix = VID1_PIPELINE;
		mgr_data->ovls[2].cfg.ix = GFX_PIPELINE;
		mgr_data->ovls[3].cfg.ix = VID3_PIPELINE;
		break;

	case VID3_PIPELINE:
		mgr_data->ovls[1].cfg.ix = VID1_PIPELINE;
		mgr_data->ovls[2].cfg.ix = VID2_PIPELINE;
		mgr_data->ovls[3].cfg.ix = GFX_PIPELINE;
		break;

	default:
		break;
	}
#if 0
	if ((test == 11) && disp_orig) {
		fill_default_setup_mgr_data(index, pipeline);

		if (sampling_factor) {
			if (sampling_cordinate == 0) {
				if (up) {
					mgr_data->ovls[index].cfg.crop.w =
						mgr_data->ovls[index].cfg.crop.w
							/ sampling_factor;

					mgr_data->ovls[index].cfg.win.w =
						mgr_data->ovls[index].cfg.win.w
							/ sampling_factor;
				}
			} else if (sampling_cordinate == 1) {
				if (up) {
					mgr_data->ovls[index].cfg.crop.h =
						mgr_data->ovls[index].cfg.crop.h
							/ sampling_factor;

					mgr_data->ovls[index].cfg.win.h =
						mgr_data->ovls[index].cfg.win.h
							/ sampling_factor;
				}
			} else if (sampling_cordinate == 2) {
				if (up) {
					mgr_data->ovls[index].cfg.crop.w =
						mgr_data->ovls[index].cfg.crop.w
							/ sampling_factor;

					mgr_data->ovls[index].cfg.win.w =
						mgr_data->ovls[index].cfg.win.w
							/ sampling_factor;

					mgr_data->ovls[index].cfg.crop.h =
						mgr_data->ovls[index].cfg.crop.h
							/ sampling_factor;

					mgr_data->ovls[index].cfg.win.h =
						mgr_data->ovls[index].cfg.win.h
							/ sampling_factor;
				}
			}
		}
		printf("Displaying original Image...\n");
		goto mem_allc;
	}
#endif
do_test:
	printf("Testcase :%d\n",test);	
	switch(test) {
	case 1:
                while(hpd_count--){
                        printf("HPD test\n");
                        printf("Connect/Disconnect the HDMI display\n");
                        sem_wait(&mutex);
                        printf("HPD detected\n");
                }
	
		return 0;
	case 2: 
#if 0
		printf("HDMI testing deep color\n");
		printf("select deep color mode\n0--->24 bits\n1--->30 bits\n2--->36 bits\n");
		scanf("%d",&deep_color);
		idis.data.hdmi.deep_color = deep_color;
		ret = ioctl(dsscomp_fd, DSSCIOC_SETUP_DISPLAY_INTERFACE, &idis);
		break;
#endif
					
	case 3:
		printf("HDMI: Testing all timings\n");
		printf("Select the timings\n");

		d.dis.modedb_len = sizeof(d.modedb) / sizeof(*d.modedb);
		ret = ioctl(dsscomp_fd, DSSCIOC_QUERY_DISPLAY, &d);
		if (ret)
			return ret;
		for(i=0;i< d.dis.modedb_len;i++)
			printf("%d-->%dx%d@%d %d\n",i,d.modedb[i].xres,d.modedb[i].yres,d.modedb[i].refresh,d.modedb[i].pixclock);

		scanf("%d",&dis_mode);
		if(dis_mode <d.dis.modedb_len){
			sdis.mode = d.dis.modedb[dis_mode];
			printf("picking #%d", dis_mode);
			ioctl(dsscomp_fd, DSSCIOC_SETUP_DISPLAY, &sdis);
		}
		else{
			printf("Wrong selection of timing value\n");
			return -1;
		}
		break;
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 16:
	case 17:
	case 18:
	case 19:
		//fill_default_setup_mgr_data(index, pipeline);
		break;
	default:
		printf("FAIL: DSSCOMP Test case %d - invalid test case\n", test);
		goto error_exit;
	}

	fill_default_setup_mgr_data(index, pipeline);
	/* Alloc memory using ion as per the buf_type provided */
	ret = test_mem_alloc(input_w, input_h,
			mgr_data->ovls[index].cfg.color_mode, index);
	if (ret) {
		printf("Error: test_mem_alloc failed");
		goto error_exit;
	}

#if 0
	/* Check overlay parameters */
	ret = comp_check_ovl(dsscomp_fd, index);
	if ((ret < 0 ) || (ret > (1 << NO_OF_PIPELINES))) {
		printf("DSSCIOC_CHECK_OVL ioctl failed: 0x%x\n", ret);
		goto error_exit;
	}
#endif

	/* Update the composition & Display */
	ret = comp_update(dsscomp_fd, mgr_data);
	if (ret) {
		printf("Error: DSSCOMP_SETUP_MGR  ioctl failed: 0x%X\n", ret);
		goto error_exit;
	}

	printf("Hit any key to continue\n");
	a = getchar();
	if (!a)
		a = getchar();
	a = 0;
	sleep(1);
	if ((test == 11) && disp_orig) {
		disp_orig = 0;
		printf("Original Image displayed!\n");
		goto do_test;
	}

	printf("SUCCESS: DSSCOMP Test case %d Done!\n", test);
#if 0	
	/* reset display */
	ret = display_reset();
	if (ret)
		printf("Error: Display Reset failed");
#endif
error_exit:
	/* Close the dsscomp device */
	close(dsscomp_fd);
	printf("\n");
	return 0;
}
