#include "dsscomp_test.h"

static void setup_all_ovls(struct dsscomp_setup_mgr_data *mgr_data,
					unsigned int gfx_zorder,  unsigned int vid1_zorder,
					unsigned int vid2_zorder,  unsigned int vid3_zorder)
{
	mgr_data->num_ovls = NO_OF_PIPELINES;

	/* GFX PIPELINE */
	mgr_data->ovls[0].cfg.zorder = gfx_zorder;
	mgr_data->ovls[0].cfg.zonly = 0;
	mgr_data->ovls[0].cfg.ix = GFX_PIPELINE;
	mgr_data->ovls[0].cfg.color_mode = OMAP_DSS_COLOR_ARGB32;
	mgr_data->ovls[0].cfg.width = COLOR_TEST_BOX_WIDTH;
	mgr_data->ovls[0].cfg.height = COLOR_TEST_BOX_HEIGHT;
	mgr_data->ovls[0].cfg.win.x = 25;
	mgr_data->ovls[0].cfg.win.y = 25;
	mgr_data->ovls[0].cfg.win.w = COLOR_TEST_BOX_WIDTH;
	mgr_data->ovls[0].cfg.win.h = COLOR_TEST_BOX_HEIGHT;
	mgr_data->ovls[0].cfg.crop.x = 0;
	mgr_data->ovls[0].cfg.crop.y = 0;
	mgr_data->ovls[0].cfg.crop.w = COLOR_TEST_BOX_WIDTH;
	mgr_data->ovls[0].cfg.crop.h = COLOR_TEST_BOX_HEIGHT;
	mgr_data->ovls[0].cfg.enabled = true;
	mgr_data->ovls[0].cfg.stride = DEFAULT_STRIDE;
	mgr_data->ovls[0].cfg.pre_mult_alpha = 0;
	mgr_data->ovls[0].cfg.global_alpha = 255;
	mgr_data->ovls[0].cfg.rotation = 0;
	mgr_data->ovls[0].cfg.mirror = 0;
	mgr_data->ovls[0].cfg.ilace = OMAP_DSS_ILACE_NONE;

	/* VID1 PIPELINE */
	mgr_data->ovls[1].cfg.zorder = vid1_zorder;
	mgr_data->ovls[1].cfg.zonly = 0;
	mgr_data->ovls[1].cfg.ix = VID1_PIPELINE;
	mgr_data->ovls[1].cfg.color_mode = OMAP_DSS_COLOR_ARGB32;
	mgr_data->ovls[1].cfg.width = COLOR_TEST_BOX_WIDTH;
	mgr_data->ovls[1].cfg.height = COLOR_TEST_BOX_HEIGHT;
	mgr_data->ovls[1].cfg.win.x = 75;
	mgr_data->ovls[1].cfg.win.y = 75;
	mgr_data->ovls[1].cfg.win.w = COLOR_TEST_BOX_WIDTH;
	mgr_data->ovls[1].cfg.win.h = COLOR_TEST_BOX_HEIGHT;
	mgr_data->ovls[1].cfg.crop.x = 0;
	mgr_data->ovls[1].cfg.crop.y = 0;
	mgr_data->ovls[1].cfg.crop.w = COLOR_TEST_BOX_WIDTH;
	mgr_data->ovls[1].cfg.crop.h = COLOR_TEST_BOX_HEIGHT;
	mgr_data->ovls[1].cfg.enabled = true;
	mgr_data->ovls[1].cfg.stride = DEFAULT_STRIDE;
	mgr_data->ovls[1].cfg.pre_mult_alpha = 0;
	mgr_data->ovls[1].cfg.global_alpha = 255;
	mgr_data->ovls[1].cfg.rotation = 0;
	mgr_data->ovls[1].cfg.mirror = 0;
	mgr_data->ovls[1].cfg.ilace = OMAP_DSS_ILACE_NONE;

	/* VID2 PIPELINE */
	mgr_data->ovls[2].cfg.zorder = vid2_zorder;
	mgr_data->ovls[2].cfg.zonly = 0;
	mgr_data->ovls[2].cfg.ix = VID2_PIPELINE;
	mgr_data->ovls[2].cfg.color_mode = OMAP_DSS_COLOR_ARGB32;
	mgr_data->ovls[2].cfg.width = COLOR_TEST_BOX_WIDTH;
	mgr_data->ovls[2].cfg.height = COLOR_TEST_BOX_HEIGHT;
	mgr_data->ovls[2].cfg.win.x = 125;
	mgr_data->ovls[2].cfg.win.y = 125;
	mgr_data->ovls[2].cfg.win.w = COLOR_TEST_BOX_WIDTH;
	mgr_data->ovls[2].cfg.win.h = COLOR_TEST_BOX_HEIGHT;
	mgr_data->ovls[2].cfg.crop.x = 0;
	mgr_data->ovls[2].cfg.crop.y = 0;
	mgr_data->ovls[2].cfg.crop.w = COLOR_TEST_BOX_WIDTH;
	mgr_data->ovls[2].cfg.crop.h = COLOR_TEST_BOX_HEIGHT;
	mgr_data->ovls[2].cfg.enabled = true;
	mgr_data->ovls[2].cfg.stride = DEFAULT_STRIDE;
	mgr_data->ovls[2].cfg.pre_mult_alpha = 0;
	mgr_data->ovls[2].cfg.global_alpha = 255;
	mgr_data->ovls[2].cfg.rotation = 0;
	mgr_data->ovls[2].cfg.mirror = 0;
	mgr_data->ovls[2].cfg.ilace = OMAP_DSS_ILACE_NONE;

	/* VID3 PIPELINE */
	mgr_data->ovls[3].cfg.zorder = vid3_zorder;
	mgr_data->ovls[3].cfg.zonly = 0;
	mgr_data->ovls[3].cfg.ix = VID3_PIPELINE;
	mgr_data->ovls[3].cfg.color_mode = OMAP_DSS_COLOR_ARGB32;
	mgr_data->ovls[3].cfg.width = COLOR_TEST_BOX_WIDTH;
	mgr_data->ovls[3].cfg.height = COLOR_TEST_BOX_HEIGHT;
	mgr_data->ovls[3].cfg.win.x = 175;
	mgr_data->ovls[3].cfg.win.y = 175;
	mgr_data->ovls[3].cfg.win.w = COLOR_TEST_BOX_WIDTH;
	mgr_data->ovls[3].cfg.win.h = COLOR_TEST_BOX_HEIGHT;
	mgr_data->ovls[3].cfg.crop.x = 0;
	mgr_data->ovls[3].cfg.crop.y = 0;
	mgr_data->ovls[3].cfg.crop.w = COLOR_TEST_BOX_WIDTH;
	mgr_data->ovls[3].cfg.crop.h = COLOR_TEST_BOX_HEIGHT;
	mgr_data->ovls[3].cfg.enabled = true;
	mgr_data->ovls[3].cfg.stride = DEFAULT_STRIDE;
	mgr_data->ovls[3].cfg.pre_mult_alpha = 0;
	mgr_data->ovls[3].cfg.global_alpha = 255;
	mgr_data->ovls[3].cfg.rotation = 0;
	mgr_data->ovls[3].cfg.mirror = 0;
	mgr_data->ovls[3].cfg.ilace = OMAP_DSS_ILACE_NONE;

}

