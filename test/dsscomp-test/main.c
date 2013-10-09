#include "dsscomp_test.h"

char *file_name = ARGB_TEST_FILE_NAME;
static unsigned int disp_width = 0, disp_height = 0;

static unsigned int input_w = DEFAULT_IN_WIDTH, input_h = DEFAULT_IN_HEIGHT;
static unsigned int disp_out = DEFAULT_DISPLAY;
static unsigned int pipeline = VID2_PIPELINE;
static unsigned int test = 4;
static unsigned int buf_type = BUF_TILER2D;
static unsigned int zorder = 2;
static unsigned int gfx_zorder = 0;
static unsigned int vid1_zorder = 2;
static unsigned int vid2_zorder = 1;
static unsigned int vid3_zorder = 3;
static unsigned int sampling_factor = 0;
static unsigned int sampling_cordinate = 0;
static int dsscomp_fd;
static bool up;
static enum omap_color_mode color_mode = DEFAULT_COLOR_MODE;

int comp_update(struct dsscomp_setup_mgr_data *s)
{
	//printf("mgr.id=%d no_of_ovls=%d zorder=%d pipeline=%d\n",
		//s->mgr.ix, s->num_ovls, s->ovls[0].cfg.zorder, s->ovls[0].cfg.ix);
	return ioctl(dsscomp_fd, DSSCIOC_SETUP_MGR, s);
}

int query_display(struct _qdis *d)
{
	return ioctl(dsscomp_fd, DSSCIOC_QUERY_DISPLAY, d);
}

int chk_ovl(struct dsscomp_check_ovl_data *c)
{
	return ioctl(dsscomp_fd, DSSCIOC_CHECK_OVL, c);
}

static int get_display_resolution(struct dsscomp_setup_mgr_data *mgr_data)
{
	int ret = 0;
	struct _qdis d = { .display_info = { .ix = 0 } };

	d.display_info.modedb_len = sizeof(d.modedb) / sizeof(*d.modedb);

	d.display_info.ix = disp_out;

	ret = query_display(&d);
	if (ret) {
		printf("Error: query display failed!\n");
		return ret;
	}

	disp_width = d.display_info.timings.x_res;
	disp_height = d.display_info.timings.y_res;
	if (!disp_width || !disp_height) {
		printf("Error: wrong display resolution!! display width = %d display "
					"height = %d\n", disp_width, disp_height);
		return -EINVAL;
	}

	return 0;
}

static void fill_default_setup_mgr_data(unsigned int pipeline,
			struct dsscomp_setup_mgr_data *mgr_data)
{
	struct timeval tv;
	static __u32 sync_id;

	gettimeofday(&tv, NULL);
	sync_id = tv.tv_usec / 476 + tv.tv_sec * 2100840;

	mgr_data->sync_id = ++sync_id;
	mgr_data->num_ovls = 1;
	mgr_data->mode = DSSCOMP_SETUP_DISPLAY;
	mgr_data->get_sync_obj = 0;
	mgr_data->mgr.ix = disp_out;
	mgr_data->mgr.default_color = DEFAULT_COLOR;
	mgr_data->mgr.trans_key_type = OMAP_DSS_COLOR_KEY_VID_SRC;
	mgr_data->mgr.trans_key = 0;
	mgr_data->mgr.trans_enabled = false;
	mgr_data->mgr.interlaced = false;
	mgr_data->mgr.alpha_blending = 1;
	mgr_data->mgr.cpr_enabled = 0;
	mgr_data->mgr.swap_rb = 0;

	mgr_data->ovls[0].cfg.enabled = true;
	mgr_data->ovls[0].cfg.stride = DEFAULT_STRIDE;
	mgr_data->ovls[0].cfg.pre_mult_alpha = 0;
	mgr_data->ovls[0].cfg.global_alpha = 255;
	mgr_data->ovls[0].cfg.rotation = 0;
	mgr_data->ovls[0].cfg.mirror = 0;
	mgr_data->ovls[0].cfg.ilace = OMAP_DSS_ILACE_NONE;
	mgr_data->ovls[0].cfg.zonly = 0;
	mgr_data->ovls[0].cfg.zorder = zorder;
	mgr_data->ovls[0].cfg.ix = pipeline;

