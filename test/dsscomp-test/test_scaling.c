#include "dsscomp_test.h"

static unsigned char *ptr = NULL;
static size_t stride = 4096;

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

	ret = ion_alloc_tiler(fd, width, height,
					TILER_PIXEL_FMT_32BIT, 0, &handle, &stride);

	if (!ret) {
		len = height * stride;
		ret = ion_map(fd, handle, len, PROT_READ | PROT_WRITE,
					MAP_SHARED, 0, &ptr, &map_fd);
	} else {
		printf("Error: ion_alloc_tiler failed ret=0x%x!\n", ret);
	}

	/* Close the ion device */
	close(fd);

	return ret;
}

static int fill_image(int width, int height)
{
	FILE *image_fp;
	int i, j, ret = 0;;

	/* Open Image file */
	image_fp = fopen(file_name, "r");
	if (!image_fp) {
		printf("Error: cannot open input image file\n");
		return -EINVAL;
	}

	/* Read the Image and place it in buffer */
	if (ptr) {
		unsigned int *temp = (unsigned int *) ptr;

		for (i= 0; i < height; i++) {
			for (j= 0; j < width; j++) {
				if (fread(temp, 1, 4, image_fp) == 4)
						*temp++ |= 0xFF000000;
			}
			temp += (stride >> 2) - width;
		}
	} else {
		printf("Error: NULL pointer ptr=%p\n", ptr);
		ret = -EINVAL;
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
	/* Fill the buffer address and stride info as provided by Ion */
	mgr_data->ovls[0].address = ptr;
	mgr_data->ovls[0].uv_address = NULL;
	mgr_data->ovls[0].cfg.stride = stride;

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
	mgr_data->ovls[0].address = ptr;
	mgr_data->ovls[0].uv_address = NULL;
	mgr_data->ovls[0].cfg.stride = stride;

	/* Update the composition & Display */
	ret = comp_update(mgr_data);

	if (!ret)
		printf("Scaled Image displayed!\n");
	else
		printf("Error: composition update failed!\n");

	return ret;
}

/* Play video file or display a 2D image */
int test_scaling(struct dsscomp_setup_mgr_data *mgr_data,
						unsigned int sampling_factor,
						unsigned int sampling_cordinate, bool up,
						bool exceed_disp_res)
{
	int ret = 0;

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
