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

#define LOG_TAG "OMX_MPEG2_VIDDEC"

#include <omx_mpeg2dec.h>


OMX_ERRORTYPE OMXMPEG2VD_Init(OMX_HANDLETYPE hComponent)
{
    OMX_ERRORTYPE               eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE          *pHandle = NULL;
    OMXVidDecComp   *pVidDecComp = NULL;
    OMXMPEG2VidDecComp        *pMPEG2VidDecComp = NULL;

    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    pVidDecComp = (OMXVidDecComp *)pHandle->pComponentPrivate;

    /*! Initialize the function parameters for OMX functions */
    pHandle->SetParameter = OMXMPEG2VD_SetParameter;
    pHandle->GetParameter = OMXMPEG2VD_GetParameter;

    /*! Initialize the function pointers */
    pVidDecComp->fpSet_StaticParams   = OMXMPEG2VD_SetStaticParams;
    pVidDecComp->fpSet_DynamicParams  = OMXMPEG2VD_SetDynamicParams;
    pVidDecComp->fpSet_Status         = OMXMPEG2VD_SetStatus;
    pVidDecComp->fpCalc_OubuffDetails = CalculateMPEG2VD_outbuff_details;
    pVidDecComp->fpDeinit_Codec       = OMXMPEG2VD_DeInit;
    pVidDecComp->cDecoderName         = "ivahd_mpeg2vdec";

    pVidDecComp->tVideoParams[OMX_VIDDEC_INPUT_PORT].eCompressionFormat = OMX_VIDEO_CodingMPEG2;
    pVidDecComp->tVideoParams[OMX_VIDDEC_OUTPUT_PORT].eCompressionFormat = OMX_VIDEO_CodingUnused;

    pVidDecComp->fpHandle_ExtendedError = OMXMPEG2VD_HandleError;

    pVidDecComp->pCodecSpecific =
        (OMXMPEG2VidDecComp *) OSAL_Malloc(sizeof (OMXMPEG2VidDecComp));
    OMX_CHECK((pVidDecComp->pCodecSpecific) != NULL, OMX_ErrorInsufficientResources);

    pMPEG2VidDecComp =(OMXMPEG2VidDecComp *) pVidDecComp->pCodecSpecific;

    OMX_BASE_INIT_STRUCT_PTR(&(pMPEG2VidDecComp->tDeblockingParam), OMX_PARAM_DEBLOCKINGTYPE);
    pMPEG2VidDecComp->tDeblockingParam.nPortIndex = OMX_VIDDEC_OUTPUT_PORT;
    pMPEG2VidDecComp->tDeblockingParam.bDeblocking = OMX_FALSE;


    OMX_BASE_INIT_STRUCT_PTR(&(pMPEG2VidDecComp->tMPEG2VideoParam), OMX_VIDEO_PARAM_MPEG2TYPE);
    pMPEG2VidDecComp->tMPEG2VideoParam.nPortIndex = OMX_VIDDEC_INPUT_PORT;
    pMPEG2VidDecComp->tMPEG2VideoParam.eProfile = OMX_VIDEO_MPEG2ProfileMain;
    pMPEG2VidDecComp->tMPEG2VideoParam.eLevel = OMX_VIDEO_MPEG2LevelML;

    /*! Allocate Memory for Static Parameter */
    pVidDecComp->pDecStaticParams = (IVIDDEC3_Params *)memplugin_alloc(sizeof(IMPEG2VDEC_Params), 1, MEM_CARVEOUT, 0, 0);
    OMX_CHECK(pVidDecComp->pDecStaticParams != NULL, OMX_ErrorInsufficientResources);
    OSAL_Memset(pVidDecComp->pDecStaticParams, 0x0, sizeof(IMPEG2VDEC_Params));

    /*! Allocate Memory for Dynamic Parameter */
    pVidDecComp->pDecDynParams = (IVIDDEC3_DynamicParams *)memplugin_alloc(sizeof(IMPEG2VDEC_DynamicParams), 1, MEM_CARVEOUT, 0, 0);
    OMX_CHECK(pVidDecComp->pDecDynParams != NULL, OMX_ErrorInsufficientResources);
    OSAL_Memset(pVidDecComp->pDecDynParams, 0x0, sizeof(IMPEG2VDEC_DynamicParams));

    /*! Allocate Memory for Status Structure */
    pVidDecComp->pDecStatus = (IVIDDEC3_Status *) memplugin_alloc(sizeof(IMPEG2VDEC_Status), 1, MEM_CARVEOUT, 0, 0);
    OMX_CHECK(pVidDecComp->pDecStatus != NULL, OMX_ErrorInsufficientResources);
    OSAL_Memset(pVidDecComp->pDecStatus, 0x0, sizeof(IMPEG2VDEC_Status));

    /*! Allocate Memory for Input Arguments */
    pVidDecComp->pDecInArgs = (IVIDDEC3_InArgs *) memplugin_alloc(sizeof(IMPEG2VDEC_InArgs), 1, MEM_CARVEOUT, 0, 0);
    OMX_CHECK(pVidDecComp->pDecInArgs != NULL, OMX_ErrorInsufficientResources);
    OSAL_Memset(pVidDecComp->pDecInArgs, 0x0, sizeof(IMPEG2VDEC_InArgs));

    /*! Allocate Memory for Output Arguments */
    pVidDecComp->pDecOutArgs    = (IVIDDEC3_OutArgs *) memplugin_alloc(sizeof(IMPEG2VDEC_OutArgs), 1, MEM_CARVEOUT, 0, 0);
    OMX_CHECK(pVidDecComp->pDecOutArgs != NULL, OMX_ErrorInsufficientResources);
    OSAL_Memset(pVidDecComp->pDecOutArgs, 0x0, sizeof(IMPEG2VDEC_OutArgs));

    pVidDecComp->pDecInArgs->size   = sizeof(IMPEG2VDEC_InArgs);
    pVidDecComp->pDecOutArgs->size  = sizeof(IMPEG2VDEC_OutArgs);

    pVidDecComp->pDecStaticParams->metadataType[0] = IVIDEO_METADATAPLANE_NONE;
    pVidDecComp->pDecStaticParams->metadataType[1] = IVIDEO_METADATAPLANE_NONE;
    pVidDecComp->pDecStaticParams->metadataType[2] = IVIDEO_METADATAPLANE_NONE;

    pVidDecComp->pDecStaticParams->operatingMode = IVIDEO_DECODE_ONLY; //IVIDEO_TRANSCODE_FRAMELEVEL; //

    pVidDecComp->pDecStaticParams->inputDataMode = IVIDEO_ENTIREFRAME;
    pVidDecComp->pDecStaticParams->numInputDataUnits  = 0;


EXIT:
    return (eError);

}