	mgr_data->ovls[0].cfg.color_mode = color_mode;
	mgr_data->ovls[0].cfg.width = input_w;
	mgr_data->ovls[0].cfg.height = input_h;

	mgr_data->ovls[0].cfg.win.x = 0;
	mgr_data->ovls[0].cfg.win.y = 0;
	mgr_data->ovls[0].cfg.win.w = input_w;
	mgr_data->ovls[0].cfg.win.h = input_h;

	mgr_data->ovls[0].cfg.crop.x = 0;
	mgr_data->ovls[0].cfg.crop.y = 0;
	mgr_data->ovls[0].cfg.crop.w = input_w;
	mgr_data->ovls[0].cfg.crop.h = input_h;

	mgr_data->ovls[0].cfg.cconv.ry = 298;
	mgr_data->ovls[0].cfg.cconv.rcr = 409;
	mgr_data->ovls[0].cfg.cconv.rcb = 0;
	mgr_data->ovls[0].cfg.cconv.gy = 298;
	mgr_data->ovls[0].cfg.cconv.gcr = -208;
	mgr_data->ovls[0].cfg.cconv.gcb = -100;
	mgr_data->ovls[0].cfg.cconv.by = 298;
	mgr_data->ovls[0].cfg.cconv.bcr = 0;
	mgr_data->ovls[0].cfg.cconv.bcb = 517;
	mgr_data->ovls[0].cfg.cconv.full_range = 0;
}

static int input_parser(int argc, char *argv[],
		struct dsscomp_setup_mgr_data *mgr_data)
{
	int i;
	char *s1, *s2;

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

		} else if (strcmp("pipeline", s1) ==  0) {
			pipeline = atoi(s2);
			mgr_data->ovls[0].cfg.ix = pipeline;
			printf("pipeline number=%d\n", atoi(s2));

		} else if (strcmp("disp_id", s1) ==  0) {
			disp_out = (unsigned int)atoi(s2);
			mgr_data->mgr.ix = disp_out;
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
			mgr_data->ovls[0].cfg.color_mode = 1 << (unsigned int)atoi(s2);
			color_mode = mgr_data->ovls[0].cfg.color_mode;
			printf("color_mode=0x%x\n", mgr_data->ovls[0].cfg.color_mode);

		} else if (strcmp("input_w", s1) ==  0) {
			input_w = 1 << (unsigned int)atoi(s2);
			mgr_data->ovls[0].cfg.width = input_w;
			printf("input_w=%d\n", input_w);

		} else if (strcmp("input_h", s1) ==  0) {
			input_h = (unsigned int)atoi(s2);
			mgr_data->ovls[0].cfg.height = input_h;
			printf("input_h=%d\n", input_h);

		} else if (strcmp("premult_alpha", s1) ==  0) {
			mgr_data->ovls[0].cfg.pre_mult_alpha = (unsigned int)atoi(s2);
			printf("pre_mult_alpha=%d\n", mgr_data->ovls[0].cfg.pre_mult_alpha);

		} else if (strcmp("global_alpha", s1) ==  0) {
			mgr_data->ovls[0].cfg.global_alpha = (unsigned int)atoi(s2);
			printf("global_alpha=%d\n", mgr_data->ovls[0].cfg.global_alpha);

		} else if (strcmp("rotation", s1) ==  0) {
			mgr_data->ovls[0].cfg.rotation = (unsigned int)atoi(s2);
			printf("rotation=%d\n", mgr_data->ovls[0].cfg.rotation);

		} else if (strcmp("mirror", s1) ==  0) {
			mgr_data->ovls[0].cfg.mirror = (unsigned int)atoi(s2);
			printf("mirror=%d\n", mgr_data->ovls[0].cfg.mirror);

		} else if (strcmp("ilace", s1) ==  0) {
			mgr_data->ovls[0].cfg.ilace = (unsigned int)atoi(s2);
			printf("mirror=%d\n", mgr_data->ovls[0].cfg.ilace);

		} else if (strcmp("user_buf", s1) ==  0) {
			if ((unsigned int)atoi(s2) == 1)
				buf_type = BUF_USER;
			printf("mirror=%d\n", buf_type);

		} else if (strcmp("zorder", s1) ==  0) {
			zorder = (unsigned int)atoi(s2);
			mgr_data->ovls[0].cfg.zorder = zorder;
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
			mgr_data->ovls[0].cfg.crop.x = (unsigned int)atoi(s2);
			printf("crop_x=%d\n", mgr_data->ovls[0].cfg.crop.x);

		} else if (strcmp("crop_y", s1) ==  0) {
			mgr_data->ovls[0].cfg.crop.y = (unsigned int)atoi(s2);
			printf("crop_y=%d\n", mgr_data->ovls[0].cfg.crop.y);

		} else if (strcmp("crop_w", s1) ==  0) {
			mgr_data->ovls[0].cfg.crop.w = (unsigned int)atoi(s2);
			printf("crop_w=%d\n", mgr_data->ovls[0].cfg.crop.w);

		} else if (strcmp("crop_h", s1) ==  0) {
			mgr_data->ovls[0].cfg.crop.h = (unsigned int)atoi(s2);
			printf("crop_h=%d\n", mgr_data->ovls[0].cfg.crop.h);

		} else {
			printf("Error: invalid input parameter: %s\n\n", s1);
			return -EINVAL;
		}
	}

	return 0;
}

