
LOCAL_PATH:= $(call my-dir)
ifeq ($(TARGET_BOARD_PLATFORM), $(filter $(TARGET_BOARD_PLATFORM), jacinto6))
include $(CLEAR_VARS)

LOCAL_SRC_FILES := main.c dsscomp_reset_ovls.c dsscomp_disp_reset.c test_image_disp.c test_all_ovls.c test_scaling.c test_chk_ovls.c test_nv12.c test_buf_types.c  test_uyvy.c test_nv12_scaling.c test_uyvy_scaling.c

LOCAL_SHARED_LIBRARIES := libion

LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_TAGS:= optional
LOCAL_MODULE := dsscomp_test
include $(BUILD_EXECUTABLE)
endif

