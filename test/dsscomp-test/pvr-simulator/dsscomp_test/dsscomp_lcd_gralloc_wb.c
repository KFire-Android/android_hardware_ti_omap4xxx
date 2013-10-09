/*
 * dsscomp_lcd_gralloc.c
 *
 * Test application for DSSCOMP Gralloc path
 *
 * Copyright (C) 2011 Texas Instruments, Inc.
 *
 * This file is licensed under the terms of the GNU General Public License
 * version 2. This program is licensed "as is" without any warranty of any
 * kind, whether express or implied.
 *
 * Author : Sreenidhi Koti
 * Date   : 25 Nov 2011
 */

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/fb.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <signal.h>

#include "../../ion/ion.h"
#include "../../dsscomp.h"
#include "pvr_sim.h"

#define MULTI_OVERLAY_TEST

#define NO_OF_PIPELINES			5
#define NO_OF_MANAGER			2
#define GFX_PIPELINE			0
#define VID1_PIPELINE			1
#define VID2_PIPELINE			2
#define VID3_PIPELINE			3
#define WB_PIPELINE				4
#define BUF_SDMA				1
#define BUF_TILER2D				2
#define BUF_TILER1D				3
#define BUF_USER				4
#define LCD_OUT					0
#define TV_OUT					1
#define LCD2_OUT				2
#define DEFAULT_IN_WIDTH		176
#define DEFAULT_IN_HEIGHT		144
#define DEFAULT_IN_WIDTH_NV12	176//416
#define DEFAULT_IN_HEIGHT_NV12	144//240

#define DEFAULT_IN_WB_WIDTH_NV12	720//416
#define DEFAULT_IN_WB_HEIGHT_NV12	1280//240

#define NUM_NV12_FRAMES			376
#define MAX_NUM_BUFFERS 		2
#define PAGE_MASK 				(~4095)
#define PAGE_ALIGN(x) 			(((x) + ~PAGE_MASK) & PAGE_MASK)
#define LINUX_PAGE_SIZE 		(4 * 1024)

static char *file_name_rgb32 = "yc.rgb32";
static char *file_name_nv12 = "test_176x144_30fps.yuv";//"avatar.yuv";//"test_176x144_15fps.yuv";

static unsigned int test = 24;
enum omap_color_mode cmode = OMAP_DSS_COLOR_ARGB32;
static unsigned int disp_out = LCD_OUT;
static struct dsscomp_setup_mgr_data *mgr_data;
static unsigned int zorder = 3;
static unsigned int vid1_zorder = 1;
static unsigned int vid2_zorder = 2;
static unsigned int vid3_zorder = 3;
int dsscomp_fd;
int pvrsim_fd;
int ion_fd;
int temp_map_fd;
unsigned int test24 = 1;;
struct sigaction action;
struct dsscomp_setup_dispc_data *dispc;
volatile int h = 0;
struct ion_handle *ion_handle_array_y[NO_OF_PIPELINES][MAX_NUM_BUFFERS], *ion_handle_array_uv[NO_OF_PIPELINES][MAX_NUM_BUFFERS], *ion_handle_array_ba[NO_OF_PIPELINES][MAX_NUM_BUFFERS];
size_t tiler_stride_y[NO_OF_PIPELINES][MAX_NUM_BUFFERS], tiler_stride_uv[NO_OF_PIPELINES][MAX_NUM_BUFFERS], tiler_stride_ba[NO_OF_PIPELINES][MAX_NUM_BUFFERS];
unsigned char *ptr_y[NO_OF_PIPELINES][MAX_NUM_BUFFERS],*ptr_uv[NO_OF_PIPELINES][MAX_NUM_BUFFERS], *ptr_ba[NO_OF_PIPELINES][MAX_NUM_BUFFERS];
bool up;
int vsync = 0;

void * memalign(size_t boundary, size_t size);

