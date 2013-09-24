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

static void *memalloc(struct ion_allocation_data *ion_data, int32_t *map_fd, int32_t *share_fd)
{
    int err = -1;
    void *ptr;

    err = ion_alloc(ion_fd, ion_data->len, ion_data->align, 1 << ION_HEAP_TYPE_CARVEOUT,
                    ion_data->flags, (struct ion_handle **)&(ion_data->handle));
    if (err < 0 || !ion_data->handle) {
        OSAL_ErrorTrace("ION CARVEOUT MEM ALLOC failed[%d]", err);
        return NULL;
    }

    err = ion_map(ion_fd, ion_data->handle, ion_data->len,
                PROT_READ | PROT_WRITE,
                MAP_SHARED, 0, (unsigned char **)&ptr, map_fd);

    if (err || !ptr) {
        OSAL_ErrorTrace("ION CARVEOUT MEM MAP failed[%d]", err);
        return NULL;
    }

    err = ion_share(ion_fd, ion_data->handle, share_fd);
    if (err || *share_fd < 0) {
        OSAL_ErrorTrace("ION CARVEOUT MEM SHARE failed[%d]", err);
        return NULL;
    }

    memset(ptr, 0, ion_data->len);
    return (ptr);

}

static void memfree(struct ion_allocation_data *ion_data, int32_t map_fd, int32_t share_fd)
{
    ion_free(ion_fd, ion_data->handle);
    close(map_fd);
    close(share_fd);
    return;
}

void *memplugin_alloc(int sz, int height, MemRegion region, int align, int flags)
{
    int32_t map_fd;
    int32_t share_fd;
    struct ion_allocation_data ion_data;
    int len = 0;
    MemHeader *h = NULL;

    switch(region) {
    case MEM_CARVEOUT:
        ion_data.len = sz + sizeof (MemHeader);
        ion_data.len = (ion_data.len + LINUX_PAGE_SIZE - 1) & ~(LINUX_PAGE_SIZE - 1);
        ion_data.align = align;
        ion_data.heap_id_mask = 1 << ION_HEAP_TYPE_CARVEOUT;
        ion_data.flags = flags;

        h = (MemHeader*)memalloc(&ion_data, &map_fd, &share_fd);

        h->size = sz;
        h->ptr = (uint8_t *)h + sizeof(MemHeader);
        h->dma_buf_fd = share_fd;
        h->region = MEM_CARVEOUT;
        h->offset = 0;
        h->map_fd = map_fd;
        h->handle = (void*)ion_data.handle;
        return (h->ptr);

    break;
    default:
        OSAL_ErrorTrace("Invalid memory type in ION mem alloc[%d]", region);
    }
    return NULL;
}

void memplugin_free(void *ptr)
{
    struct ion_allocation_data ion_data;

    if (!ptr)
        return;

    MemHeader *h = (MemHeader*)P2H(ptr);
    switch(h->region) {
    case MEM_CARVEOUT:
        ion_data.handle = h->handle;
        memfree(&ion_data, h->map_fd, h->dma_buf_fd);
        munmap(h, h->size+sizeof(MemHeader));
    break;
    default:
        OSAL_ErrorTrace("Invalid memory type in ION mem free[%d]", h->region);
    }
    return;
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

void *memplugin_alloc_noheader(MemHeader *memHdr, int sz, int height, MemRegion region, int align, int flags)
{
    int32_t map_fd;
    int32_t share_fd;
    struct ion_allocation_data ion_data;
    MemHeader *h = memHdr;
    if (!memHdr)
        return NULL;

    switch(region) {
    case MEM_CARVEOUT:
        ion_data.len = sz;
        ion_data.len = (ion_data.len + LINUX_PAGE_SIZE - 1) & ~(LINUX_PAGE_SIZE - 1);
        ion_data.align = align;
        ion_data.heap_id_mask = 1 << ION_HEAP_TYPE_CARVEOUT;
        ion_data.flags = flags;

        h->ptr = memalloc(&ion_data, &map_fd, &share_fd);
        h->size = sz;
        h->dma_buf_fd = share_fd;
        h->region = MEM_CARVEOUT;
        h->offset = 0;
        h->map_fd = map_fd;
        h->handle = (void*)ion_data.handle;

        return (h->ptr);

    break;
    default:
        OSAL_ErrorTrace("Invalid memory type in ION mem alloc[%d]", region);
    }
    return NULL;
}

void memplugin_free_noheader(MemHeader *memHdr)
{
    struct ion_allocation_data ion_data;
    if (!memHdr)
        return;

    MemHeader *h = memHdr;
    switch(h->region) {
    case MEM_CARVEOUT:
        ion_data.handle = h->handle;
        memfree(&ion_data, h->map_fd, h->dma_buf_fd);
        munmap(h->ptr, h->size);
    break;
    default:
        OSAL_ErrorTrace("Invalid memory type in ION mem free[%d]", h->region);
    }
    return;
}