void OMXMPEG2VD_SetStaticParams(OMX_HANDLETYPE hComponent, void *staticparams)
{
    OMX_COMPONENTTYPE          *pHandle = NULL;
    OMXVidDecComp   *pVidDecComp = NULL;
    OMXMPEG2VidDecComp        *pMPEG2VidDecComp = NULL;
    IMPEG2VDEC_Params          *params;

    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    pVidDecComp
        = (OMXVidDecComp *)pHandle->pComponentPrivate;

    pMPEG2VidDecComp =
        (OMXMPEG2VidDecComp *) pVidDecComp->pCodecSpecific;

    params = (IMPEG2VDEC_Params *) staticparams;

    params->viddecParams.size = sizeof(IMPEG2VDEC_Params);

    params->viddecParams.maxFrameRate        = 30000;

    params->viddecParams.maxBitRate          = 10000000;

    params->viddecParams.dataEndianness      = XDM_BYTE;
    params->viddecParams.forceChromaFormat      = XDM_YUV_420SP;
    params->viddecParams.displayDelay      = IVIDDEC3_DISPLAY_DELAY_1;
    params->viddecParams.inputDataMode      = IVIDEO_ENTIREFRAME;
    params->viddecParams.numInputDataUnits  = 0;
    params->viddecParams.numOutputDataUnits  = 0;
    params->viddecParams.outputDataMode      = IVIDEO_ENTIREFRAME;
    params->viddecParams.displayBufsMode = IVIDDEC3_DISPLAYBUFS_EMBEDDED;

    params->viddecParams.errorInfoMode  = IVIDEO_ERRORINFO_OFF;

    params->outloopDeBlocking = pMPEG2VidDecComp->tDeblockingParam.bDeblocking;
    params->ErrorConcealmentON = FALSE;

    params->debugTraceLevel= 0;
    params->lastNFramesToLog= 0;

    return;
}


