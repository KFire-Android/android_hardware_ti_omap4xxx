#include "dsscomp_test.h"

int test_check_ovl(struct dsscomp_setup_mgr_data *mgr_data)
{
	int ret = 0;

	struct dsscomp_check_ovl_data *chk_ovl_data = NULL;

	chk_ovl_data = (typeof(chk_ovl_data)) calloc(1, sizeof(*chk_ovl_data));
	if (!chk_ovl_data) {
		printf("Error: Could not allocate memory for check_ovl data\n");
		return -EINVAL;
	}

	chk_ovl_data->mgr.ix = mgr_data->mgr.ix;
	chk_ovl_data->mgr.default_color = mgr_data->mgr.default_color;
	chk_ovl_data->mgr.trans_key_type = mgr_data->mgr.trans_key_type;
	chk_ovl_data->mgr.trans_key = mgr_data->mgr.trans_key;
	chk_ovl_data->mgr.trans_enabled = mgr_data->mgr.trans_enabled;
	chk_ovl_data->mgr.interlaced = mgr_data->mgr.interlaced;
	chk_ovl_data->mgr.alpha_blending = mgr_data->mgr.alpha_blending;
	chk_ovl_data->mgr.cpr_enabled = mgr_data->mgr.cpr_enabled;
	chk_ovl_data->mgr.swap_rb = mgr_data->mgr.swap_rb;

	chk_ovl_data->ovl.address = mgr_data->ovls[0].address;
	chk_ovl_data->ovl.cfg.stride = mgr_data->ovls[0].cfg.stride;
	chk_ovl_data->ovl.cfg.ix = mgr_data->ovls[0].cfg.ix;
	chk_ovl_data->ovl.cfg.enabled = mgr_data->ovls[0].cfg.enabled;
	chk_ovl_data->ovl.cfg.color_mode = mgr_data->ovls[0].cfg.color_mode;
	chk_ovl_data->ovl.cfg.pre_mult_alpha = mgr_data->ovls[0].cfg.pre_mult_alpha;
	chk_ovl_data->ovl.cfg.global_alpha = mgr_data->ovls[0].cfg.global_alpha;
	chk_ovl_data->ovl.cfg.rotation = mgr_data->ovls[0].cfg.rotation;
	chk_ovl_data->ovl.cfg.mirror = mgr_data->ovls[0].cfg.mirror;
	chk_ovl_data->ovl.cfg.ilace = mgr_data->ovls[0].cfg.ilace;
	chk_ovl_data->ovl.cfg.zorder = mgr_data->ovls[0].cfg.zorder;
	chk_ovl_data->ovl.cfg.zonly = mgr_data->ovls[0].cfg.zonly;
	chk_ovl_data->ovl.cfg.width = mgr_data->ovls[0].cfg.width;
	chk_ovl_data->ovl.cfg.height = mgr_data->ovls[0].cfg.height;
	chk_ovl_data->ovl.cfg.win.x = mgr_data->ovls[0].cfg.win.x;
	chk_ovl_data->ovl.cfg.win.y = mgr_data->ovls[0].cfg.win.y;
	chk_ovl_data->ovl.cfg.win.w = mgr_data->ovls[0].cfg.win.w;
	chk_ovl_data->ovl.cfg.win.h = mgr_data->ovls[0].cfg.win.h;
	chk_ovl_data->ovl.cfg.crop.x = mgr_data->ovls[0].cfg.crop.x;
	chk_ovl_data->ovl.cfg.crop.y = mgr_data->ovls[0].cfg.crop.y;
	chk_ovl_data->ovl.cfg.crop.w = mgr_data->ovls[0].cfg.crop.w;
	chk_ovl_data->ovl.cfg.crop.h = mgr_data->ovls[0].cfg.crop.h;
	chk_ovl_data->mode = mgr_data->mode;

	ret = chk_ovl(chk_ovl_data);
	if ((ret < 0 ) || (ret > (1 << NO_OF_PIPELINES)))
		printf("DSSCIOC_CHECK_OVL ioctl failed: 0x%x\n", ret);
	else
		ret = 0;

	return ret;
}