static inline int comp_check_ovl(int fd, unsigned int index)
{
	struct dsscomp_check_ovl_data *chk_ovl = NULL;
	int ret;
	int i,num_ovls=2;

	chk_ovl = (typeof(chk_ovl)) calloc(1, sizeof(*chk_ovl));
	if (!chk_ovl) {
		printf("Error: Could not allocate memory for check_ovl data\n");
		return -EINVAL;
	}

	for (i=0;i<num_ovls;i++) {
		chk_ovl->mgr.ix = dispc->mgrs[0].ix;
		chk_ovl->mgr.default_color = dispc->mgrs[0].default_color;
		chk_ovl->mgr.trans_key_type = dispc->mgrs[0].trans_key_type;
		chk_ovl->mgr.trans_key = dispc->mgrs[0].trans_key;
		chk_ovl->mgr.trans_enabled = dispc->mgrs[0].trans_enabled;
		chk_ovl->mgr.interlaced = dispc->mgrs[0].interlaced;
		chk_ovl->mgr.alpha_blending = dispc->mgrs[0].alpha_blending;
		chk_ovl->mgr.cpr_enabled = dispc->mgrs[0].cpr_enabled;
		chk_ovl->mgr.swap_rb = dispc->mgrs[0].swap_rb;

		chk_ovl->ovl.address = dispc->ovls[i].address;
		chk_ovl->ovl.cfg.stride = dispc->ovls[i].cfg.stride;
		chk_ovl->ovl.cfg.ix = dispc->ovls[i].cfg.ix;
		chk_ovl->ovl.cfg.enabled = dispc->ovls[i].cfg.enabled;
		chk_ovl->ovl.cfg.color_mode = dispc->ovls[i].cfg.color_mode;
		chk_ovl->ovl.cfg.pre_mult_alpha = dispc->ovls[i].cfg.pre_mult_alpha;
		chk_ovl->ovl.cfg.global_alpha = dispc->ovls[i].cfg.global_alpha;
		chk_ovl->ovl.cfg.rotation = dispc->ovls[i].cfg.rotation;
		chk_ovl->ovl.cfg.mirror = dispc->ovls[i].cfg.mirror;
		chk_ovl->ovl.cfg.ilace = dispc->ovls[i].cfg.ilace;
		chk_ovl->ovl.cfg.zorder = dispc->ovls[i].cfg.zorder;
		chk_ovl->ovl.cfg.zonly = dispc->ovls[i].cfg.zonly;
		chk_ovl->ovl.cfg.width = dispc->ovls[i].cfg.width;
		chk_ovl->ovl.cfg.height = dispc->ovls[i].cfg.height;
		chk_ovl->ovl.cfg.win.x = dispc->ovls[i].cfg.win.x;
		chk_ovl->ovl.cfg.win.y = dispc->ovls[i].cfg.win.y;
		chk_ovl->ovl.cfg.win.w = dispc->ovls[i].cfg.win.w;
		chk_ovl->ovl.cfg.win.h = dispc->ovls[i].cfg.win.h;
		chk_ovl->ovl.cfg.crop.x = dispc->ovls[i].cfg.crop.x;
		chk_ovl->ovl.cfg.crop.y = dispc->ovls[i].cfg.crop.y;
		chk_ovl->ovl.cfg.crop.w = dispc->ovls[i].cfg.crop.w;
		chk_ovl->ovl.cfg.crop.h = dispc->ovls[i].cfg.crop.h;
		chk_ovl->mode = dispc->mode;

		ret = ioctl(fd, DSSCIOC_CHECK_OVL, chk_ovl);
		if (ret)
			printf("DSSCIOC_CHECK_OVL Failed for Overlay#: %d\n",i);
	}

	return ret;

}

static inline int comp_update(int fd,  struct dsscomp_setup_mgr_data *s)
{
	return ioctl(fd, DSSCIOC_SETUP_MGR, s);
}

int reset_all(void)
{
	int ret =0;
#ifdef OMAP5_DEFINED
	int a;
#endif
	int i,j;
        struct _qdis {
                struct dsscomp_display_info dis;
                struct dsscomp_videomode modedb[4];
        } d = { .dis = { .ix = 0 } };

	d.dis.modedb_len = sizeof(d.modedb) / sizeof(*d.modedb);


	for(i = 0; i < NO_OF_MANAGER; i++) {
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
	for(j = 0; j < NO_OF_PIPELINES-1; j++){
		if(d.dis.overlays_owned &(1<<j)){
			mgr_data->ovls[mgr_data->num_ovls].cfg.ix = j;
			mgr_data->ovls[mgr_data->num_ovls].cfg.enabled = false;
			printf("Overlay[%d] owned by mgr[%d]\n",j,i);
			mgr_data->num_ovls++;
		}
	}

	ret = comp_update(dsscomp_fd, mgr_data);
	if (ret)
		printf("Error: DSSCOMP_SETUP_MGR  ioctl failed: 0x%X\n", ret);
	}

#ifdef OMAP5_DEFINED
	printf("Hit any key to continue\n");
	a = getchar();
	if (!a)
		a = getchar();
	a = 0;
#endif

	return ret;
}

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
	mgr_data->num_ovls = NO_OF_PIPELINES-1;
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
	mgr_data->num_ovls = NO_OF_PIPELINES-1;

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

void signal_handler (int signum) {
     if (signum == SIGPOLL) {
		 /* Decrementing the composition counter*/
		 h--;
		 vsync++;
		 /* Indicate the callback has been received for the last buffer*/
		 if (h == 1 && test24 != 1) {
		 	 ioctl(pvrsim_fd, PVR_SIM_LAST_BUFFER);
		 }
	 }
     return;
}

/* Cleanup the allocs properly */
void clear_allocs(enum omap_color_mode color_mode) {
	int i,k;
	int ht;
	for (k = 0;k < NO_OF_PIPELINES;k++) {
		for (i = 0;i < MAX_NUM_BUFFERS; i++) {
			if (color_mode == OMAP_DSS_COLOR_ARGB32 || color_mode == OMAP_DSS_COLOR_RGBA32) {
				if (ion_handle_array_ba[k][i]) {
					ion_free(ion_fd, ion_handle_array_ba[k][i]);
					munmap(ptr_ba[i], (DEFAULT_IN_WIDTH * DEFAULT_IN_HEIGHT * 4));
					ion_handle_array_ba[k][i] = NULL;
				}
				else if (ptr_ba[k][i]) {
					printf("Freeing Heap memory : %p\n",ptr_ba[k][i]);
					free(ptr_ba[k][i]);
					ptr_ba[k][i] = NULL;
				}
			}
			else {
				if (ion_handle_array_y[k][i] && ion_handle_array_uv[k][i]) {
					ion_free(ion_fd, ion_handle_array_y[k][i]);
					ion_free(ion_fd, ion_handle_array_uv[k][i]);
					printf("Freeing TILER Y memory : %p\n",ptr_y[k][i]);
					printf("Freeing TILER UV memory : %p\n",ptr_uv[k][i]);
					ht = (k == 0)? DEFAULT_IN_HEIGHT_NV12:DEFAULT_IN_WB_HEIGHT_NV12;
					munmap(ptr_y[k][i], (ht * tiler_stride_y[k][i]));
					munmap(ptr_uv[k][i], (ht/2 * tiler_stride_uv[k][i]));
					ion_handle_array_y[k][i] = ion_handle_array_uv[k][i] = NULL;
				}
			}
		}
	}
	if (temp_map_fd) {
		close(temp_map_fd);
		temp_map_fd = 0;
	}
}