void OMXMPEG2VD_SetDynamicParams(OMX_HANDLETYPE hComponent, void *dynParams)
{
    IMPEG2VDEC_DynamicParams   *dynamicParams;

    dynamicParams = (IMPEG2VDEC_DynamicParams *)dynParams;

    /*! Update the Individual fields in the Dyanmic Params of MPEG2 decoder */
    dynamicParams->viddecDynamicParams.size = sizeof(IMPEG2VDEC_DynamicParams);
    dynamicParams->viddecDynamicParams.decodeHeader  = XDM_DECODE_AU;
    dynamicParams->viddecDynamicParams.displayWidth  = 0;
    dynamicParams->viddecDynamicParams.frameSkipMode = IVIDEO_NO_SKIP;
    dynamicParams->viddecDynamicParams.newFrameFlag  = XDAS_TRUE;
    dynamicParams->viddecDynamicParams.putDataFxn = NULL;
    dynamicParams->viddecDynamicParams.putDataHandle = NULL;
    dynamicParams->viddecDynamicParams.getDataFxn = NULL;
    dynamicParams->viddecDynamicParams.getDataHandle = NULL;
    dynamicParams->viddecDynamicParams.putBufferFxn = NULL;
    dynamicParams->viddecDynamicParams.putBufferHandle = NULL;
    dynamicParams->viddecDynamicParams.lateAcquireArg = IRES_HDVICP2_UNKNOWNLATEACQUIREARG;
    return;
}


void OMXMPEG2VD_SetStatus(OMX_HANDLETYPE hComponent, void *decstatus)
{
    IMPEG2VDEC_Status   *status;

    status = (IMPEG2VDEC_Status *)decstatus;

    status->viddecStatus.size     = sizeof(IMPEG2VDEC_Status);
    return;
}


PaddedBuffParams CalculateMPEG2VD_outbuff_details(OMX_HANDLETYPE hComponent,
                                                 OMX_U32 width, OMX_U32 height)
{
    PaddedBuffParams    OutBuffDetails;

    /* OutBuffDetails.nBufferSize
        = ((((width + PADX + 127) & ~127) * (height + PADY))); */
    /* The 2 additional rows and columns are required as per codec request to boost performance. */
    OutBuffDetails.nBufferSize
        = ((width + 2) * (height + 2));
    /* Multiply buffer size by 1.5 to account for both luma and chroma */
    OutBuffDetails.nBufferSize = (OutBuffDetails.nBufferSize * 3) >> 1;

    OutBuffDetails.nBufferCountMin = 4;
    OutBuffDetails.nBufferCountActual = 8;
    OutBuffDetails.n1DBufferAlignment = 16;
    //OutBuffDetails.nPaddedWidth = (width + PADX + 127) & ~127;
    //OutBuffDetails.nPaddedHeight = height + PADY;
    OutBuffDetails.nPaddedWidth = width;
    OutBuffDetails.nPaddedHeight = height;
    OutBuffDetails.n2DBufferYAlignment = 1;
    OutBuffDetails.n2DBufferXAlignment = 16;

    return (OutBuffDetails);
}


