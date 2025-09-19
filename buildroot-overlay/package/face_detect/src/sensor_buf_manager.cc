#include "sensor_buf_manager.h"

/**
 * @brief 构造函数，初始化 SensorBufManager 对象
 * 
 * @param isp_shape ISP 输入图像尺寸（CHW）
 * @param sensor_bufs DMA 缓冲区列表，每个元素为<size, pointer>的tuple
 */
SensorBufManager::SensorBufManager(FrameCHWSize isp_shape, std::vector<std::tuple<int, void*>> sensor_bufs)
{
    // 构造输入张量形状，格式为 {batch, channel, height, width}
    dims_t in_shape{1, isp_shape.channel, isp_shape.height, isp_shape.width};

    // 复制传入的 sensor_bufs 缓冲区列表
    if (sensor_bufs.size()) 
    {
        for (int i = 0; i < sensor_bufs.size(); ++i)
        {
            this->sensor_bufs.push_back(sensor_bufs[i]);
        }
    }

    // 创建 AI2D 输入张量，数据类型为无符号8位整型，使用共享内存池
    ai2d_in_tensor = hrt::create(typecode_t::dt_uint8, in_shape, hrt::pool_shared).expect("create ai2d input tensor failed");

    // 计算 ISP 输入图像数据大小（字节数）
    isp_size = isp_shape.channel * isp_shape.height * isp_shape.width;
}

/**
 * @brief 获取指定索引的 DMA 缓冲区数据，复制到 AI2D 输入张量并返回该张量引用
 * 
 * @param index 传入的缓冲区索引
 * @return 返回 AI2D 输入张量的引用
 * 
 * @note 若索引无效则触发断言
 */
runtime_tensor& SensorBufManager::get_buf_for_index(unsigned index)
{
    if (index < this->sensor_bufs.size())
    {
        // 获取 AI2D 输入张量对应的主机映射缓冲区，准备写入数据
        auto buf = ai2d_in_tensor.impl()->to_host().unwrap()->buffer().as_host().unwrap()
                    .map(map_access_::map_write).unwrap().buffer();

        // 将指定索引的 DMA 缓冲区数据拷贝到 AI2D 输入张量缓冲区
        memcpy(reinterpret_cast<char *>(buf.data()), (void *)(std::get<1>(this->sensor_bufs[index])), isp_size);

        // 同步写回，保证数据一致性
        hrt::sync(ai2d_in_tensor, sync_op_t::sync_write_back, true).expect("sync write_back failed");

        // 返回准备好的 AI2D 输入张量引用
        return ai2d_in_tensor;
    }
    else
    {
        // 打印错误索引信息并断言终止程序
        printf("index : %d , buf_size : %d", index, this->sensor_bufs.size());
        assert(("Invalid index", 0));
    }
}

/**
 * @brief 析构函数，释放资源（目前为空）
 */
SensorBufManager::~SensorBufManager()
{

}
