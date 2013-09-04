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
    return ion_close(ion_fd);
}

int memplugin_alloc(BuffAttribs *buffer)
{
    int err = 0;
    uint32_t len = 0;
    MemHeader *h;

    if (!buffer) {
        OSAL_ErrorTrace("memplugin_alloc: Invalid Argument");
        return -1;
    }

    switch(buffer->memory_region) {
    case MEM_CARVEOUT:
        len = buffer->len;
        len = (len + LINUX_PAGE_SIZE - 1) & ~(LINUX_PAGE_SIZE - 1);

        err = ion_alloc(ion_fd, len, buffer->align, 1 << ION_HEAP_TYPE_CARVEOUT,
                        buffer->flags, (struct ion_handle **)&buffer->handle);
        if (err < 0 || !buffer->handle) {
            OSAL_ErrorTrace("ION CARVEOUT MEM ALLOC failed[%d]", err);
            return -1;
        }
        err = ion_map(ion_fd, buffer->handle, buffer->len,
                    PROT_READ | PROT_WRITE,
                    MAP_SHARED, 0, (unsigned char **)&buffer->ptr, &buffer->map_fd);

        if (err || !buffer->ptr) {
            OSAL_ErrorTrace("ION CARVEOUT MEM MAP failed[%d]", err);
            return -1;
        }

        err = ion_share(ion_fd, buffer->handle, &buffer->share_fd);
        if (err || buffer->share_fd < 0) {
            OSAL_ErrorTrace("ION CARVEOUT MEM SHARE failed[%d]", err);
            return -1;
        }

        memset(buffer->ptr, 0, len);
    break;
    default:
        OSAL_ErrorTrace("Invalid memory type in ION mem alloc[%d]", buffer->memory_region);
    }
    return 0;
}

int memplugin_free(BuffAttribs *buffer)
{
    int err = 0;
    if (!buffer) {
        OSAL_ErrorTrace("memplugin_free: Invalid Argument");
        return -1;
    }

    switch(buffer->memory_region) {
    case MEM_CARVEOUT:
        err = ion_free(ion_fd, buffer->handle);
        if (err < 0) {
            OSAL_ErrorTrace("ION CARVEOUT MEM FREE failed[%d]", err);
            return -1;
        }
        close(buffer->map_fd);
        munmap(buffer->ptr, buffer->len);
        close(buffer->share_fd);
    break;
    default:
        OSAL_ErrorTrace("Invalid memory type in ION mem free[%d]", buffer->memory_region);
    }
    return 0;
}

void *memplugin_share(void *ptr, MemRegion memory_region)
{
    /* No Userspace Virtual pointers to DMA BUF Handles conversion required*/
    /* Do nothing */
    return (ptr);
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

