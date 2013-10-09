#include "dsscomp_test.h"

static unsigned char *y_addr = NULL, *uv_address = NULL;
static size_t stride_uv = 4096;
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

	/* Get y pointer */
	ret = ion_alloc_tiler(fd, width, height,
					TILER_PIXEL_FMT_8BIT, 0, &handle, &stride_y);

	if (!ret) {
		len = height * stride_y;
		ret = ion_map(fd, handle, len, PROT_READ | PROT_WRITE,
					MAP_SHARED, 0, &y_addr, &map_fd);
	} else {
		printf("Error: ion_alloc_tiler failed for y_addr ret=0x%x!\n", ret);
		goto err_done;
	}

	/* Get uv pointer */
	ret = ion_alloc_tiler(fd, width >> 1, height >> 1,
					TILER_PIXEL_FMT_16BIT, 0, &handle, &stride_uv);

	if (!ret) {
		len = (height >> 1) * stride_uv;
		ret = ion_map(fd, handle, len, PROT_READ | PROT_WRITE,
				MAP_SHARED, 0, &uv_address, &map_fd);
	} else {
		printf("Error: ion_alloc_tiler failed for uv_address ret=0x%x!\n", ret);
	}

err_done:
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
	if (!y_addr || !uv_address) {
		printf("Error: NULL image pointer y_addr=%p uv_address=%p\n",
					y_addr, uv_address);
		ret = -EINVAL;
	} else {
		unsigned char *temp = y_addr;

		/* Fill the y contents */
		for (i = 0; i < height; i++) {
			fread(temp, width, 1, image_fp);
			temp += stride_y;
		}

		/* Fill the uv contents */
		temp = uv_address;
		for (i = 0; i < height >> 1; i++) {
			fread(temp, width >> 1, 2, image_fp);
			temp += stride_uv;
		}
	}

	/* Close the Image file */
	fclose(image_fp);

	return ret;
}

static int alloc_fill_data(struct dsscomp_setup_mgr_data *mgr_data)
{
	int ret = 0;
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
	mgr_data->ovls[0].uv_address = uv_address;
	mgr_data->ovls[0].cfg.stride = stride_y;

	/* Update the composition & Display */
	ret = comp_update(mgr_data);
	if (ret)
		printf("Error: composition update failed!\n");

	return ret;
}

/* Display a 2D image */
int test_nv12(struct dsscomp_setup_mgr_data *mgr_data)
{
	file_name = NV12_TEST_FILE_NAME;
	mgr_data->ovls[0].cfg.color_mode = OMAP_DSS_COLOR_NV12;

	return (alloc_fill_data(mgr_data));
}