int allocate_heap_memory(unsigned int buffer_num, enum omap_color_mode def_color_mode) {
	int ret = 0;
	unsigned int len;
	int index = 0;
	void *temp_buf;

	if (def_color_mode == OMAP_DSS_COLOR_ARGB32 || def_color_mode == OMAP_DSS_COLOR_RGBA32) {
		len = (DEFAULT_IN_WIDTH * DEFAULT_IN_HEIGHT * 4);
		/* Make the length 4K multiple*/
		len = (len + LINUX_PAGE_SIZE -1) & ~(LINUX_PAGE_SIZE - 1);
		dispc->ovls[index].cfg.stride = tiler_stride_ba[0][buffer_num] = DEFAULT_IN_WIDTH * 4;
	}
	else {
		/* TODO */
		printf("NV-12 HEAP MEMORY ALLOCATION IS CURRENTLY NOT SUPPORTED..DEFAULTING TO ARGB-32");
		len = (DEFAULT_IN_WIDTH * DEFAULT_IN_HEIGHT * 4);
		/* Make the length 4K multiple*/
		len = (len + LINUX_PAGE_SIZE -1) & ~(LINUX_PAGE_SIZE - 1);
		dispc->ovls[index].cfg.stride = tiler_stride_ba[0][buffer_num] = DEFAULT_IN_WIDTH * 4;
	}
	/* Align the memory at 4k boundary. Needed to fix corrupted image*/
	temp_buf = (void *)memalign(4096,len);
	if (!temp_buf) {
		printf("Failed to allocate memory. returning..\n");
		return -ENOMEM;
	}
	memset(temp_buf,0,len);

	ptr_ba[0][buffer_num] = (unsigned char*)temp_buf;

    dispc->ovls[index].address = ptr_ba[0][buffer_num];

	return ret;
}

int allocate_gralloc_memory(unsigned int buffer_num, enum omap_color_mode def_color_mode) {
	unsigned int flags = 0;
	int ret = 0;
	int index = 0;
	int num_ovls = 3;
	int i;
	int wd,ht;

	for (i=0;i<num_ovls;i++) {
		if (def_color_mode == OMAP_DSS_COLOR_NV12) {
			if (i == 0) {
				wd = DEFAULT_IN_WIDTH_NV12;
				ht = DEFAULT_IN_HEIGHT_NV12;
			}
			else {
				wd = DEFAULT_IN_WB_WIDTH_NV12;
				ht = DEFAULT_IN_WB_HEIGHT_NV12;
			}
		}
		/* Default allocate for ARGB32 */
		if (def_color_mode == OMAP_DSS_COLOR_ARGB32 || def_color_mode == OMAP_DSS_COLOR_RGBA32) {
			ret = ion_alloc_tiler(ion_fd, DEFAULT_IN_WIDTH, DEFAULT_IN_HEIGHT, TILER_PIXEL_FMT_32BIT, 0, &ion_handle_array_ba[i][buffer_num], &tiler_stride_ba[i][buffer_num]);
			if (ret) {
				printf("ion_alloc_tiler failed for ARGB32 Buffer (Buff No : %d)..Err : %d\n",buffer_num,ret);
				return ret;
			}
			else {
				ret = ion_map(ion_fd, ion_handle_array_ba[i][buffer_num], (DEFAULT_IN_HEIGHT * tiler_stride_ba[i][buffer_num]), (PROT_READ | PROT_WRITE), MAP_SHARED, 0, &ptr_ba[i][buffer_num], &temp_map_fd);
				if (ret) {
					printf("Failed mapping for ARGB32 Buffer (Buff No : %d)..Err : %d\n",buffer_num,ret);
					return ret;
				}
				dispc->ovls[i].address = ptr_ba[i][buffer_num];
				dispc->ovls[i].cfg.stride = tiler_stride_ba[i][buffer_num];
			}
		}
		/* Allocate for NV-12 Buffer */
		else {
			/* Allocate memory for Y-Buffer */
			ret = ion_alloc_tiler(ion_fd, wd, ht, TILER_PIXEL_FMT_8BIT, flags, &ion_handle_array_y[i][buffer_num], &tiler_stride_y[i][buffer_num]);
			if (!ret) {
				/* Allocate memory for UV-Buffer */
				ret = ion_alloc_tiler(ion_fd, wd/2,ht/2, TILER_PIXEL_FMT_16BIT, flags, &ion_handle_array_uv[i][buffer_num], &tiler_stride_uv[i][buffer_num]);
				if (ret) {
					printf("ion_alloc_tiler failed for UV-Buffer (Buff No : %d)..Err : %d\n",buffer_num,ret);
					return ret;
				}
			}
			else {
				printf("ion_alloc_tiler failed for Y-Buffer (Buff No : %d)..Err : %d\n",buffer_num,ret);
				return ret;
			}

			/* Map the Y-Buffer */
			ret = ion_map(ion_fd, ion_handle_array_y[i][buffer_num], (ht * tiler_stride_y[i][buffer_num]), (PROT_READ | PROT_WRITE), MAP_SHARED, 0, &ptr_y[i][buffer_num], &temp_map_fd);
			if (!ret) {
				dispc->ovls[i].address = ptr_y[i][buffer_num];
				dispc->ovls[i].cfg.stride = tiler_stride_y[i][buffer_num];
				printf("dispc->ovls[%d].address : %p\n",i,dispc->ovls[i].address);
				printf("Stride Y : %d\n",tiler_stride_y[i][buffer_num]);
			}
			else {
				printf("Failed mapping for Y-Buffer (Buffer Num : %d). Err : %d",buffer_num,ret);
				clear_allocs(OMAP_DSS_COLOR_NV12);
				return ret;
			}
			/* Map the UV-Buffer */
			ret = ion_map(ion_fd, ion_handle_array_uv[i][buffer_num], (ht/2 * tiler_stride_uv[i][buffer_num]), (PROT_READ | PROT_WRITE), MAP_SHARED, 0, &ptr_uv[i][buffer_num], &temp_map_fd);
			if (!ret) {
				dispc->ovls[i].uv_address = ptr_uv[i][buffer_num];
				printf("dispc->ovls[%d].uv_address  : %p\n",i,dispc->ovls[i].uv_address );
				printf("Stride UV : %d\n",tiler_stride_uv[i][buffer_num]);
			}
			else {
				printf("Failed mapping for UV-Buffer (Buffer Num : %d). Err : %d",buffer_num,ret);
				clear_allocs(OMAP_DSS_COLOR_NV12);
				return ret;
			}
		}
	}
	return 0;
}

