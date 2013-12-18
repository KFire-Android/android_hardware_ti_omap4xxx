LOCAL_PATH:= $(call my-dir)

#
# libOMX.TI.DUCATI1.VIDEO.H264E
#

include $(CLEAR_VARS)

LOCAL_C_INCLUDES += \
    frameworks/native/include/media/openmax \
    $(HARDWARE_TI_OMAP4_BASE)/omx/base/omx_core/inc \
    $(HARDWARE_TI_OMAP4_BASE)/omx/osal/inc \
    $(HARDWARE_TI_OMAP4_BASE)/omx/base/omx_base_comp/inc \
    $(HARDWARE_TI_OMAP4_BASE)/omx/base/omx_base_dio_plugin/inc \
    $(HARDWARE_TI_OMAP4_BASE)/hwc \
    $(LOCAL_PATH)/omx_h264_enc/inc \
    hardware/ti/dce/packages/codec_engine/ \
    hardware/ti/dce/packages/framework_components/ \
    hardware/ti/dce/packages/ivahd_codecs/ \
    hardware/ti/dce/packages/xdais/ \
    hardware/ti/dce/packages/xdctools

LOCAL_SHARED_LIBRARIES := \
    libosal \
    libc \
    liblog \
    libOMX \
    libhardware \
    libdce


LOCAL_CFLAGS += -Dxdc_target_types__=google/targets/arm/std.h

LOCAL_MODULE_TAGS:= optional

LOCAL_SRC_FILES:= omx_h264_enc/src/omx_H264videoencoder.c \
                  omx_h264_enc/src/omx_H264videoencoderutils.c

LOCAL_MODULE:= libOMX.TI.DUCATI1.VIDEO.H264E

include $(BUILD_SHARED_LIBRARY)
