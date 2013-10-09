#include "dsscomp_test.h"

static unsigned char *ptr = NULL;
#define ALLOC_FROM_1D
static size_t stride = 4096;
static size_t bpp = 4;

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

    #if defined ALLOC_FROM_1D
	stride = width;
	#endif
	len = stride * height * bpp;

	ret = ion_alloc(fd, len, 0, 1, 0, &handle);
	if (!ret) {
		ret = ion_map(fd, handle, len, PROT_READ | PROT_WRITE,
					MAP_SHARED, 0, &ptr, &map_fd);
		if (ret)
			printf("ion_map failed err [%d]", errno);
	} else {
		printf("Error: ion_alloc_tiler failed ret=0x%x!\n", ret);
	}

	/* Close the ion device */
	close(fd);

	return ret;
}

static int fill_image(int width, int height, unsigned int pre_mult_alpha)
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

		for (i= 0; i < height; i++) {
			for (j= 0; j < width; j++) {

				fread(temp, 1, 4, image_fp);

				if (!pre_mult_alpha)
					*temp++ |= 0xFF000000;
				else
					temp++;
			}
			temp += ((stride >> 2)*bpp) - width;
		}
	} else {
		printf("Error: NULL pointer ptr=%p\n", ptr);
		ret = -EINVAL;
	}

	/* Close the Image file */
	fclose(image_fp);

	return ret;
}

static int alloc_fill_data(struct dsscomp_setup_mgr_data *mgr_data,
					unsigned int pre_mult_alpha)
{
	int ret = 0;

	if (!mgr_data->ovls[0].cfg.width || !mgr_data->ovls[0].cfg.height) {
		printf("Invalid value width=%d, height=%d\n", mgr_data->ovls[0].cfg.width,
						mgr_data->ovls[0].cfg.height);
		goto alloc_fill_done;
	}

	ret = test_mem_alloc(mgr_data->ovls[0].cfg.width,
					mgr_data->ovls[0].cfg.height);
	if (ret) {
		printf("Error: test_mem_alloc failed\n");
		return ret;
	}

	ret = fill_image(mgr_data->ovls[0].cfg.width,
					mgr_data->ovls[0].cfg.height, pre_mult_alpha);
	if (ret) {
		printf("Error: fill_image failed\n");
		return ret;
	}

alloc_fill_done:
	/* Fill the buffer address and stride info as provided by Ion */
	mgr_data->ovls[0].address = ptr;
	mgr_data->ovls[0].uv_address = NULL;
	mgr_data->ovls[0].cfg.stride = stride*bpp;

	/* Update the composition & Display */
	ret = comp_update(mgr_data);
	if (ret)
		printf("Error: composition update on mgr.id=%d failed!\n", mgr_data->mgr.ix);

	return ret;
}

/* Display a 2D image */
int test_image_display(struct dsscomp_setup_mgr_data *mgr_data)
{
	return (alloc_fill_data(mgr_data, (unsigned int) mgr_data->ovls[0].cfg.pre_mult_alpha));
}

/* Display same image in all the available displays */
int test_multi_display(struct dsscomp_setup_mgr_data *mgr_data)
{
	int ret = 0;

	mgr_data->mgr.ix = PRIM_DISPLAY;

	/* using the pipeline manager id  - no intentions behind this */
	mgr_data->ovls[0].cfg.ix = VID3_PIPELINE;
	mgr_data->ovls[0].cfg.enabled = true;
	mgr_data->ovls[0].cfg.zorder = 3;

	ret = alloc_fill_data(mgr_data, (unsigned int) mgr_data->ovls[0].cfg.pre_mult_alpha);
	if (ret)
		return ret;

	mgr_data->mgr.ix = SEC_DISPLAY;

	/* using the pipeline manager id  - no intentions behind this */
	mgr_data->ovls[0].cfg.ix = VID2_PIPELINE;
	mgr_data->ovls[0].cfg.enabled = true;
	mgr_data->ovls[0].cfg.zorder = 2;

	ret = alloc_fill_data(mgr_data, (unsigned int) mgr_data->ovls[0].cfg.pre_mult_alpha);
	if (ret)
		return ret;

	WAIT_CODE

	/* Reset these pipelines from display managers */
	mgr_data->mgr.ix = PRIM_DISPLAY;
	mgr_data->num_ovls = 1;
	mgr_data->ovls[0].cfg.ix = VID3_PIPELINE;
	mgr_data->ovls[0].cfg.enabled = false;
	mgr_data->ovls[0].cfg.zorder = 3;
	if (comp_update(mgr_data))
		printf("Error: While reseting composition update on mgr.id=%d failed!\n", mgr_data->mgr.ix);

	mgr_data->mgr.ix = SEC_DISPLAY;
	mgr_data->num_ovls = 1;
	mgr_data->ovls[0].cfg.ix = VID2_PIPELINE;
	mgr_data->ovls[0].cfg.enabled = false;
	mgr_data->ovls[0].cfg.zorder = 2;
	if (comp_update(mgr_data))
		printf("Error: While reseting composition update on mgr.id=%d failed!\n", mgr_data->mgr.ix);

	return ret;
}

/* Assign display ids dynamically */
int test_dynamic_display(struct dsscomp_setup_mgr_data *mgr_data)
{
	int ret = 0;

	mgr_data->mgr.ix = PRIM_DISPLAY;
	mgr_data->ovls[0].cfg.ix = VID1_PIPELINE;
	mgr_data->ovls[0].cfg.enabled = true;
	ret = alloc_fill_data(mgr_data, (unsigned int) mgr_data->ovls[0].cfg.pre_mult_alpha);
	if (ret)
		return ret;

	WAIT_CODE
	mgr_data->ovls[0].cfg.enabled = false;
	ret = alloc_fill_data(mgr_data, (unsigned int) mgr_data->ovls[0].cfg.pre_mult_alpha);
	if (ret)
		return ret;

	mgr_data->mgr.ix = SEC_DISPLAY;
	mgr_data->ovls[0].cfg.enabled = true;

	ret = alloc_fill_data(mgr_data, (unsigned int) mgr_data->ovls[0].cfg.pre_mult_alpha);
	if (ret)
		return ret;

	WAIT_CODE
	mgr_data->ovls[0].cfg.enabled = false;
	ret = alloc_fill_data(mgr_data, (unsigned int) mgr_data->ovls[0].cfg.pre_mult_alpha);
	if (ret)
		return ret;

	mgr_data->mgr.ix = PRIM_DISPLAY;
	mgr_data->ovls[0].cfg.enabled = true;
	ret = alloc_fill_data(mgr_data, (unsigned int) mgr_data->ovls[0].cfg.pre_mult_alpha);
	if (ret)
		return ret;

	WAIT_CODE
	mgr_data->ovls[0].cfg.enabled = false;
	ret = alloc_fill_data(mgr_data, (unsigned int) mgr_data->ovls[0].cfg.pre_mult_alpha);
	return ret;
}


/* Display a 2D image with any image resolution */
int test_any_resolution(struct dsscomp_setup_mgr_data *mgr_data,
						unsigned int disp_width, unsigned int disp_height)
{
	mgr_data->ovls[0].cfg.win.w = disp_width;
	mgr_data->ovls[0].cfg.win.h = disp_height;
	mgr_data->ovls[0].cfg.crop.w = disp_width;
	mgr_data->ovls[0].cfg.crop.h = disp_height;

	return (alloc_fill_data(mgr_data, (unsigned int) mgr_data->ovls[0].cfg.pre_mult_alpha));
}