void OMXMPEG2VD_DeInit(OMX_HANDLETYPE hComponent)
{
    OMX_COMPONENTTYPE          *pHandle = NULL;
    OMXVidDecComp   *pVidDecComp = NULL;

    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    pVidDecComp = (OMXVidDecComp *)pHandle->pComponentPrivate;
    /*! Delete all the memory which was allocated during init of decoder */
    if( pVidDecComp->pDecStaticParams ) {
        memplugin_free(pVidDecComp->pDecStaticParams);
        pVidDecComp->pDecStaticParams = NULL;
    }
    if( pVidDecComp->pDecDynParams ) {
        memplugin_free(pVidDecComp->pDecDynParams);
        pVidDecComp->pDecDynParams = NULL;
    }
    if( pVidDecComp->pDecStatus ) {
        memplugin_free(pVidDecComp->pDecStatus);
        pVidDecComp->pDecStatus = NULL;
    }
    if( pVidDecComp->pDecInArgs ) {
        memplugin_free(pVidDecComp->pDecInArgs);
        pVidDecComp->pDecInArgs = NULL;
    }
    if( pVidDecComp->pDecOutArgs ) {
        memplugin_free(pVidDecComp->pDecOutArgs);
        pVidDecComp->pDecOutArgs = NULL;
    }
    if( pVidDecComp->pCodecSpecific ) {
        OSAL_Free(pVidDecComp->pCodecSpecific);
        pVidDecComp->pCodecSpecific = NULL;
    }
    pHandle->SetParameter = OMXVidDec_SetParameter;
    pHandle->GetParameter = OMXVidDec_GetParameter;
    pVidDecComp->fpHandle_ExtendedError = NULL;
}

OMX_ERRORTYPE OMXMPEG2VD_SetParameter(OMX_HANDLETYPE hComponent,
                                          OMX_INDEXTYPE nIndex, OMX_PTR pParamStruct)
{
    OMX_ERRORTYPE                eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE           *pHandle = NULL;
    OMXVidDecComp    *pVidDecComp  = NULL;
    OMXMPEG2VidDecComp         *pMPEG2VidDecComp = NULL;
    OMX_PARAM_DEBLOCKINGTYPE    *pDeblockingParam = NULL;
    OMX_VIDEO_PARAM_MPEG2TYPE   *pMPEG2VideoParam = NULL;

    OMX_CHECK((hComponent != NULL) && (pParamStruct != NULL),
                     OMX_ErrorBadParameter);

    /*! Initialize the pointers */
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    pVidDecComp = (OMXVidDecComp *)pHandle->pComponentPrivate;
    pMPEG2VidDecComp =(OMXMPEG2VidDecComp *) pVidDecComp->pCodecSpecific;

    switch( nIndex ) {
        case OMX_IndexParamVideoMpeg2 :

            pMPEG2VideoParam = (OMX_VIDEO_PARAM_MPEG2TYPE *) pParamStruct;
            /* SetParameter can be invoked in Loaded State  or on Disabled ports only*/
            OMX_CHECK((pVidDecComp->sBase.tCurState == OMX_StateLoaded) ||
                            (pVidDecComp->sBase.pPorts[pMPEG2VideoParam->nPortIndex]->sPortDef.bEnabled == OMX_FALSE),
                            OMX_ErrorIncorrectStateOperation);

            OMX_BASE_CHK_VERSION(pParamStruct, OMX_VIDEO_PARAM_MPEG2TYPE, eError);
            OMX_CHECK(pMPEG2VideoParam->eProfile == OMX_VIDEO_MPEG2ProfileSimple
                            || pMPEG2VideoParam->eProfile == OMX_VIDEO_MPEG2ProfileMain,
                            OMX_ErrorUnsupportedSetting);
            if( pMPEG2VideoParam->nPortIndex == OMX_VIDDEC_INPUT_PORT ) {
                pMPEG2VidDecComp->tMPEG2VideoParam = *pMPEG2VideoParam;
            } else if( pMPEG2VideoParam->nPortIndex == OMX_VIDDEC_OUTPUT_PORT ) {
                OSAL_ErrorTrace("OMX_IndexParamVideoMpeg2 supported only on i/p port");
                eError = OMX_ErrorUnsupportedIndex;
            } else {
                eError = OMX_ErrorBadPortIndex;
            }
            break;
        case OMX_IndexParamCommonDeblocking :

            pDeblockingParam = (OMX_PARAM_DEBLOCKINGTYPE *) pParamStruct;

            /* SetParameter can be invoked in Loaded State  or on Disabled ports only*/
            OMX_CHECK((pVidDecComp->sBase.tCurState == OMX_StateLoaded) ||
                            (pVidDecComp->sBase.pPorts[pDeblockingParam->nPortIndex]->sPortDef.bEnabled == OMX_FALSE),
                            OMX_ErrorIncorrectStateOperation);

            OMX_BASE_CHK_VERSION(pParamStruct, OMX_PARAM_DEBLOCKINGTYPE, eError);
            if( pDeblockingParam->nPortIndex == OMX_VIDDEC_OUTPUT_PORT ) {
                pMPEG2VidDecComp->tDeblockingParam.bDeblocking = pDeblockingParam->bDeblocking;
            } else if( pDeblockingParam->nPortIndex == OMX_VIDDEC_INPUT_PORT ) {
                OSAL_ErrorTrace("\n Deblocking supported only on o/p port...  \n");
                eError = OMX_ErrorUnsupportedIndex;
            } else {
                eError = OMX_ErrorBadPortIndex;
            }
            break;
        default :
            eError = OMXVidDec_SetParameter(hComponent, nIndex, pParamStruct);
    }

EXIT:
    return (eError);
}

