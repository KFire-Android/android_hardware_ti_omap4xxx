#include "dsscomp_test.h"

static unsigned char *ptr = NULL;
static size_t stride = 4096;

static int test_tiler_alloc(int width, int height)
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

	ret = ion_alloc_tiler(fd, (width * height) << 2, 1,
					TILER_PIXEL_FMT_PAGE, 0, &handle, &stride);

	if (!ret) {
		len = (height * width) << 2;
		ret = ion_map(fd, handle, len, PROT_READ | PROT_WRITE,
					MAP_SHARED, 0, &ptr, &map_fd);
	} else {
		printf("Error: ion_alloc_tiler failed!\n");
	}

	stride = width * sizeof(int);

	/* Close the ion device */
	close(fd);

	return ret;
}

static int test_non_tiler_alloc(int width, int height)
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

	len = (((width * height) << 2) + 0x2000) & 0xFFFFE000;
	ret = ion_alloc(fd, len, 0x1000, 1 << ION_HEAP_TYPE_CARVEOUT,
					&handle);

	if (!ret) {
		ret = ion_map(fd, handle, len, PROT_READ | PROT_WRITE,
					MAP_SHARED, 0, &ptr, &map_fd);
	} else {
		printf("Error: ion_alloc_tiler failed ret=0x%x!\n", ret);
	}

	stride = width * sizeof(int);

	/* Close the ion device */
	close(fd);

	return ret;
}

static int fill_image_1D(int width, int height)
{
	FILE *image_fp;
	int i, j, ret = 0;

	/* Open Image file */
	image_fp = fopen(file_name, "r");
	if (!image_fp) {
		printf("Error: cannot open input image file\n");
		return -EINVAL;
	}

	/* Read the Image and place it in buffer */
	if (ptr) {
		unsigned int *temp = (unsigned int *) ptr;

		for (i = 0; i < height; i++) {
			for (j = 0; j < width; j++) {
				if (fread(temp, 1, 4, image_fp) == 4)
						*temp++ |= 0xFF000000;
			}
		}
	} else {
		printf("Error: NULL pointer ptr=%p\n", ptr);
		ret = -EINVAL;
	}

	/* Close the Image file */
	fclose(image_fp);

	return ret;
}

/* use non tiler 1D buffer */
int test_non_tiler_buf(struct dsscomp_setup_mgr_data *mgr_data)
{
	int ret = 0;

	/* Test with non tiler buffer */
	ret = test_non_tiler_alloc(mgr_data->ovls[0].cfg.width,
					mgr_data->ovls[0].cfg.height);
	if (ret) {
		printf("Error: test_non_tiler_alloc failed");
		return ret;
	}

	ret = fill_image_1D(mgr_data->ovls[0].cfg.width,
					mgr_data->ovls[0].cfg.height);
	if (ret) {
		printf("Error: fill_image failed");
		return ret;
	}

	/* Fill the buffer address and stride info as provided by Ion */
	mgr_data->ovls[0].address = ptr;
	mgr_data->ovls[0].uv_address = NULL;
	mgr_data->ovls[0].cfg.stride = stride;

	/* Update the composition & Display */
	ret = comp_update(mgr_data);
	if (ret)
		printf("Error: composition update failed!\n");

	return ret;
}

/* use tiler 1D buffer */
int test_tiler_1d_buf(struct dsscomp_setup_mgr_data *mgr_data)
{
	int ret = 0;

	/* Test with tiler 1D buffer */
	ret = test_tiler_alloc(mgr_data->ovls[0].cfg.width,
					mgr_data->ovls[0].cfg.height);
	if (ret) {
		printf("Error: test_tiler_alloc failed");
		return ret;
	}

	ret = fill_image_1D(mgr_data->ovls[0].cfg.width,
					mgr_data->ovls[0].cfg.height);
	if (ret) {
		printf("Error: fill_image failed");
		return ret;
	}

	/* Fill the buffer address and stride info as provided by Ion */
	mgr_data->ovls[0].address = ptr;
	mgr_data->ovls[0].uv_address = NULL;
	mgr_data->ovls[0].cfg.stride = stride;

	/* Update the composition & Display */
	ret = comp_update(mgr_data);
	if (ret)
		printf("Error: composition update failed!\n");

	return ret;
}

/*
 * Use tiler 2D buffer
 * All other test cases use tiler 2D buffers
 * hence not written as a separate test case
 */
 /* int test_tiler_2d_buf(struct dsscomp_setup_mgr_data *mgr_data) {} */
