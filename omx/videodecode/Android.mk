LOCAL_PATH:= $(call my-dir)

#
# libOMX.TI.DUCATI1.VIDEO.DECODER.so
#

include $(CLEAR_VARS)

LOCAL_C_INCLUDES += \
        frameworks/native/include/media/openmax \
        $(HARDWARE_TI_OMAP4_BASE)/omx/base/omx_core/inc \
        $(HARDWARE_TI_OMAP4_BASE)/omx/osal/inc \
        $(HARDWARE_TI_OMAP4_BASE)/omx/base/omx_base_comp/inc \
        $(HARDWARE_TI_OMAP4_BASE)/omx/base/omx_base_dio_plugin/inc \
        $(HARDWARE_TI_OMAP4_BASE)/../../libhardware/include \
        $(HARDWARE_TI_OMAP4_BASE)/hwc/ \
        $(LOCAL_PATH)/../libdce/ \
        $(LOCAL_PATH)/omx_videodec_common/inc/ \
        $(LOCAL_PATH)/omx_h264_dec/inc/ \
        $(LOCAL_PATH)/omx_mpeg4_dec/inc/ \
        hardware/ti/libdce/packages/codec_engine/ \
        hardware/ti/libdce/packages/framework_components/ \
        hardware/ti/libdce/packages/ivahd_codecs/ \
        hardware/ti/libdce/packages/xdais/ \
        hardware/ti/libdce/packages/xdctools

LOCAL_SHARED_LIBRARIES := \
        libosal \
        libc \
        liblog \
        libOMX \
        libhardware \
        libdce

LOCAL_CFLAGS += -Dxdc_target_types__=google/targets/arm/std.h

LOCAL_MODULE_TAGS:= optional

LOCAL_SRC_FILES:= omx_videodec_common/src/omx_video_decoder.c \
        omx_videodec_common/src/omx_video_decoder_componenttable.c \
        omx_videodec_common/src/omx_video_decoder_internal.c \
        omx_h264_dec/src/omx_h264dec.c \
        omx_mpeg4_dec/src/omx_mpeg4dec.c

LOCAL_MODULE:= libOMX.TI.DUCATI1.VIDEO.DECODER

include $(BUILD_HEAPTRACKED_SHARED_LIBRARY)