void fill_gralloc_memory(FILE *image_fp, unsigned int buffer_num, enum omap_color_mode def_color_mode) {
	int i,j;
	unsigned char *buf_y, *buf_uv;
	unsigned int *temp;
	int index = 0;
	int k,num_ovls=1;

   for (k=0;k<num_ovls;k++) {
	   if (def_color_mode == OMAP_DSS_COLOR_ARGB32 || def_color_mode == OMAP_DSS_COLOR_RGBA32) {
			/* Read the Image and place it in buffer */
			temp = (unsigned int *) ptr_ba[k][buffer_num];

			for (i= 0; i < DEFAULT_IN_HEIGHT; i++) {
				for (j= 0; j < DEFAULT_IN_WIDTH; j++) {
					if (fread(temp, 1, 4, image_fp) == 4) {
						*temp++ |= 0xFF000000;
					}
				}
				temp += (tiler_stride_ba[k][buffer_num] >> 2) - DEFAULT_IN_WIDTH;
			}
			dispc->ovls[k].address = ptr_ba[k][buffer_num];
		}
		else {
			/* Fill the Y-Buffer */
			buf_y = (unsigned char *) ptr_y[k][buffer_num];
			for (i= 0; i < DEFAULT_IN_HEIGHT_NV12; i++) {
				fread(buf_y, DEFAULT_IN_WIDTH_NV12, 1, image_fp);
				buf_y += tiler_stride_y[k][buffer_num];
			}
			dispc->ovls[k].address = ptr_y[k][buffer_num];

			/* Fill the UV-Buffer */
			buf_uv = (unsigned char *) ptr_uv[k][buffer_num];
			for (i= 0; i < DEFAULT_IN_HEIGHT_NV12/2; i++) {
				fread(buf_uv, DEFAULT_IN_WIDTH_NV12/2, 2,image_fp);
				buf_uv += tiler_stride_uv[k][buffer_num];
			}
			dispc->ovls[k].uv_address = ptr_uv[k][buffer_num];
		}
	}
}