static int do_test(struct dsscomp_setup_mgr_data *mgr_data)
{
	int ret = -EINVAL;
	int negative_test = 0;

	switch (test) {
	case 1:		/* Simultaneous display on all available displays */
		ret = test_multi_display(mgr_data);
		break;

	case 2:		/* Query display info */
		/*
		 * get_display_resolution and reset_all uses query display info.
		 * exceution has reached here only after query display info was
		 * successful
		 */
		ret = 0;
		break;

	case 3:		/* Check overlay configuration */
		ret = test_check_ovl(mgr_data);
		break;

	case 12:	/* Cropping */
		mgr_data->ovls[0].cfg.win.x = mgr_data->ovls[0].cfg.crop.x;
		mgr_data->ovls[0].cfg.win.y = mgr_data->ovls[0].cfg.crop.y;
		mgr_data->ovls[0].cfg.win.w = mgr_data->ovls[0].cfg.crop.w;
		mgr_data->ovls[0].cfg.win.h = mgr_data->ovls[0].cfg.crop.h;

	case 4:		/* Play video file or display a 2D image */
	case 11:	/* Rotation and/or mirroring */
	case 13:	/* Global alpha blending */
		ret = test_image_display(mgr_data);
		break;

	case 14:	/* Pre-mult-apha test case */
		ret = test_premult_alpha(mgr_data);
		break;



	case 5:		/*
				 * Display content synchronized for each display frame
				 * between the DSS pipelines and the frame-buffer updates
				 * that have occurred for that frame
				 */
	case 15:	/* Queuing multiple content compositions */
	case 18:	/* Mapping of non-tiler buffer into tiler */
	case 20:	/* Various states of the composition */
		printf("FAIL: DSSCOMP - PVR Simulator test case %d yet to be "
				"added\n", test);
		break;

	case 6:
		printf("Verify if any other DSSCOMP works on OMAP4 Blaze board's "
				"primary LCD.\n If yes, this manually updated displays "
				"test case is successful\n");
		ret = 0;
		break;

	case 7:		/* Different color formats */
		/* currently this supports only NV12 format */
	case 21:	/* Rotation of YUV format */
	case 22:	/* Color conversion matrixYCbCr to RGB */
		switch(color_mode) {
		case OMAP_DSS_COLOR_CLUT1:
		case OMAP_DSS_COLOR_CLUT2:
		case OMAP_DSS_COLOR_CLUT4:
		case OMAP_DSS_COLOR_CLUT8:	
		case OMAP_DSS_COLOR_RGB12U:
		case OMAP_DSS_COLOR_ARGB16:
		case OMAP_DSS_COLOR_RGB16:
		case OMAP_DSS_COLOR_RGB24U:
		case OMAP_DSS_COLOR_RGB24P:
		case OMAP_DSS_COLOR_YUV2:
		case OMAP_DSS_COLOR_RGBA32:
		case OMAP_DSS_COLOR_RGBX32:
		case OMAP_DSS_COLOR_RGBA16:
		case OMAP_DSS_COLOR_RGBX12:
		case OMAP_DSS_COLOR_ARGB16_1555:
		case OMAP_DSS_COLOR_XRGB16_1555:
			printf("This color format support test "
					"yet to be added\n");
			break;
		case OMAP_DSS_COLOR_UYVY:
			ret = test_uyvy(mgr_data);
			break;
		case OMAP_DSS_COLOR_ARGB32:
			ret = test_image_display(mgr_data);
			break;
		case OMAP_DSS_COLOR_NV12:
			ret = test_nv12(mgr_data);
			break;
		default:
			printf("Incorrect color format\n");
			break;
		}

		break;

	case 8:		/* Trans color key */
		/*
		 * This test is excluded as in ICS we do not support this with
		 * alpha only mode
		 */
		ret = test_trans_key(mgr_data);
		break;

	case 9:		/* Zorder or multi pipeline test case */
	case 17:	/* Default color with multi pipelines */
		ret = test_all_ovls(mgr_data, gfx_zorder, vid1_zorder,
								vid2_zorder, vid3_zorder);
		break;

	case 10:	/* Up or down scaling test case */
		ret = test_scaling(mgr_data, sampling_factor, sampling_cordinate, up,
							0);
		break;

	case 16:	/* Simultaneous display on all available displays */
		ret = test_dynamic_display(mgr_data);
		break;

	case 19:	/* Any image resolution */
		ret = test_any_resolution(mgr_data, disp_width, disp_width);
		break;

	case 23:	/* Negative test case for supporting only LCD and TV only */
		mgr_data->mgr.ix = 5;
		ret = test_image_display(mgr_data);
		negative_test = 1;
		break;

	case 24:	/* Negative test case for invalid piepline number */
		mgr_data->ovls[0].cfg.ix = NO_OF_PIPELINES + 1;
		ret = test_image_display(mgr_data);
		negative_test = 1;
		break;

	case 25:	/* Negative test case for invalid color format */
		printf("As per design this test case would result in a kernel crash\n");
		WAIT_CODE
		mgr_data->ovls[0].cfg.color_mode = 90;
		ret = test_image_display(mgr_data);
		negative_test = 1;

		break;

	case 26:	/* Negative test case for invalid image width/height */
		mgr_data->ovls[0].cfg.width = 0;
		mgr_data->ovls[0].cfg.height = 0;
		ret = test_image_display(mgr_data);
		/* negative_test = 1; */ /*
								  * Commenting this because for these  settings,
								  * the DSSCOMP driver is designed to return zero
								  */
		break;

	case 27:	/* Negative test case for invalid disp id to query display */
		{
			struct _qdis d = { .display_info = { .ix = 0 } };

			d.display_info.modedb_len = sizeof(d.modedb) / sizeof(*d.modedb);
			d.display_info.ix = 5;
			ret = query_display(&d);
			negative_test = 1;
		}
		break;

	case 28:	/* Negative test case for invalid zorder */
		mgr_data->ovls[0].cfg.zorder = 9;
		ret = test_image_display(mgr_data);
		/* negative_test = 1; */ /*
								  * Commenting this because for these  settings,
								  * the DSSCOMP driver is designed to return zero
								  */
		printf("If you are able to see any error message while executing "
							"this test case, then this test is PASS\n");
		break;

	case 29:	/* Negative test case for invalid trans key settings */
		mgr_data->mgr.trans_key_type = 5;
		mgr_data->mgr.trans_key = 0x88FF0;
		mgr_data->mgr.trans_enabled = true;
		ret = test_all_ovls(mgr_data, gfx_zorder, vid1_zorder,
								vid2_zorder, vid3_zorder);
		/* negative_test = 1; */ /*
								  * Commenting this because for these  settings,
								  * the DSSCOMP driver is designed to return zero
								  */
		break;

	case 30:	/* Scaled output greater than display resolution */
		ret = test_scaling(mgr_data, sampling_factor, sampling_cordinate, up,
							1);
		break;

	case 31:	/* Negative test case for rotation of non-tiler 2D buffer */
		mgr_data->ovls[0].cfg.rotation = 2;
		ret = test_non_tiler_buf(mgr_data);
		/* negative_test = 1; */ /*
								  * Commenting this because for these  settings,
								  * the DSSCOMP driver is designed to return zero
								  */
		printf("If you are able to see any error message while executing "
							"this test case, then this test is PASS\n");
		break;

	case 32:		/*
				 * Not a required as per UTS but does 1D tiler
				 * buffer testing. Use for any debugging.
				 */
		ret = test_tiler_1d_buf(mgr_data);
		break;

	case 33:		/*
				 * Not a required as per UTS but does non tiler
				 * buffer testing. Use for any debugging.
				 * untested. Tested with crashes
				 */
		ret = test_non_tiler_buf(mgr_data);
		break;

	case 34:		/* NV12 scaling test */
		ret = test_nv12_scaling(mgr_data, sampling_factor,
				sampling_cordinate, up, 0);
		break;

	case 35:		/* UYVY scaling test */
		ret = test_uyvy_scaling(mgr_data, sampling_factor,
				sampling_cordinate, up, 0);
		break;

	default:
		printf("FAIL: DSSCOMP Test case %d - invalid test case\n", test);
		break;
	}

	/* If negative test case, non-zero return value is a success */
	if (negative_test) {
		if (ret)
			ret = 0;
		else
			ret = -EINVAL;
	} else {
		/* Get input from user to continue, only for non-negative test cases */
		WAIT_CODE
		//SLEEP_SEC(1);
	}

	return ret;
}

int main(int argc, char *argv[])
{
	int ret = 0;
	struct dsscomp_setup_mgr_data *mgr_data;

	printf("\n");

	/* Allocate manager data buffer */
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

	/* Reset all pipelines */
	ret = reset_all(mgr_data, disp_out);
	if (ret) {
		printf("Error: Reset all pipelines failed");
		goto error_exit;
	}

	ret = get_display_resolution(mgr_data);
	if (ret) {
		printf("Error: Get display resolution failed");
		goto error_exit;
	}

	/* Fill default values */
	fill_default_setup_mgr_data(pipeline, mgr_data);

	ret = input_parser(argc, argv, mgr_data);
	if (ret)
		goto error_exit;

	/* Call the specific test case */
	ret = do_test(mgr_data);
	if (ret) {
		printf("FAILED: DSSCOMP Test case %d Failed!\n", test);
		goto error_exit;
	}

	printf("SUCCESS: DSSCOMP Test case %d done!\n", test);

	display_reset(mgr_data, disp_out);

error_exit:
	/* Close the dsscomp device */
	close(dsscomp_fd);
	printf("\n");
	return 0;
}
