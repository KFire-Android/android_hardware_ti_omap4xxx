#include "dsscomp_test.h"

static unsigned char *y_addr = NULL;
static size_t stride_y = 4096;

static int test_mem_alloc(int width, int height)
{
	int fd, ret = 0;
	size_t len;
	int map_fd;
	struct ion_handle *handle;

	/* Open Ion Device */
	fd = open("/dev/ion", O_RDWR);
	if (fd < 0) {
    		printf("Error: open /dev/ion failed!\n");
	    	return -EINVAL;
	}

	ret = ion_alloc_tiler(fd, (width * height) << 1, 1,
					TILER_PIXEL_FMT_PAGE, 0, &handle,
					&stride_y);

	if (!ret) {
		len = (height * width) << 1;
		ret = ion_map(fd, handle, len, PROT_READ | PROT_WRITE,
					MAP_SHARED, 0, &y_addr, &map_fd);
	} else {
		printf("Error: ion_alloc_tiler failed!\n");
	}

	stride_y = width * 2;

	/* Close the ion device */
	close(fd);

	return ret;
}

static int fill_image(int width, int height)
{
	FILE *image_fp;
	int i, ret = 0;

	/* Open Image file */
	image_fp = fopen(file_name, "r");
	if (!image_fp) {
		printf("Error: cannot open input image file\n");
		return -EINVAL;
	}

	/* Read the Image and place it in buffer */
	if (!y_addr) {
		printf("Error: NULL image pointer y_addr=%p\n",
					y_addr);
		ret = -EINVAL;
	} else {
		unsigned char *temp = y_addr;

		/* Fill the content */
		for (i = 0; i < height; i++) {
			int j;
			for (j = 0; j < (width); j++) {
				fread(temp, 1, 2, image_fp);
				temp += 2;
			}
		}
	}

	/* Close the Image file */
	fclose(image_fp);

	return ret;
}

static int display_input_image(struct dsscomp_setup_mgr_data *mgr_data,
					unsigned int sampling_factor,
					unsigned int sampling_cordinate, bool up)
{
	int ret = 0;

	/* Get the image to be displayed */
	if (sampling_factor) {
		/* Downscale the image */
		if (sampling_cordinate == 0) {
			if (up) {
				mgr_data->ovls[0].cfg.crop.w =
							mgr_data->ovls[0].cfg.crop.w / sampling_factor;

				mgr_data->ovls[0].cfg.win.w =
							mgr_data->ovls[0].cfg.win.w / sampling_factor;
			}
		} else if (sampling_cordinate == 1) {
			if (up) {
				mgr_data->ovls[0].cfg.crop.h =
							mgr_data->ovls[0].cfg.crop.h / sampling_factor;

				mgr_data->ovls[0].cfg.win.h =
							mgr_data->ovls[0].cfg.win.h / sampling_factor;
			}
		} else if (sampling_cordinate == 2) {
			if (up) {
				mgr_data->ovls[0].cfg.crop.w =
							mgr_data->ovls[0].cfg.crop.w / sampling_factor;

				mgr_data->ovls[0].cfg.win.w =
							mgr_data->ovls[0].cfg.win.w / sampling_factor;

				mgr_data->ovls[0].cfg.crop.h =
							mgr_data->ovls[0].cfg.crop.h / sampling_factor;

				mgr_data->ovls[0].cfg.win.h =
							mgr_data->ovls[0].cfg.win.h / sampling_factor;
			}
		}
	}

	printf("Displaying input image...\n");

	ret = test_mem_alloc(mgr_data->ovls[0].cfg.width,
					mgr_data->ovls[0].cfg.height);
	if (ret) {
		printf("Error: test_mem_alloc failed\n");
		return ret;
	}

	ret = fill_image(mgr_data->ovls[0].cfg.width,
					mgr_data->ovls[0].cfg.height);
	if (ret) {
		printf("Error: fill_image failed\n");
		return ret;
	}
	mgr_data->ovls[0].address = y_addr;
	mgr_data->ovls[0].uv_address = NULL;
	mgr_data->ovls[0].cfg.stride = stride_y;

	/* Update the composition & Display */
	ret = comp_update(mgr_data);

	if (!ret) {
		printf("Input Image displayed!\n");
		WAIT_CODE
	} else {
		printf("Error: composition update failed!\n");
	}

	return ret;
}