void fill_default_wb_data(enum omap_color_mode cmode) {

	struct timeval tv;
	static __u32 sync_id;
	unsigned int index = 0;
	int i;

	/* Fill the default values */
	gettimeofday(&tv, NULL);
	sync_id = tv.tv_usec / 476 + tv.tv_sec * 2100840;

	dispc->sync_id = ++sync_id;
	dispc->mode = DSSCOMP_SETUP_DISPLAY;
	dispc->get_sync_obj = 0;
	dispc->num_ovls = 2;
	dispc->num_mgrs = 1;

	/* Setting Manager Info (Manager 1 - LCD) */
	dispc->mgrs[index].ix = LCD_OUT;
	dispc->mgrs[index].default_color = 0xA000EE;
	dispc->mgrs[index].trans_key_type = OMAP_DSS_COLOR_KEY_VID_SRC;
	dispc->mgrs[index].trans_key = 0;
	dispc->mgrs[index].trans_enabled = false;
	dispc->mgrs[index].interlaced = false;
	dispc->mgrs[index].alpha_blending = 1;
	dispc->mgrs[index].cpr_enabled = 0;
	dispc->mgrs[index].swap_rb = 0;

	for (i=0;i<dispc->num_ovls;i++) {
		/* Setting Overlay info (Pipeline 1) */
		dispc->ovls[i].cfg.enabled = true;
		dispc->ovls[i].cfg.stride = 4096;
		dispc->ovls[i].cfg.pre_mult_alpha = 0;
		dispc->ovls[i].cfg.global_alpha = 255;
		dispc->ovls[i].cfg.rotation = 0;
		dispc->ovls[i].cfg.mirror = 0;
		dispc->ovls[i].cfg.ilace = OMAP_DSS_ILACE_NONE;
		dispc->ovls[i].cfg.zonly = 0;
		if ((i+1) != WB_PIPELINE-2)
			dispc->ovls[i].cfg.zorder = i+1;
		dispc->ovls[i].cfg.ix = i+1;
		dispc->ovls[i].cfg.color_mode = cmode;
		if (cmode == OMAP_DSS_COLOR_ARGB32 || cmode == OMAP_DSS_COLOR_RGBA32) {
			dispc->ovls[i].cfg.width = DEFAULT_IN_WIDTH;
			dispc->ovls[i].cfg.height = DEFAULT_IN_HEIGHT;
			dispc->ovls[i].cfg.win.w = DEFAULT_IN_WIDTH;
			dispc->ovls[i].cfg.win.h = DEFAULT_IN_HEIGHT;
			dispc->ovls[i].cfg.crop.w = DEFAULT_IN_WIDTH;
			dispc->ovls[i].cfg.crop.h = DEFAULT_IN_HEIGHT;
		}
		else {
			dispc->ovls[i].cfg.width = DEFAULT_IN_WIDTH_NV12;
			dispc->ovls[i].cfg.height = DEFAULT_IN_HEIGHT_NV12;
			dispc->ovls[i].cfg.win.w = DEFAULT_IN_WIDTH_NV12;
			dispc->ovls[i].cfg.win.h = DEFAULT_IN_HEIGHT_NV12;
			dispc->ovls[i].cfg.crop.w = DEFAULT_IN_WIDTH_NV12;
			dispc->ovls[i].cfg.crop.h = DEFAULT_IN_HEIGHT_NV12;
			if ((i+1) != WB_PIPELINE-2) {
				dispc->ovls[i].cfg.cconv.ry = 298;
				dispc->ovls[i].cfg.cconv.rcr = 409;
				dispc->ovls[i].cfg.cconv.rcb = 0;
				dispc->ovls[i].cfg.cconv.gy = 298;
				dispc->ovls[i].cfg.cconv.gcr = -208;
				dispc->ovls[i].cfg.cconv.gcb = -100;
				dispc->ovls[i].cfg.cconv.by = 298;
				dispc->ovls[i].cfg.cconv.bcr = 0;
				dispc->ovls[i].cfg.cconv.bcb = 517;
				dispc->ovls[i].cfg.cconv.full_range = 0;
			}
		}
		if ((i+1) == VID1_PIPELINE) {
			dispc->ovls[i].cfg.win.x = 0;
			dispc->ovls[i].cfg.win.y = 0;
		}
		else if ((i+1) == WB_PIPELINE-2) {
			printf("Setting for WB Pileline\n");
			/* WB Pipeline settings */
			dispc->ovls[i].cfg.ix = WB_PIPELINE;
			dispc->ovls[i].cfg.enabled = true;
			dispc->ovls[i].cfg.color_mode = cmode;
			dispc->ovls[i].cfg.mgr_ix = LCD_OUT;
			dispc->ovls[i].cfg.width = 720;
			dispc->ovls[i].cfg.height = 1280;
			dispc->ovls[i].cfg.win.w = 720;
			dispc->ovls[i].cfg.win.h = 1280;
			dispc->ovls[i].cfg.crop.w = 720;
			dispc->ovls[i].cfg.crop.h = 1280;
			dispc->ovls[i].cfg.win.x = 0;
			dispc->ovls[i].cfg.win.y = 0;
		}
	}

	/* Setting Pipeline #2 */
	dispc->ovls[2].cfg.width = 720;
	dispc->ovls[2].cfg.height = 1280;
	dispc->ovls[2].cfg.win.w = DEFAULT_IN_HEIGHT_NV12;
	dispc->ovls[2].cfg.win.h = DEFAULT_IN_HEIGHT_NV12;
	dispc->ovls[2].cfg.crop.w = DEFAULT_IN_HEIGHT_NV12;
	dispc->ovls[2].cfg.crop.h = DEFAULT_IN_HEIGHT_NV12;
	dispc->ovls[2].cfg.enabled = false;
	dispc->ovls[2].cfg.stride = 4096;
	dispc->ovls[2].cfg.pre_mult_alpha = 0;
	dispc->ovls[2].cfg.global_alpha = 255;
	dispc->ovls[2].cfg.rotation = 0;
	dispc->ovls[2].cfg.mirror = 0;
	dispc->ovls[2].cfg.ilace = OMAP_DSS_ILACE_NONE;
	dispc->ovls[2].cfg.zonly = 0;
	dispc->ovls[2].cfg.zorder = 2;
	dispc->ovls[2].cfg.ix = 2;
	dispc->ovls[2].cfg.color_mode = cmode;
	dispc->ovls[2].cfg.win.x = 250;
	dispc->ovls[2].cfg.win.y = 250;
	dispc->ovls[2].cfg.cconv.ry = 298;
	dispc->ovls[2].cfg.cconv.rcr = 409;
	dispc->ovls[2].cfg.cconv.rcb = 0;
	dispc->ovls[2].cfg.cconv.gy = 298;
	dispc->ovls[2].cfg.cconv.gcr = -208;
	dispc->ovls[2].cfg.cconv.gcb = -100;
	dispc->ovls[2].cfg.cconv.by = 298;
	dispc->ovls[2].cfg.cconv.bcr = 0;
	dispc->ovls[2].cfg.cconv.bcb = 517;
	dispc->ovls[2].cfg.cconv.full_range = 0;

}

