import os
import time
import datetime

def get_china_time():
    """
    获取标准中国时间（东八区，UTC+8），消除utcnow()废弃警告，纯Python实现
    返回：格式化后的时间字符串（YYYY-MM-DD HH:MM:SS）
    """
    # 替代废弃的utcnow()，使用带时区标识的UTC时间（Python3.12+推荐）
    utc_now = datetime.datetime.now(datetime.UTC)
    # 加上8小时，转换为中国时间（UTC+8）
    china_now = utc_now + datetime.timedelta(hours=8)
    # 格式化返回，忽略时区标识（仅保留标准时间字符串，符合CSV存储需求）
    return china_now.strftime('%Y-%m-%d %H:%M:%S')

def get_memory_info():
    """
    读取/proc/meminfo，解析Linux内存使用情况（适配Buildroot嵌入式系统）
    返回：包含中国时间的内存信息字典（单位：MB）
    """
    mem_info = {}
    meminfo_path = '/proc/meminfo'

    if not os.path.exists(meminfo_path):
        raise FileNotFoundError(f"无法找到 {meminfo_path}，非标准Linux环境（Buildroot）？")

    try:
        with open(meminfo_path, 'r', encoding='utf-8') as f:
            for line in f:
                line = line.strip()
                if not line:
                    continue
                key_value = line.split(':', 1)
                if len(key_value) != 2:
                    continue
                key, value = key_value
                # 提取数值并转换为MB（/proc/meminfo单位为KB，1MB=1024KB）
                try:
                    num = int(value.strip().split()[0]) / 1024
                    mem_info[key] = round(num, 2)
                except (ValueError, IndexError):
                    mem_info[key] = 0.0
    except Exception as e:
        print(f"当前采集读取失败：{e}，跳过本次采集")
        return None

    # 计算核心内存指标，采集时间替换为中国时间（无废弃警告）
    mem_stats = {
        '采集时间(中国时间)': get_china_time(),
        '总内存(MB)': mem_info.get('MemTotal', 0.0),
        '空闲内存(MB)': mem_info.get('MemFree', 0.0),
        '可用内存(MB)': mem_info.get('MemAvailable', 0.0),
        '已用内存(MB)': round(mem_info.get('MemTotal', 0.0) - mem_info.get('MemAvailable', 0.0), 2),
        '缓存(MB)': mem_info.get('Cached', 0.0),
        '缓冲区(MB)': mem_info.get('Buffers', 0.0)
    }

    return mem_stats

def monitor_memory_over_night(interval=300, duration=36000, output_dir='./memory_night_report'):
    """
    纯Python无依赖：一整晚内存监控（中国时间，无废弃警告）并生成CSV（适配Buildroot系统）
    :param interval: 采集间隔（秒），默认300秒/5分钟
    :param duration: 监控总时长（秒），默认36000秒/10小时
    :param output_dir: CSV输出目录，默认当前目录memory_night_report
    """
    # 1. 目录创建（优先持久化目录，若失败切换到/tmp）
    persist_dir = output_dir
    tmp_dir = '/tmp/memory_night_report'
    try:
        if not os.path.exists(persist_dir):
            os.makedirs(persist_dir)
        use_dir = persist_dir
    except OSError as e:
        print(f"持久化目录 {persist_dir} 创建失败（可能无写入权限）：{e}")
        print(f"切换到临时目录 {tmp_dir} 存储，注意：设备重启后数据丢失！")
        if not os.path.exists(tmp_dir):
            os.makedirs(tmp_dir)
        use_dir = tmp_dir

    # 2. 初始化参数
    memory_data_list = []
    total_times = int(duration / interval)
    # 计算预计结束时间（中国时间，无废弃警告）
    china_now = datetime.datetime.now(datetime.UTC) + datetime.timedelta(hours=8)
    end_time_china = china_now + datetime.timedelta(seconds=duration)
    print("="*60)
    print(f"开始一整晚内存监控（Buildroot专用，中国时间，无废弃警告）")
    print(f"采集间隔：{interval} 秒（{int(interval/60)} 分钟）")
    print(f"总采集次数：{total_times} 次")
    print(f"总监控时长：{duration} 秒（{int(duration/3600)} 小时）")
    print(f"当前中国时间：{get_china_time()}")
    print(f"预计结束时间（中国时间）：{end_time_china.strftime('%Y-%m-%d %H:%M:%S')}")
    print(f"报表存储目录：{use_dir}")
    print("="*60 + "\n")

    # 3. 长时间循环采集
    try:
        for i in range(total_times):
            # 采集当前内存信息（跳过异常采集）
            current_mem = get_memory_info()
            if current_mem is not None:
                memory_data_list.append(current_mem)
                print(f"第 {i+1}/{total_times} 次采集完成：{current_mem['采集时间(中国时间)']}，已用内存 {current_mem['已用内存(MB)']} MB")
            else:
                print(f"第 {i+1}/{total_times} 次采集失败，跳过本次")

            # 间隔等待（最后一次无需等待，节省资源）
            if i < total_times - 1:
                time.sleep(interval)

    except KeyboardInterrupt:
        print("\n\n用户手动终止监控，正在生成已采集数据的CSV报表...")
    except Exception as e:
        print(f"\n\n监控过程中出现严重错误：{e}，正在生成已采集数据的CSV报表...")

    # 4. 生成CSV文件（无有效数据则跳过）
    if not memory_data_list:
        print("无有效采集数据，无法生成CSV报表")
        return

    # 生成带中国时间戳的CSV文件名
    start_time_str = memory_data_list[0]['采集时间(中国时间)'].replace(' ', '_').replace(':', '')
    end_time_str = memory_data_list[-1]['采集时间(中国时间)'].replace(' ', '_').replace(':', '')
    csv_filename = os.path.join(use_dir, f"memory_monitor_night_{start_time_str}_to_{end_time_str}.csv")

    # 写入CSV文件（utf-8-sig编码，兼容Windows Excel打开）
    try:
        with open(csv_filename, 'w', encoding='utf-8-sig', newline='') as f:
            # 写入表头
            headers = list(memory_data_list[0].keys())
            f.write(','.join(headers) + '\n')

            # 写入所有采集数据
            for data in memory_data_list:
                row_data = [str(data[header]) for header in headers]
                f.write(','.join(row_data) + '\n')

        print("\n" + "="*60)
        print(f"CSV报表生成成功！")
        print(f"文件路径：{csv_filename}")
        print(f"有效采集记录：{len(memory_data_list)} 条")
        print(f"可通过scp导出到PC用Excel打开分析")
        print("="*60)
    except OSError as e:
        print(f"写入CSV文件失败：{e}，请检查目录写入权限")

if __name__ == "__main__":
    # 一整晚监控参数配置（中国时间，无废弃警告，直接运行即可）
    monitor_memory_over_night(interval=120, duration=1000000)
