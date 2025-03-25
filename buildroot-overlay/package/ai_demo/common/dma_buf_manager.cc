/* Copyright (c) 2024, Canaan Bright Sight Co., Ltd
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/ioctl.h>
#include <fcntl.h>
#include "dma_buf_manager.h"

DMABufManager::DMABufManager(FrameCHWSize isp_shape,std::vector<std::tuple<int, void*>> dmabufs)
{
    dims_t in_shape{1, isp_shape.channel, isp_shape.height, isp_shape.width};
    if (dmabufs.size()) 
    {
        ai2d_fd = open("/dev/k230-ai2d", O_RDWR);
        for (auto [fd, ptr]: dmabufs) {
            struct ai2d_import_dmabuf import = { .fd = fd };
            if (ioctl(ai2d_fd, AI2D_IMPORT_DMABUF, &import)) {
                // error
                std::terminate();
            }
            auto tensor = host_runtime_tensor::create(
                typecode_t::dt_uint8, in_shape,
                { (gsl::byte *)ptr, isp_shape.channel * isp_shape.height * isp_shape.width },
                false, hrt::pool_shared_first, import.addr)
                .expect("cannot create input tensor");
            this->dmabufs.push_back(tensor);
        }
    }
}

runtime_tensor& DMABufManager::get_buf_for_index(unsigned index)
{
    if(index<this->dmabufs.size())
    {
        return this->dmabufs[index];
    }
    else
    {
        printf("index : %d , buf_size : %d",index,dmabufs.size());
        assert(("Invalid index", 0));
    }
}

DMABufManager::~DMABufManager()
{
    if (ai2d_fd >= 0) 
    {
        close(ai2d_fd);
        printf("close ai2d_fd\n");
    }
}