static int test_mem_alloc(struct dsscomp_setup_mgr_data *mgr_data)
{
	int fd, i, ret = 0;
	size_t len;
	int map_fd;
	struct ion_handle *handle;
	static unsigned char *ptr;
	static size_t stride = 4096;

	/* Open Ion Device */
	fd = open("/dev/ion", O_RDWR);
    if (fd < 0) {
    	printf("Error: open /dev/ion failed!\n");
    	return -EINVAL;
	}

	for (i = 0; i < mgr_data->num_ovls; i++) {
		ret = ion_alloc_tiler(fd, mgr_data->ovls[i].cfg.width,
				mgr_data->ovls[i].cfg.height,
				TILER_PIXEL_FMT_32BIT, 0, &handle, &stride);

		if (!ret) {
			len = mgr_data->ovls[i].cfg.height * stride;
			ret = ion_map(fd, handle, len, PROT_READ | PROT_WRITE,
						MAP_SHARED, 0, &ptr, &map_fd);

			/* Fill the buffer address and stride info as provided by Ion */
			mgr_data->ovls[i].address = (void *) ptr;
			mgr_data->ovls[i].uv_address = NULL;
			mgr_data->ovls[i].cfg.stride = stride;
		} else {
			printf("Error: ion_alloc_tiler failed ret=0x%x!\n", ret);
			break;
		}
	}

	/* Close the ion device */
	close(fd);

	return ret;
}

static int fill_image(struct dsscomp_setup_mgr_data *mgr_data)
{
	int i, j, k;
	unsigned int *temp;

	for (i = 0; i < NO_OF_PIPELINES; i++) {

		temp = (unsigned int *) mgr_data->ovls[i].address;
		if (temp == NULL) {
			printf("Error: NULL pointer mgr_data->ovls[%d].address=%p\n",
						i, temp);
			return -EINVAL;
		}

		for (j = 0; j < mgr_data->ovls[i].cfg.height; j++) {
			unsigned int ch;

			for (k = 0; k < mgr_data->ovls[i].cfg.width; k++) {
				if (i == GFX_PIPELINE)
					ch = 0xFFFFFFFF;
				else if (i == VID1_PIPELINE)
					ch = 0xFFFF0000;
				else if (i == VID2_PIPELINE)
					ch = 0xFF00FF00;
				else if (i == VID3_PIPELINE)
					ch = 0xFF000000;
				else
					return -EINVAL;

	    		*temp++ = ch;
			}
			temp += ((mgr_data->ovls[i].cfg.stride >> 2) -
										mgr_data->ovls[i].cfg.width);
		}
	}

	return 0;
}