void fill_default_dispc_data(enum omap_color_mode cmode) {

	struct timeval tv;
	static __u32 sync_id;
	unsigned int index = 0;
	int i;

	/* Fill the default values */
	gettimeofday(&tv, NULL);
	sync_id = tv.tv_usec / 476 + tv.tv_sec * 2100840;

	dispc->sync_id = ++sync_id;
	dispc->mode = DSSCOMP_SETUP_DISPLAY;
	dispc->get_sync_obj = 0;
	dispc->num_ovls = 3;
	dispc->num_mgrs = 1;

	for (i=1;i<dispc->num_ovls;i++) {
		/* Setting Manager Info (Manager 1 - LCD) */
		dispc->mgrs[index].ix = LCD_OUT;
		dispc->mgrs[index].default_color = 0xA000EE;
		dispc->mgrs[index].trans_key_type = OMAP_DSS_COLOR_KEY_VID_SRC;
		dispc->mgrs[index].trans_key = 0;
		dispc->mgrs[index].trans_enabled = false;
		dispc->mgrs[index].interlaced = false;
		dispc->mgrs[index].alpha_blending = 1;
		dispc->mgrs[index].cpr_enabled = 0;
		dispc->mgrs[index].swap_rb = 0;

		/* Setting Overlay info (Pipeline 1) */
		dispc->ovls[i].cfg.enabled = true;
		dispc->ovls[i].cfg.stride = 4096;
		dispc->ovls[i].cfg.pre_mult_alpha = 0;
		dispc->ovls[i].cfg.global_alpha = 255;
		dispc->ovls[i].cfg.rotation = 0;
		dispc->ovls[i].cfg.mirror = 0;
		dispc->ovls[i].cfg.ilace = OMAP_DSS_ILACE_NONE;
		dispc->ovls[i].cfg.zonly = 0;
		dispc->ovls[i].cfg.zorder = i;
		dispc->ovls[i].cfg.ix = i;
		dispc->ovls[i].cfg.color_mode = cmode;
		if (cmode == OMAP_DSS_COLOR_ARGB32 || cmode == OMAP_DSS_COLOR_RGBA32) {
			dispc->ovls[i].cfg.width = DEFAULT_IN_WIDTH;
			dispc->ovls[i].cfg.height = DEFAULT_IN_HEIGHT;
			dispc->ovls[i].cfg.win.w = DEFAULT_IN_WIDTH;
			dispc->ovls[i].cfg.win.h = DEFAULT_IN_HEIGHT;
			dispc->ovls[i].cfg.crop.w = DEFAULT_IN_WIDTH;
			dispc->ovls[i].cfg.crop.h = DEFAULT_IN_HEIGHT;
		}
		else {
			dispc->ovls[i].cfg.width = DEFAULT_IN_WIDTH_NV12;
			dispc->ovls[i].cfg.height = DEFAULT_IN_HEIGHT_NV12;
			dispc->ovls[i].cfg.win.w = DEFAULT_IN_WIDTH_NV12;
			dispc->ovls[i].cfg.win.h = DEFAULT_IN_HEIGHT_NV12;
			dispc->ovls[i].cfg.crop.w = DEFAULT_IN_WIDTH_NV12;
			dispc->ovls[i].cfg.crop.h = DEFAULT_IN_HEIGHT_NV12;
			dispc->ovls[i].cfg.cconv.ry = 298;
			dispc->ovls[i].cfg.cconv.rcr = 409;
			dispc->ovls[i].cfg.cconv.rcb = 0;
			dispc->ovls[i].cfg.cconv.gy = 298;
			dispc->ovls[i].cfg.cconv.gcr = -208;
			dispc->ovls[i].cfg.cconv.gcb = -100;
			dispc->ovls[i].cfg.cconv.by = 298;
			dispc->ovls[i].cfg.cconv.bcr = 0;
			dispc->ovls[i].cfg.cconv.bcb = 517;
			dispc->ovls[i].cfg.cconv.full_range = 0;
		}
		if (i == VID1_PIPELINE) {
			dispc->ovls[i].cfg.win.x = 0;
			dispc->ovls[i].cfg.win.y = 0;
		}
		else if (i == VID2_PIPELINE) {
			dispc->ovls[i].cfg.win.x = 50;
			dispc->ovls[i].cfg.win.y = 50;
		}
		else if (i == VID3_PIPELINE) {
			dispc->ovls[i].cfg.win.x = 150;
			dispc->ovls[i].cfg.win.y = 150;
		}
		dispc->ovls[i].cfg.crop.x = 0;
		dispc->ovls[i].cfg.crop.y = 0;
	}
}

