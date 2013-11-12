/*
 * Copyright (C) Texas Instruments - http://www.ti.com/
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _OMX_MPEG2DEC_H_
#define _OMX_MPEG2DEC_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/******************************************************************
 *   INCLUDE FILES
 ******************************************************************/
#include <omx_video_decoder.h>
#include <ti/sdo/codecs/mpeg2vdec/impeg2vdec.h>


void OMXMPEG2VD_SetStaticParams(OMX_HANDLETYPE hComponent, void *staticparams);

void OMXMPEG2VD_SetDynamicParams(OMX_HANDLETYPE hComponent, void *dynamicParams);

void OMXMPEG2VD_SetStatus(OMX_HANDLETYPE hComponent, void *status);

PaddedBuffParams CalculateMPEG2VD_outbuff_details(OMX_HANDLETYPE hComponent,
                                                 OMX_U32 width, OMX_U32 height);

/*----------          function prototypes      ------------------- */
extern OMX_ERRORTYPE OMX_TI_VideoDecoder_ComponentInit(OMX_HANDLETYPE hComponent);
OMX_ERRORTYPE OMXMPEG2VD_Init(OMX_HANDLETYPE hComponent);
void OMXMPEG2VD_DeInit(OMX_HANDLETYPE hComponent);
OMX_ERRORTYPE OMXMPEG2VD_HandleError(OMX_HANDLETYPE hComponent);

extern OMX_ERRORTYPE OMXMPEG2VD_SetParameter(OMX_HANDLETYPE hComponent,
                                                 OMX_INDEXTYPE nIndex, OMX_PTR pParamStruct);
extern OMX_ERRORTYPE OMXMPEG2VD_GetParameter(OMX_HANDLETYPE hComponent,
                                                 OMX_INDEXTYPE nIndex, OMX_PTR pParamStruct);

extern OMX_ERRORTYPE OMXVidDec_SetParameter(OMX_HANDLETYPE hComponent,
                                                      OMX_INDEXTYPE nIndex, OMX_PTR pParamStruct);
extern OMX_ERRORTYPE OMXVidDec_GetParameter(OMX_HANDLETYPE hComponent,
                                                      OMX_INDEXTYPE nIndex, OMX_PTR pParamStruct);


typedef struct OMXMPEG2VidDecComp {
    OMX_PARAM_DEBLOCKINGTYPE  tDeblockingParam;
    OMX_VIDEO_PARAM_MPEG2TYPE tMPEG2VideoParam;
} OMXMPEG2VidDecComp;


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _OMX_MPEG2DEC_H_ */

