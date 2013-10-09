#include "dsscomp_test.h"

int reset_all(struct dsscomp_setup_mgr_data *mgr_data, unsigned int disp_out)
{
	int ret = 0;
	int i, j;
	struct _qdis d = { .display_info = { .ix = 0 } };

	d.display_info.modedb_len = sizeof(d.modedb) / sizeof(*d.modedb);

	printf("Reseting all pipelines...\n");
	/* Reset all the pipelines before starting the test */
	for(i = 0; i < NO_OF_MANAGERS; i++) {
		d.display_info.ix = i;
	   	ret = query_display(&d);
		if (ret) {
			printf("Error: reset_all: query display failed!\n");
			return ret;
		}

		mgr_data->mgr.default_color = DEFAULT_COLOR;
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
		for(j = 0; j < NO_OF_PIPELINES; j++){

			printf("mgr.ix=%d pipeline=%d overlays_owned=0x%x\n", i, j,
								d.display_info.overlays_owned);
			if(d.display_info.overlays_owned & (1 << j)){
				mgr_data->ovls[mgr_data->num_ovls].cfg.ix = j;
				mgr_data->ovls[mgr_data->num_ovls].cfg.enabled = false;
				printf("Overlay[%d] owned by mgr[%d]\n",j,i);
				mgr_data->num_ovls++;
			}
		}

		ret = comp_update( mgr_data);
		if (ret)
			printf("Error: reset_all: mgr.id=%d composition update failed\n",
						mgr_data->mgr.ix);
		ret = 0;
	}
	SLEEP_SEC(3);
	printf("Reset all pipelines done!\n");
	return ret;
}
