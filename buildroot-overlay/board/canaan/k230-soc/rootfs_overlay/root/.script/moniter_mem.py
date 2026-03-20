import os
import time
import datetime

def get_china_time():
    """
    Get standard China time (UTC+8, East 8th zone), eliminate deprecated utcnow() warning,
    pure Python implementation.
    Returns: formatted time string (YYYY-MM-DD HH:MM:SS)
    """
    # Use timezone-aware UTC time instead of deprecated utcnow() (Python 3.12+ recommended)
    utc_now = datetime.datetime.now(datetime.UTC)
    # Add 8 hours to convert to China time (UTC+8)
    china_now = utc_now + datetime.timedelta(hours=8)
    # Format and return, ignoring timezone identifier (keep standard time string for CSV storage)
    return china_now.strftime('%Y-%m-%d %H:%M:%S')

def get_memory_info():
    """
    Read /proc/meminfo and parse Linux memory usage (compatible with Buildroot embedded system).
    Returns: dictionary containing memory info with China time (unit: MB).
    """
    mem_info = {}
    meminfo_path = '/proc/meminfo'

    if not os.path.exists(meminfo_path):
        raise FileNotFoundError(f"Cannot find {meminfo_path}, non-standard Linux environment (Buildroot)?")

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
                # Extract numeric value and convert to MB (/proc/meminfo unit is KB, 1MB = 1024KB)
                try:
                    num = int(value.strip().split()[0]) / 1024
                    mem_info[key] = round(num, 2)
                except (ValueError, IndexError):
                    mem_info[key] = 0.0
    except Exception as e:
        print(f"Current collection read failed: {e}, skipping this collection")
        return None

    # Calculate core memory metrics, using China time for timestamp
    mem_stats = {
        'Collection Time (China)': get_china_time(),
        'Total Memory (MB)': mem_info.get('MemTotal', 0.0),
        'Free Memory (MB)': mem_info.get('MemFree', 0.0),
        'Available Memory (MB)': mem_info.get('MemAvailable', 0.0),
        'Used Memory (MB)': round(mem_info.get('MemTotal', 0.0) - mem_info.get('MemAvailable', 0.0), 2),
        'Cached (MB)': mem_info.get('Cached', 0.0),
        'Buffers (MB)': mem_info.get('Buffers', 0.0)
    }

    return mem_stats

def monitor_memory_over_night(interval=300, duration=36000, output_dir='./memory_night_report'):
    """
    Pure Python with no dependencies: overnight memory monitoring (China time,
    no deprecated warnings) and CSV generation (compatible with Buildroot system).

    :param interval: Collection interval in seconds, default 300 seconds (5 minutes)
    :param duration: Total monitoring duration in seconds, default 36000 seconds (10 hours)
    :param output_dir: CSV output directory, default ./memory_night_report
    """
    # 1. Directory creation (prefer persistent directory, fall back to /tmp on failure)
    persist_dir = output_dir
    tmp_dir = '/tmp/memory_night_report'
    try:
        if not os.path.exists(persist_dir):
            os.makedirs(persist_dir)
        use_dir = persist_dir
    except OSError as e:
        print(f"Persistent directory {persist_dir} creation failed (possibly no write permission): {e}")
        print(f"Switching to temporary directory {tmp_dir} for storage, note: data will be lost after reboot!")
        if not os.path.exists(tmp_dir):
            os.makedirs(tmp_dir)
        use_dir = tmp_dir

    # 2. Initialize parameters
    memory_data_list = []
    total_times = int(duration / interval)
    # Calculate expected end time (China time)
    china_now = datetime.datetime.now(datetime.UTC) + datetime.timedelta(hours=8)
    end_time_china = china_now + datetime.timedelta(seconds=duration)
    print("="*60)
    print(f"Starting overnight memory monitoring (Buildroot, China time, no deprecation warnings)")
    print(f"Collection interval: {interval} seconds ({int(interval/60)} minutes)")
    print(f"Total collections: {total_times}")
    print(f"Total monitoring duration: {duration} seconds ({int(duration/3600)} hours)")
    print(f"Current China time: {get_china_time()}")
    print(f"Expected end time (China time): {end_time_china.strftime('%Y-%m-%d %H:%M:%S')}")
    print(f"Report save directory: {use_dir}")
    print("="*60 + "\n")

    # 3. Long-running collection loop
    try:
        for i in range(total_times):
            # Collect current memory info (skip on failure)
            current_mem = get_memory_info()
            if current_mem is not None:
                memory_data_list.append(current_mem)
                print(f"Collection {i+1}/{total_times} completed: {current_mem['Collection Time (China)']}, "
                      f"Used Memory {current_mem['Used Memory (MB)']} MB")
            else:
                print(f"Collection {i+1}/{total_times} failed, skipping")

            # Wait for interval (no wait after last collection to save resources)
            if i < total_times - 1:
                time.sleep(interval)

    except KeyboardInterrupt:
        print("\n\nUser manually terminated monitoring, generating CSV report from collected data...")
    except Exception as e:
        print(f"\n\nCritical error during monitoring: {e}, generating CSV report from collected data...")

    # 4. Generate CSV file (skip if no valid data)
    if not memory_data_list:
        print("No valid collection data, cannot generate CSV report")
        return

    # Generate CSV filename with China timestamp
    start_time_str = memory_data_list[0]['Collection Time (China)'].replace(' ', '_').replace(':', '')
    end_time_str = memory_data_list[-1]['Collection Time (China)'].replace(' ', '_').replace(':', '')
    csv_filename = os.path.join(use_dir, f"memory_monitor_night_{start_time_str}_to_{end_time_str}.csv")

    # Write CSV file (utf-8-sig encoding for Windows Excel compatibility)
    try:
        with open(csv_filename, 'w', encoding='utf-8-sig', newline='') as f:
            # Write header row
            headers = list(memory_data_list[0].keys())
            f.write(','.join(headers) + '\n')

            # Write all collected data
            for data in memory_data_list:
                row_data = [str(data[header]) for header in headers]
                f.write(','.join(row_data) + '\n')

        print("\n" + "="*60)
        print(f"CSV report generated successfully!")
        print(f"File path: {csv_filename}")
        print(f"Valid collection records: {len(memory_data_list)}")
        print(f"Can export to PC via scp for Excel analysis")
        print("="*60)
    except OSError as e:
        print(f"Failed to write CSV file: {e}, please check directory write permissions")

if __name__ == "__main__":
    # Overseas monitoring parameters (China time, no deprecation warnings, run directly)
    monitor_memory_over_night(interval=120, duration=1000000)