OMX_ERRORTYPE OMXMPEG2VD_GetParameter(OMX_HANDLETYPE hComponent,
                                          OMX_INDEXTYPE nIndex, OMX_PTR pParamStruct)
{
    OMX_ERRORTYPE                       eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE                  *pHandle = NULL;
    OMXVidDecComp           *pVidDecComp  = NULL;
    OMXMPEG2VidDecComp                *pMPEG2VidDecComp = NULL;
    OMX_PARAM_DEBLOCKINGTYPE           *pDeblockingParam = NULL;
    OMX_VIDEO_PARAM_MPEG2TYPE          *pMPEG2VideoParam = NULL;
    OMX_VIDEO_PARAM_PROFILELEVELTYPE   *pMPEG2ProfileLevelParam = NULL;

    OMX_CHECK((hComponent != NULL) && (pParamStruct != NULL),
                     OMX_ErrorBadParameter);

    /*! Initialize the pointers */
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    pVidDecComp = (OMXVidDecComp *)pHandle->pComponentPrivate;
    /* GetParameter can't be invoked incase the comp is in Invalid State  */
    OMX_CHECK(pVidDecComp->sBase.tCurState != OMX_StateInvalid,
                    OMX_ErrorIncorrectStateOperation);

    pMPEG2VidDecComp =(OMXMPEG2VidDecComp *) pVidDecComp->pCodecSpecific;

    switch( nIndex ) {
        case OMX_IndexParamVideoMpeg2 :

            pMPEG2VideoParam = (OMX_VIDEO_PARAM_MPEG2TYPE *) pParamStruct;

            OMX_BASE_CHK_VERSION(pParamStruct, OMX_VIDEO_PARAM_MPEG2TYPE, eError);
            if( pMPEG2VideoParam->nPortIndex == OMX_VIDDEC_INPUT_PORT ) {
                *pMPEG2VideoParam = pMPEG2VidDecComp->tMPEG2VideoParam;
            } else if( pMPEG2VideoParam->nPortIndex == OMX_VIDDEC_OUTPUT_PORT ) {
                OSAL_ErrorTrace("OMX_IndexParamVideoMpeg2 supported only on i/p port");
                eError = OMX_ErrorUnsupportedIndex;
            } else {
                eError = OMX_ErrorBadPortIndex;
            }
            break;
        case OMX_IndexParamCommonDeblocking :
            OMX_BASE_CHK_VERSION(pParamStruct, OMX_PARAM_DEBLOCKINGTYPE, eError);
            pDeblockingParam = (OMX_PARAM_DEBLOCKINGTYPE *) pParamStruct;
            if( pDeblockingParam->nPortIndex == OMX_VIDDEC_OUTPUT_PORT ) {
                pDeblockingParam->bDeblocking = pMPEG2VidDecComp->tDeblockingParam.bDeblocking;
            } else if( pDeblockingParam->nPortIndex == OMX_VIDDEC_INPUT_PORT ) {
                OSAL_ErrorTrace("\n Deblocking supported only on o/p port...  \n");
                eError = OMX_ErrorUnsupportedIndex;
            } else {
                eError = OMX_ErrorBadPortIndex;
            }
            break;
        case OMX_IndexParamVideoProfileLevelQuerySupported :
        {
            OMX_BASE_CHK_VERSION(pParamStruct, OMX_VIDEO_PARAM_PROFILELEVELTYPE, eError);
            pMPEG2ProfileLevelParam = (OMX_VIDEO_PARAM_PROFILELEVELTYPE *)pParamStruct;
            if( pMPEG2ProfileLevelParam->nPortIndex == OMX_VIDDEC_INPUT_PORT ) {
                if( pMPEG2ProfileLevelParam->nProfileIndex == 0 ) {
                    if( pVidDecComp->tVideoParams[OMX_VIDDEC_INPUT_PORT].eCompressionFormat == OMX_VIDEO_CodingMPEG2 ) {
                        pMPEG2ProfileLevelParam->eProfile = (OMX_U32) OMX_VIDEO_MPEG2ProfileSimple;
                        pMPEG2ProfileLevelParam->eLevel = (OMX_U32) OMX_VIDEO_MPEG2LevelLL;
                    }
                } else if( pMPEG2ProfileLevelParam->nProfileIndex == 1 ) {
                    if( pVidDecComp->tVideoParams[OMX_VIDDEC_INPUT_PORT].eCompressionFormat == OMX_VIDEO_CodingMPEG2 ) {
                        pMPEG2ProfileLevelParam->eProfile = (OMX_U32) OMX_VIDEO_MPEG2ProfileMain;
                        pMPEG2ProfileLevelParam->eLevel = (OMX_U32) OMX_VIDEO_MPEG2LevelML;
                    }
                } else {
                    eError = OMX_ErrorNoMore;
                }
            } else if( pMPEG2ProfileLevelParam->nPortIndex == OMX_VIDDEC_OUTPUT_PORT ) {
                OSAL_ErrorTrace("OMX_IndexParamVideoProfileLevelQuerySupported supported only on i/p port");
                eError = OMX_ErrorUnsupportedIndex;
            } else {
                eError = OMX_ErrorBadPortIndex;
            }
        }
        break;

        default :
            eError = OMXVidDec_GetParameter(hComponent, nIndex, pParamStruct);
    }

EXIT:
    return (eError);
}


OMX_ERRORTYPE OMXMPEG2VD_HandleError(OMX_HANDLETYPE hComponent)
{
    OMX_ERRORTYPE               eError = OMX_ErrorNone;
    XDAS_Int32                  status = 0;
    OMX_COMPONENTTYPE          *pHandle = NULL;
    OMXVidDecComp   *pVidDecComp = NULL;

    /* Initialize pointers */
    pHandle = (OMX_COMPONENTTYPE *)hComponent;
    pVidDecComp = (OMXVidDecComp *)pHandle->pComponentPrivate;

    /*! Call the Codec Status function to know cause of error */
    status = VIDDEC3_control(pVidDecComp->pDecHandle,
                             XDM_GETSTATUS,
                             pVidDecComp->pDecDynParams,
                             pVidDecComp->pDecStatus);
    /* Check whether the Codec Status call was succesful */
    if( status != VIDDEC3_EOK ) {
        OSAL_ErrorTrace("VIDDEC3_control XDM_GETSTATUS failed");
        eError = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

EXIT:
    return (eError);
}

