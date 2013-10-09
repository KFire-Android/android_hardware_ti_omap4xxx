#ifndef DSSCOMP_TEST
#define DSSCOMP_TEST

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
#include <linux/ion.h>
#include <linux/omap_ion.h>

#include "dsscomp.h"

#define NO_OF_PIPELINES	4

#define GFX_PIPELINE	0
#define VID1_PIPELINE	1
#define VID2_PIPELINE	2
#define VID3_PIPELINE	3

#define BUF_SDMA	1
#define BUF_TILER2D	2
#define BUF_TILER1D	3
#define BUF_USER	4

#define LCD_OUT		1
#define TV_OUT		0
#define LCD2_OUT	2

#define DEFAULT_COLOR 		0xA000EE
#define DEFAULT_STRIDE		4096
#define DEFAULT_COLOR_MODE	OMAP_DSS_COLOR_ARGB32

#define ARGB_TEST_FILE_NAME	"butterfly.rgb32"
#define NV12_TEST_FILE_NAME	"xena.yuv"
#define UYVY_TEST_FILE_NAME	"xena.uyvy"

#define DEFAULT_IN_WIDTH	640
#define DEFAULT_IN_HEIGHT	480

#define COLOR_TEST_BOX_WIDTH	400
#define COLOR_TEST_BOX_HEIGHT	600

#define PRIM_DISPLAY		LCD_OUT
#define SEC_DISPLAY		TV_OUT

#define DEFAULT_DISPLAY		TV_OUT
#define NO_OF_MANAGERS		1

#define SLEEP_SEC(x)	sleep(x)

#define WAIT_CODE		{				\
int tmp_char = 0;						\
printf("\nHit any key to continue\n");	\
tmp_char = getchar();					\
if (!tmp_char)							\
tmp_char = getchar();					\
tmp_char = 0;							\
}

struct _qdis {
		struct dsscomp_display_info display_info;
		struct dsscomp_videomode modedb[32];
};

extern char *file_name;

int reset_all(struct dsscomp_setup_mgr_data *mgr_data, unsigned int disp_out);
int display_reset(struct dsscomp_setup_mgr_data *mgr_data, int disp_out);

int comp_update(struct dsscomp_setup_mgr_data *s);
int chk_ovl(struct dsscomp_check_ovl_data *c);
int query_display(struct _qdis *d);

int test_check_ovl(struct dsscomp_setup_mgr_data *mgr_data);
int test_image_display(struct dsscomp_setup_mgr_data *mgr_data);
int test_any_resolution(struct dsscomp_setup_mgr_data *mgr_data,
						unsigned int disp_width, unsigned int disp_height);
int test_premult_alpha(struct dsscomp_setup_mgr_data *mgr_data);
int test_trans_key(struct dsscomp_setup_mgr_data *mgr_data);
int test_all_ovls(struct dsscomp_setup_mgr_data *mgr_data,
						unsigned int gfx_zorder, unsigned int vid1_zorder,
						unsigned int vid2_zorder, unsigned int vid3_zorder);
int test_scaling(struct dsscomp_setup_mgr_data *mgr_data,
						unsigned int sampling_factor,
						unsigned int sampling_cordinate, bool up,
						bool exceed_disp_res);
int test_multi_display(struct dsscomp_setup_mgr_data *mgr_data);
int test_dynamic_display(struct dsscomp_setup_mgr_data *mgr_data);
int test_nv12(struct dsscomp_setup_mgr_data *mgr_data);
int test_nv12_scaling(struct dsscomp_setup_mgr_data *mgr_data,
						unsigned int sampling_factor,
						unsigned int sampling_cordinate, bool up,
						bool exceed_disp_res);
int test_uyvy(struct dsscomp_setup_mgr_data *mgr_data);
int test_uyvy_scaling(struct dsscomp_setup_mgr_data *mgr_data,
						unsigned int sampling_factor,
						unsigned int sampling_cordinate, bool up,
						bool exceed_disp_res);
int test_non_tiler_buf(struct dsscomp_setup_mgr_data *mgr_data);
int test_tiler_1d_buf(struct dsscomp_setup_mgr_data *mgr_data);
#endif
