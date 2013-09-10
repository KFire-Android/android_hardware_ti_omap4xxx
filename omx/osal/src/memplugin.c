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

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/eventfd.h>
#include <fcntl.h>

#include "osal_trace.h"
#include "memplugin.h"

#define LINUX_PAGE_SIZE 4096
static int ion_fd = 0;

int memplugin_open()
{
    int err = MEM_ERR_NONE;
    if (ion_fd > 0)
        return 0;
    ion_fd = ion_open();
    if(ion_fd <= 0) {
        OSAL_ErrorTrace("ion open failed");
        return -1;
    }
    return 0;
}

int memplugin_close()
{
    int ret = ion_close(ion_fd);
    ion_fd = -1;
    return ret;
}

void *memplugin_alloc(int sz, int height, MemRegion region, int align, int flags)
{
    int err = -1;
    uint32_t len = 0;
    void *handle;
    void *ptr;
    int32_t map_fd;
    int32_t share_fd;
    MemHeader *h;

    switch(region) {
    case MEM_CARVEOUT:
        len = sz + sizeof (MemHeader);
        len = (len + LINUX_PAGE_SIZE - 1) & ~(LINUX_PAGE_SIZE - 1);

        err = ion_alloc(ion_fd, len, align, 1 << ION_HEAP_TYPE_CARVEOUT,
                        flags, (struct ion_handle **)&handle);
        if (err < 0 || !handle) {
            OSAL_ErrorTrace("ION CARVEOUT MEM ALLOC failed[%d]", err);
            return NULL;
        }

        err = ion_map(ion_fd, handle, len,
                    PROT_READ | PROT_WRITE,
                    MAP_SHARED, 0, (unsigned char **)&ptr, &map_fd);

        if (err || !ptr) {
            OSAL_ErrorTrace("ION CARVEOUT MEM MAP failed[%d]", err);
            return NULL;
        }

        err = ion_share(ion_fd, handle, &share_fd);
        if (err || share_fd < 0) {
            OSAL_ErrorTrace("ION CARVEOUT MEM SHARE failed[%d]", err);
            return NULL;
        }


        h = (MemHeader*)ptr;
        h->size = sz;
        h->ptr = (uint8_t *)h + sizeof(MemHeader);
        h->dma_buf_fd = share_fd;
        h->region = MEM_CARVEOUT;
        h->offset = 0;
        h->map_fd = map_fd;
        h->handle = (void*)handle;

        memset(H2P(h), 0, sz);
        return (H2P(h));
    break;
    default:
        OSAL_ErrorTrace("Invalid memory type in ION mem alloc[%d]", region);
    }
    return NULL;
}

void memplugin_free(void *ptr)
{
    int err = 0;
    if (!ptr)
        return;

    MemHeader *h = (MemHeader*)P2H(ptr);
    switch(h->region) {
    case MEM_CARVEOUT:
        err = ion_free(ion_fd, h->handle);
        if (err < 0) {
            OSAL_ErrorTrace("ION CARVEOUT MEM FREE failed[%d]", err);
            return -1;
        }
        close(h->map_fd);
        close(h->dma_buf_fd);
        munmap(h, h->size+sizeof(MemHeader));
    break;
    default:
        OSAL_ErrorTrace("Invalid memory type in ION mem free[%d]", h->region);
    }
    return 0;
}

int32_t memplugin_share(void *ptr)
{
    if (!ptr)
        return -1;

    MemHeader *h = P2H(ptr);
    return h->dma_buf_fd;

}

int memplugin_xlate(int alloc_fd, int* share_fd)
{
    int ret;
    //Get the DMA BUFF_FDs for the gralloc pointers
    if (!share_fd) {
        return -1;
    }
    ret = ion_lookup_share_fd(ion_fd, alloc_fd, 2, share_fd);
    return ret;
}