/* Play video file or display a 2D image */
int test_all_ovls(struct dsscomp_setup_mgr_data *mgr_data,
				unsigned int gfx_zorder,  unsigned int vid1_zorder,
				unsigned int vid2_zorder,  unsigned int vid3_zorder)
{
	int ret = 0;

	if ((gfx_zorder == vid1_zorder) ||
			(gfx_zorder == vid2_zorder) ||
			(gfx_zorder == vid3_zorder) ||
			(vid1_zorder == vid2_zorder) ||
			(vid1_zorder == vid3_zorder) ||
			(vid2_zorder == vid3_zorder)) {
		printf("Error: invalid zorder: gfx_zorder = %d, "
				"vid1_zorder = %d, vid2_zorder = %d, vid3_zorder = %d, \n",
				gfx_zorder, vid1_zorder, vid2_zorder, vid3_zorder);

		return -EINVAL;
	}

	setup_all_ovls(mgr_data, gfx_zorder, vid1_zorder, vid2_zorder,
				vid3_zorder);

	ret = test_mem_alloc(mgr_data);
	if (ret) {
		printf("Error: test_mem_alloc failed\n");
		return ret;
	}

	ret = fill_image(mgr_data);
	if (ret) {
		printf("Error: fill_image failed\n");
		return ret;
	}

	/* Update the composition & Display */
	ret = comp_update(mgr_data);
	if (ret)
		printf("Error: composition update failed!\n");

	return ret;
}

/* Fill the color boxes for testing pre_mult_alpha */
static int premult_fill_image(struct dsscomp_setup_mgr_data *mgr_data)
{
	int i, j, k;
	unsigned int *temp;

	for (i = 0; i < mgr_data->num_ovls; i++) {

		temp = (unsigned int *) mgr_data->ovls[i].address;
		if (temp == NULL) {
			printf("Error: NULL pointer mgr_data->ovls[%d].address=%p\n",
						i, temp);
			return -EINVAL;
		}

		for (j = 0; j < mgr_data->ovls[i].cfg.height; j++) {
			unsigned int ch;

			for (k = 0; k < mgr_data->ovls[i].cfg.width; k++) {
				if (i == GFX_PIPELINE) {
					ch = 0xFF0000FF;
				}
				else if (i == VID1_PIPELINE)
					ch = 0x7F00FF00;
				else
					return -EINVAL;

				*temp++ = ch;
			}
			temp += ((mgr_data->ovls[i].cfg.stride >> 2) -
					mgr_data->ovls[i].cfg.width);
		}
	}

	return 0;
}


/* Pre-mult-alpha test case */
int test_premult_alpha(struct dsscomp_setup_mgr_data *mgr_data)
{
	int ret = 0,i;

	for(i = 0; i < 6; i++) {

		setup_all_ovls(mgr_data, 0, 2, 1, 3);

		mgr_data->num_ovls = 2;

		mgr_data->ovls[0].cfg.width = COLOR_TEST_BOX_WIDTH;
		mgr_data->ovls[0].cfg.win.w = COLOR_TEST_BOX_WIDTH;
		mgr_data->ovls[0].cfg.crop.w = COLOR_TEST_BOX_WIDTH;
		mgr_data->ovls[0].cfg.height = COLOR_TEST_BOX_HEIGHT;
		mgr_data->ovls[0].cfg.win.h = COLOR_TEST_BOX_HEIGHT;
		mgr_data->ovls[0].cfg.crop.h = COLOR_TEST_BOX_HEIGHT;

		mgr_data->ovls[1].cfg.win.x = 200;
		mgr_data->ovls[1].cfg.win.y = 200;

		mgr_data->ovls[1].cfg.width = COLOR_TEST_BOX_WIDTH;
		mgr_data->ovls[1].cfg.win.w = COLOR_TEST_BOX_WIDTH;
		mgr_data->ovls[1].cfg.crop.w = COLOR_TEST_BOX_WIDTH;
		mgr_data->ovls[1].cfg.height = COLOR_TEST_BOX_HEIGHT;
		mgr_data->ovls[1].cfg.win.h = COLOR_TEST_BOX_HEIGHT;
		mgr_data->ovls[1].cfg.crop.h = COLOR_TEST_BOX_HEIGHT;

		mgr_data->ovls[1].cfg.pre_mult_alpha = i & 1;

		ret = test_mem_alloc(mgr_data);
		if (ret) {
			printf("Error: test_mem_alloc failed\n");
			return ret;
		}

		ret = premult_fill_image(mgr_data);
		if (ret) {
			printf("Error: fill_image failed\n");
			return ret;
		}

		/* Update the composition & Display */
		ret = comp_update(mgr_data);
		if (ret)
			printf("Error: composition update failed!\n");

		WAIT_CODE;
	}

	return ret;
}