static int display_scaled_image(struct dsscomp_setup_mgr_data *mgr_data,
					unsigned int sampling_factor,
					unsigned int sampling_cordinate, bool up)
{
	int ret = 0;

	/* Get the scaling parameters */
	if (sampling_factor) {
		if (sampling_cordinate == 0) {
			if (up) {
				mgr_data->ovls[0].cfg.win.w =
					mgr_data->ovls[0].cfg.win.w	* sampling_factor;
			} else {
				mgr_data->ovls[0].cfg.win.w =
					mgr_data->ovls[0].cfg.win.w / sampling_factor;
			}
		} else if (sampling_cordinate == 1) {
			if (up) {
				mgr_data->ovls[0].cfg.win.h =
					mgr_data->ovls[0].cfg.win.h	* sampling_factor;
			} else {
				mgr_data->ovls[0].cfg.win.h =
					mgr_data->ovls[0].cfg.win.h / sampling_factor;
			}
		} else if (sampling_cordinate == 2) {
			if (up) {
				mgr_data->ovls[0].cfg.win.w =
					mgr_data->ovls[0].cfg.win.w	* sampling_factor;

				mgr_data->ovls[0].cfg.win.h =
					mgr_data->ovls[0].cfg.win.h	* sampling_factor;
			} else {
				mgr_data->ovls[0].cfg.win.w =
					mgr_data->ovls[0].cfg.win.w	/ sampling_factor;

				mgr_data->ovls[0].cfg.win.h =
					mgr_data->ovls[0].cfg.win.h / sampling_factor;
			}
		}
	}

	printf("Displaying scaled image...\n");

	ret = test_mem_alloc(mgr_data->ovls[0].cfg.width,
					mgr_data->ovls[0].cfg.height);
	if (ret) {
		printf("Error: test_mem_alloc failed\n");
		return ret;
	}

	ret = fill_image(mgr_data->ovls[0].cfg.width,
					mgr_data->ovls[0].cfg.height);
	if (ret) {
		printf("Error: fill_image failed\n");
		return ret;
	}

	/* Fill the buffer address and stride info as provided by Ion */
	mgr_data->ovls[0].address = y_addr;
	mgr_data->ovls[0].uv_address = NULL;
	mgr_data->ovls[0].cfg.stride = stride_y;

	/* Update the composition & Display */
	ret = comp_update(mgr_data);

	if (!ret)
		printf("Scaled Image displayed!\n");
	else
		printf("Error: composition update failed!\n");

	return ret;
}

/* Play video file or display a 2D image */
int test_uyvy_scaling(struct dsscomp_setup_mgr_data *mgr_data,
						unsigned int sampling_factor,
						unsigned int sampling_cordinate, bool up,
						bool exceed_disp_res)
{
	int ret = 0;

	file_name = UYVY_TEST_FILE_NAME;
	mgr_data->ovls[0].cfg.color_mode = OMAP_DSS_COLOR_UYVY;

	if (!exceed_disp_res) {
		ret = display_input_image(mgr_data, sampling_factor,
								sampling_cordinate, up);
		if (ret) {
			printf("Error: display input image failed\n");
			return ret;
		}
	}

	ret = display_scaled_image(mgr_data, sampling_factor, sampling_cordinate,
								up);

	/*printf("output width=%d output height=%d\n", mgr_data->ovls[0].cfg.win.w,
								mgr_data->ovls[0].cfg.win.h); */
	if (ret)
		printf("Error: display scaled image failed\n");

	return ret;
}