void get_gralloc_option() {
	int a = 0;
	printf("\nTEST-CASE 24 GIVES THE FOLLOWING OPTIONS\n");
	printf("----------------------------------------\n\n");
	printf("1. PLAY A VIDEO FILE (TO VERIFY SYNC & QUEUING)\n");
	printf("2. VERIFY VARIOUS STATES OF COMPOSITION\n");
	printf("3. TO VERIFY NON-TILER BUFFER MAPPING IN DSSCOMP\n");
	printf("4. TO VERIFY WB CAPTURE MODE\n\n");
	printf("CHOOSE YOUR OPTION: ");
	scanf("%d",&test24);
	switch(test24) {
		case 1:
		printf("\n\n PLAYING THE VIDEO FILE...\n");
		test24 = 4;
		cmode = OMAP_DSS_COLOR_NV12;
		break;

		case 2:
		printf("\n\n VERIFYING VARIOUS STATES OF COMPOSITION...\n");
		test24 = 4;
		cmode = OMAP_DSS_COLOR_NV12;
		break;

		case 3:
		printf("\n\n VERIFYING NON TILER BUFFER COMPOSITION...\n");
		test24 = 4;
		cmode = OMAP_DSS_COLOR_NV12;//OMAP_DSS_COLOR_ARGB32;
		break;

		case 4:
		printf("\n\n TESTING WB CAPTURE MODE...\n");
		cmode = OMAP_DSS_COLOR_NV12;
		break;

		default:
		test24 = 1;
		cmode = OMAP_DSS_COLOR_NV12;
		printf("\n\n DEFAULT CHOOSING VIDEO PLAYBACK...\n");
		break;
	}
	/* Flush the enter key*/
	a = getchar();
	if (!a)
		a = getchar();
	a = 0;

	if (cmode == 2 && test24 == 3) {
		printf("*******************************************************************************\n");
		printf("NON-TILER COMPOSITION FOR NV12 BUFFERS IS NOT SUPPORTED. DEFAULTING TO ARGB32..\n");
		printf("*******************************************************************************\n");
		cmode = OMAP_DSS_COLOR_ARGB32;
	}
}

int queue_gralloc_composition(enum omap_color_mode cmode) {
	int ret = 0,counter = 0, i = 0, j = 0;
	FILE *fp;
	enum omap_color_mode def_color_mode = cmode;
	static char *file_name;

	/* dispc initialization */
	dispc = (typeof(dispc)) calloc(1, sizeof(*dispc) +
				NO_OF_PIPELINES * sizeof(*dispc->ovls));
	if (!dispc) {
		printf("Error: Could not allocate memory for dispc data\n");
		return -EINVAL;
	}

	if (def_color_mode == OMAP_DSS_COLOR_ARGB32 || def_color_mode == OMAP_DSS_COLOR_RGBA32) {
		file_name = file_name_rgb32;
	}
	else {
		file_name = file_name_nv12;
	}

	if (test24 == 4) {
		/* Fill default WB data */
		fill_default_wb_data(def_color_mode);
	}
	else {
		/* Fill default dispc data */
		fill_default_dispc_data(def_color_mode);
	}

	printf("\nCurrent PID : %d\n",getpid());

	/* Allocate & map the buffers for MAX buffers */
	for (counter = 0; counter < MAX_NUM_BUFFERS; counter++) {

		if (test24 != 3) {
			printf("Allocating gralloc memory..\n");
			/* Allocate the Gralloc memory */
			ret = allocate_gralloc_memory(counter, def_color_mode);
			if(ret) {
				printf("failed to allocate gralloc memory..\n");
				return ret;
			}
		}
		else {
			printf("Allocating Heap memory to test non tiler buffer mapping..\n");
			ret = allocate_heap_memory(counter, def_color_mode);
		}

		/* Check overlay parameters */
		ret = comp_check_ovl(dsscomp_fd, 0);
		if ((ret < 0 ) || (ret > (1 << NO_OF_PIPELINES))) {
			printf("DSSCOMP_CHECK_OVL ioctl failed: 0x%x\n", ret);
			return ret;
		}

		/* Queuing the allocated max buffers for MAX_NUM_BUFFERS*/
		if (test24 == 3 || test24 == 2)
		{
			/* Open the same Image file */
			fp = fopen(file_name, "r");
			if (!fp) {
				printf("\ncannot open input file\n");
			}

			printf("Filling gralloc/Heap memory..\n");
			/* Fill the Gralloc memory */
			fill_gralloc_memory(fp, counter, def_color_mode);

			/* close the file pointer */
			fclose(fp);

			/* increment the composition */
			h++;

			/* Call the pvrsim IOCTL for posting the buffer*/
			ioctl(pvrsim_fd, PVR_SIM_POST2, dispc);
		}
	}
	if (test24 == 3 || test24 == 2) {
		/* Wait while we get the callbacks for all the compositions */
		while (h != 1) {
			usleep(10);
		}
	}
	else if (test24 == 1 || test24 == 4) {
		/* Open the Image file */
		fp = fopen(file_name, "r");
		if (!fp) {
			printf("\ncannot open input file\n");
		}

		while (counter < NUM_NV12_FRAMES) {
			/* Initially we have all the buffers */
			while (i<MAX_NUM_BUFFERS) {
				/* Queue the available buffers first */
				fill_gralloc_memory(fp, i, def_color_mode);
				i++;
				dispc->ovls[1].address = ptr_y[1][0];
				dispc->ovls[1].uv_address = ptr_uv[1][0];
				/* increment the composition */
				h++;
				counter++;
				j ^= 1;
				/* Call the pvrsim IOCTL for posting the buffer */
				ioctl(pvrsim_fd, PVR_SIM_POST2, dispc);
			}
			while(h != 1) {
				usleep(10);
			}
			fill_gralloc_memory(fp, j, def_color_mode);
			/* increment the composition */
			h++;
			counter++;
			j ^= 1;

			dispc->ovls[2].address = ptr_y[1][0];
			dispc->ovls[2].uv_address = ptr_uv[1][0];
			dispc->ovls[2].cfg.enabled = true;
			dispc->num_ovls = 3;
			/* Call the pvrsim IOCTL for posting the buffer */
			ioctl(pvrsim_fd, PVR_SIM_POST2, dispc);
		}
		/* close the file pointer */
		fclose(fp);
		/* Finished Queuing composition */
		 printf("Sending the indication about last buffer\n");
		 ioctl(pvrsim_fd, PVR_SIM_LAST_BUFFER);
	}

	return 0;
}