/* Fill the color boxes for testing trans color key */
static int trans_key_fill_image(struct dsscomp_setup_mgr_data *mgr_data)
{
	int i, j, k;
	unsigned int *temp;

	for (i = 0; i < mgr_data->num_ovls; i++) {

		temp = (unsigned int *) mgr_data->ovls[i].address;
		if (temp == NULL) {
			printf("Error: NULL pointer mgr_data->ovls[%d].address=%p\n",
						i, temp);
			return -EINVAL;
		}

		for (j = 0; j < mgr_data->ovls[i].cfg.height; j++) {
			unsigned int ch;

			for (k = 0; k < mgr_data->ovls[i].cfg.width; k++) {
				if (i == GFX_PIPELINE)
					ch = 0xFFFFFFFF;
				else if (i == VID1_PIPELINE)
					ch = 0xFF0000FF;
				else
					return -EINVAL;

				*temp++ = ch;
			}
			temp += ((mgr_data->ovls[i].cfg.stride >> 2) -
					mgr_data->ovls[i].cfg.width);
		}
	}

	for (i = 0; i < mgr_data->num_ovls; i++) {
		temp = (unsigned int *) mgr_data->ovls[i].address;

		temp += ( 50 * (mgr_data->ovls[i].cfg.stride >> 2) + 200);
		for (j = 50; j < 200; j++) {
			unsigned int ch;
			for (k = 50; k < 200; k++) {
				if (i == GFX_PIPELINE)
					ch = 0xFF000000;
				else if (i == VID1_PIPELINE)
					ch = 0xFF00FF00;
				else
					return -EINVAL;

				*temp++ = ch;
			}
			temp += ((mgr_data->ovls[i].cfg.stride >> 2) - 150);
		}
	}

	return 0;
}

/* Transcolor key test case */
int test_trans_key(struct dsscomp_setup_mgr_data *mgr_data)
{
	int ret = 0;

	setup_all_ovls(mgr_data, 2, 0, 1, 0);

	mgr_data->num_ovls = 2;
	mgr_data->mgr.trans_key_type = OMAP_DSS_COLOR_KEY_VID_SRC;
	mgr_data->mgr.trans_key = 0x00000000;
	mgr_data->mgr.trans_enabled = true;
	mgr_data->mgr.alpha_blending = 0;

	mgr_data->ovls[0].cfg.color_mode = OMAP_DSS_COLOR_RGBX32;
	mgr_data->ovls[0].cfg.width = COLOR_TEST_BOX_WIDTH;
	mgr_data->ovls[0].cfg.win.w = COLOR_TEST_BOX_WIDTH;
	mgr_data->ovls[0].cfg.crop.w = COLOR_TEST_BOX_WIDTH;
	mgr_data->ovls[0].cfg.height = COLOR_TEST_BOX_HEIGHT;
	mgr_data->ovls[0].cfg.win.h = COLOR_TEST_BOX_HEIGHT;
	mgr_data->ovls[0].cfg.crop.h = COLOR_TEST_BOX_HEIGHT;

	mgr_data->ovls[1].cfg.win.x = 100;
	mgr_data->ovls[1].cfg.win.y = 100;

	mgr_data->ovls[1].cfg.width = COLOR_TEST_BOX_WIDTH;
	mgr_data->ovls[1].cfg.win.w = COLOR_TEST_BOX_WIDTH;
	mgr_data->ovls[1].cfg.crop.w = COLOR_TEST_BOX_WIDTH;
	mgr_data->ovls[1].cfg.height = COLOR_TEST_BOX_HEIGHT;
	mgr_data->ovls[1].cfg.win.h = COLOR_TEST_BOX_HEIGHT;
	mgr_data->ovls[1].cfg.crop.h = COLOR_TEST_BOX_HEIGHT;

	ret = test_mem_alloc(mgr_data);
	if (ret) {
		printf("Error: test_mem_alloc failed\n");
		return ret;
	}

	ret = trans_key_fill_image(mgr_data);
	if (ret) {
		printf("Error: fill_image failed\n");
		return ret;
	}

	/* Update the composition & Display */
	ret = comp_update(mgr_data);
	if (ret)
		printf("Error: composition update failed!\n");

	WAIT_CODE;

	return ret;
}
