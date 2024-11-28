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
#include "sensor_buf_manager.h"

SensorBufManager::SensorBufManager(FrameCHWSize isp_shape,std::vector<std::tuple<int, void*>> sensor_bufs)
{
    dims_t in_shape{1, isp_shape.channel, isp_shape.height, isp_shape.width};
    if (sensor_bufs.size()) 
    {
        for(int i=0;i<sensor_bufs.size();++i)
        {
            this->sensor_bufs.push_back(sensor_bufs[i]);
        }
    }
    ai2d_in_tensor = hrt::create(typecode_t::dt_uint8, in_shape, hrt::pool_shared).expect("create ai2d input tensor failed");
    isp_size = isp_shape.channel * isp_shape.height * isp_shape.width;
}

runtime_tensor& SensorBufManager::get_buf_for_index(unsigned index)
{
    if(index<this->sensor_bufs.size())
    {
        auto buf = ai2d_in_tensor.impl()->to_host().unwrap()->buffer().as_host().unwrap().map(map_access_::map_write).unwrap().buffer();
        memcpy(reinterpret_cast<char *>(buf.data()), (void *)(std::get<1>(this->sensor_bufs[index])), isp_size);
        hrt::sync(ai2d_in_tensor, sync_op_t::sync_write_back, true).expect("sync write_back failed");
        return ai2d_in_tensor;
    }
    else
    {
        printf("index : %d , buf_size : %d",index,this->sensor_bufs.size());
        assert(("Invalid index", 0));
    }
}

SensorBufManager::~SensorBufManager()
{

}