int main(int argc, char *argv[])
{
	int ret = 0, a = 0;
	char *s1, *s2;
	unsigned int i;

	printf("\n");

	/* Open the dsscomp device */
	dsscomp_fd = open("/dev/dsscomp", O_RDWR);
	if (dsscomp_fd < 0) {
		printf("could not open DSSCOMP: %d\n", dsscomp_fd);
		return -EINVAL;
	}

	/* Open the ION Device */
	ion_fd = open("/dev/ion", O_RDWR);
	if (ion_fd < 0) {
		printf("open /dev/ion failed!\n");
		return -EINVAL;
	}

	/* Open the pvrsim device */
	pvrsim_fd = open("/dev/pvrsim", O_RDWR);
	if (pvrsim_fd < 0) {
		printf("could not open PVRSIM: %d\n", pvrsim_fd);
		return -EINVAL;
	}

	/* Allocate manager data buffer */
	mgr_data = (typeof(mgr_data)) calloc(1, sizeof(*mgr_data) +
				NO_OF_PIPELINES * sizeof(*mgr_data->ovls));
	if (!mgr_data) {
		printf("Error: Could not allocate memory for manager data\n");
		return -EINVAL;
	}

	/* clean variable */
	memset(&action, 0, sizeof (action));
	/* specify signal handler */
	action.sa_handler = signal_handler;
	/* operation flags setting */
	action.sa_flags = 0;
	/* attach action with SIGPOLL */
	sigaction(SIGPOLL, &action, NULL);

	/* reset_all */
	ret = reset_all();
	if (ret) {
		printf("Error: Reset all pipelines failed");
		goto error_exit;
	}

	/* Parse the input arguments to get the test number*/
	for (i = 1; i < argc; i++) {
		char *a = argv[i];

		s1= strtok(a, "=");
		s2 = strtok(NULL, "=");

		if (strcmp("test", s1) ==  0) {
			test = atoi(s2);
			printf("test number=%d\n", test);
		}
	}

	if (test == 24) {
		/* Get menu Options */
		get_gralloc_option();
		/* Start queuing the compositions */
		ret = queue_gralloc_composition(cmode);
		if (ret) {
			printf("error in queuing composition\n");
			return ret;
		}
	}

	if (test24 == 1) {
		printf("\n\n******************************************************************* \n");
		printf("The Video is Played with TILER 2D BUFFERS Using YUV-420 NV12 format \n");
		printf("******************************************************************* \n");
	}

	else if (test24 == 2) {
		printf("\n\n*********************************************************************** \n");
		printf("Please confirm the Image is Displayed on the screen. This verifies all \n");
		printf("the states of the DSS composition, ACTIVE, APPLYING, APPLIED, PROGRAMMED, \n");
		printf("DISPLAYED and RELEASED. \n");
		printf("*********************************************************************** \n");
	}

	if (test24 == 3) {
		printf("\n\n********************************************************************** \n");
		printf("The ARGB32 Image is displayed with mapped TILER 1D buffers in DSSCOMP.\n");
		printf("********************************************************************** \n");
	}

	if (test24 == 4) {
		printf("\n\n********************************************************************** \n");
		printf("The Output of the LCD is Catured in the WB Buffer. Displaying is Pending\n");
		printf("********************************************************************** \n");
	}
	printf("Hit any key to continue\n");

	a = getchar();
	if (!a)
		a = getchar();
	a = 0;

	/* delete the allocated tiler/Heap memories */
	clear_allocs(cmode);

	/* reset display */
	ret = display_reset();
	if (ret)
		printf("Error: Display Reset failed");

error_exit:
	/* Close the opened devices */
	close(ion_fd);
	close(dsscomp_fd);
	close(pvrsim_fd);
	printf("\n");

	return 0;
}
