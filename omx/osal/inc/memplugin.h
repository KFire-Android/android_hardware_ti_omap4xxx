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

#ifndef __MEMPLUGIN_H__
#define __MEMPLUGIN_H__

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <ion/ion.h>
#include <linux/ion.h>

#define P2H(p) (&(((MemHeader *)(p))[-1]))
#define H2P(h) ((void *)&(h)[1])

/*
* MemHeader is important because it is necessary to know the
* size of the parameter buffers on IPU for Cache operations
* The size can't be assumed as codec supports different inputs
* For ex: static params can be VIDDEC3_Params, IVIDDEC3_Params
* or IH264DEC_Params
*/
typedef struct MemHeader {
    int   size;
    void *ptr;
} MemHeader;

typedef enum MemRegion {
    MEM_TILER_1D,
    MEM_TILER_2D,
    MEM_CARVEOUT,
    MEM_GRALLOC,
    MEM_VIRTUAL,
    MEM_METADATA
} MemRegion;

typedef enum BufAccessMode {
    MemAccess_8Bit,
    MemAccess_16Bit,
    MemAccess_32Bit
}BufAccessMode;

typedef struct Mem_2DParams {
    uint32_t nHeight;
    uint32_t nWidth;
    uint32_t nStride;
    BufAccessMode eAccessMode;
}Mem_2DParams;

/* Error Types */
typedef enum MemErrorStatus {
    MEM_ERR_NONE = 0,
    MEM_ERR_INVALID_INPUT = -1,
    MEM_ERR_OUT_OF_TILER = -2,
    MEM_ERR_OUT_OF_CARVEOUT = -3
} MEM_ERROR;


typedef struct BufferAttributes {
    MemRegion memory_region;
    size_t len;
    size_t align;
    unsigned int flags;
    void *ptr;
    int share_fd;
    int map_fd;
    void *handle;
} BuffAttribs;

int memplugin_open();
int memplugin_close();
int memplugin_alloc(BuffAttribs *buffer);
int memplugin_free(BuffAttribs *buffer);
int memplugin_xlate(int alloc_fd, int* share_fd);

#endif  /* __MEMPLUGIN_H__ */

