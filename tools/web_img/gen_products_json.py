#!/usr/bin/env python
# -*- coding: utf-8 -*-
import os
import json
import hashlib
import re
import sys
import io
from datetime import datetime,timedelta
from collections import OrderedDict

# --- 兼容性处理 ---
# Python 3 不再需要 reload(sys) 且 sys 模块没有 setdefaultencoding
if sys.version_info[0] < 3:
    reload(sys)
    sys.setdefaultencoding('utf-8')

def get_china_time():
    if sys.version_info[0] >= 3 and sys.version_info[1] >= 12:
        # Python 3.12+：推荐使用带时区的方式（兼容新版）
        utc_now = datetime.now(datetime.UTC)
    else:
        # Python 2.7 + Python 3.11及以下：使用utcnow()获取UTC时间（无时区标识，跨版本兼容）
        utc_now = datetime.utcnow()

    china_now = utc_now + timedelta(hours=8)

    # 格式化返回，忽略时区标识（仅保留标准时间字符串，符合CSV/JSON存储需求）
    return china_now.strftime('%Y-%m-%d %H:%M:%S')

def get_md5_from_file(file_path):
    md5_file = file_path + ".md5"
    if os.path.exists(md5_file):
        # 使用 io.open 并指定编码，确保双版本下读取行为一致
        with io.open(md5_file, 'r', encoding='utf-8') as f:
            content = f.read().split()
            return content[0] if content else "N/A"
    return "N/A"

def get_dir_creation_time(dir_path):
    if not os.path.exists(dir_path):
        return "dir '{0}' not exist".format(dir_path)

    dir_stat = os.stat(dir_path)
    try:
        # 优先获取出生时间（macOS/FreeBSD），否则获取 ctime
        create_timestamp = getattr(dir_stat, 'st_birthtime', dir_stat.st_ctime)
    except AttributeError:
        create_timestamp = dir_stat.st_ctime

    create_datetime = datetime.fromtimestamp(create_timestamp)
    return create_datetime.strftime('%Y-%m-%d %H:%M:%S')

def get_version_from_filename(filename):
    if "micropython" in filename:
        pattern = r'v\d+\.\d+(?:\.\d+)?'
    else:
        pattern = r'v\d+\.\d+(?:\.\d+)?'

    m = re.search(pattern, filename)
    return m.group(0) if m else "unknown"

def get_product_name_from_filepath(file_path):
    if "linux_sdk_images" in file_path:
        return os.path.basename(os.path.dirname(file_path))
    elif "micropython" in file_path:
        filename = os.path.basename(file_path)
        product_ = filename.split("_micropython")[0]
        product_dict = {
            "CanMV_K230_01Studio": "k230_canmv_01studio_defconfig",
            "CanMV_K230_LCKFB": "k230_canmv_lckfb_defconfig",
            "CanMV_K230_V1P0_P1": "k230_canmv_defconfig",
            "CanMV-K230_DONGSHANPI": "k230_canmv_dongshanpi_defconfig",
            "CanMV_K230D_Zero": "BPI-CanMV-K230D-Zero_defconfig",
            "CanMV_K230_01Studio_Emmc": "k230_canmv_01studio_emmc_defconfig",
            "CanMV-K230_RTT_EVB_BOARD":"k230_canmv_rtt_evb_defconfig",
            "k230_canmv_rtt_evb_defconfig":"CanMV-K230_RTT_EVB_BOARD",
            "CanMV_K230D_ATK_DNK230D":"k230d_canmv_atk_dnk230d_defconfig",
            "CanMV_K230_Hiwonder":"k230_canmv_hiwonder_defconfig",
            "CanMV_K230_YAHBOOM":"k230_canmv_yahboom_defconfig",
            "CanMV_K230_GT6700" :"k230_canmv_gt6700_defconfig",
            "CanMV_K230D_LabPlusAiCamera":"k230d_canmv_labplus_ai_camera_defconfig",
            "CanMV_K230D_JUNROC_AI_CAM":"k230d_canmv_junroc_ai_cam_defconfig",
            "CanMV_K230_MRT":"k230_canmv_mrt_defconfig",
            "CanMV_K230_V3P0": "k230_canmv_v3_defconfig"
        }
        return product_dict.get(product_, product_)
    return ""

def get_url(file_path):
    a = file_path.replace("/data/kendryte-download", "https://download.kendryte.com")
    return a.replace("/data1/k230/release/", "https://ai.b-bug.org/k230/release/")

def update_file_to_json(file_path, products):
    filename = os.path.basename(file_path)
    if not file_path.endswith(".img.gz"):
        return

    if "latest" in file_path:
        return

    product_name = get_product_name_from_filepath(file_path)
    if product_name not in products:
        return

    if "linux" in filename:
        variant_key = "linux"
    elif "debian" in filename:
        variant_key = "debian"
    elif "ubuntu" in filename:
        variant_key = "ubuntu"
    elif "micropython" in filename:
        variant_key = "micropython"
    else:
        variant_key = "unknown"

    variants = products[product_name].get("variants", {})
    if variant_key not in variants:
        return

    version_type = "latest" if ("daily_build" in file_path) else "history"

    ver_info = {
        "version": get_version_from_filename(filename),
        "date": get_dir_creation_time(os.path.dirname(file_path)),
        "url": get_url(file_path),
        "md5": get_md5_from_file(file_path)
    }

    if version_type == "latest":
        variants[variant_key][version_type] = ver_info
    else:
        if not isinstance(variants[variant_key].get(version_type), list):
            variants[variant_key][version_type] = []
        variants[variant_key][version_type].append(ver_info)

    products[product_name]["variants"] = variants

def update_directory_to_json(directory, products):
    file_info_list = []
    for root, _, files in os.walk(directory):
        for f in files:
            file_path = os.path.join(root, f)
            try:
                file_mtime = os.path.getmtime(file_path)
                file_info_list.append((file_mtime, file_path))
            except OSError:
                continue

    file_info_list.sort(key=lambda x: x[0])

    for _, file_path in file_info_list:
        update_file_to_json(file_path, products)

def sdk_release_dirs_2_json(base_dir, products, max_dirs=3):
    if not os.path.isdir(base_dir):
        return
    subdirs = [
        os.path.join(base_dir, d)
        for d in os.listdir(base_dir)
        if os.path.isdir(os.path.join(base_dir, d))
    ]
    subdirs.sort(key=os.path.getmtime, reverse=True)

    for d in subdirs[:max_dirs]:
        update_directory_to_json(d, products)

# 内置初始 JSON 字符串
json_str = u'''
{
    "time": "2024-07-29 16:00:00",
    "products": {
        "k230_canmv_01studio_defconfig": {
            "name": "01studio",
            "description": "01studio canmv and xCAM EVT1",
            "image_url": "https://www.kendryte.com/api/post/attachment?id=833",
            "variants": { "linux" :{"latest": {}, "history": []},"debian" : {"latest": {}, "history": [] },"ubuntu" : {"latest": {}, "history": [] },"micropython" : {"latest": {}, "history": []} }
        },
        "k230_canmv_lckfb_defconfig": {
            "name": "嘉立创k230",
            "description": "k230_canmv_lckfb",
            "image_url": "https://www.kendryte.com/api/post/attachment?id=832",
            "variants": { "linux" :{"latest": {}, "history": []},"debian" : {"latest": {}, "history": [] },"ubuntu" : {"latest": {}, "history": [] },"micropython" : {"latest": {}, "history": []} }
        },
        "k230_canmv_v3_defconfig": {
            "name": "CanMV V3.0",
            "description": "CanMV K230 V3.0(创乐博)",
            "image_url": "https://www.kendryte.com/api/post/attachment?id=834",
            "variants": { "linux" :{"latest": {}, "history": []},"debian" : {"latest": {}, "history": [] },"ubuntu" : {"latest": {}, "history": [] },"micropython" : {"latest": {}, "history": []} }
        },
        "k230_canmv_dongshanpi_defconfig": {
            "name": "dongshanpai",
            "description": "百问网，东山派",
            "image_url": "https://eai.100ask.net/assets/images/image-20240729155648319-504e2c8f5bc5607d6e03148239c45087.png",
            "variants": { "linux" :{"latest": {}, "history": []},"debian" : {"latest": {}, "history": [] },"ubuntu" : {"latest": {}, "history": [] },"micropython" : {"latest": {}, "history": []} }
        },
        "k230_canmv_defconfig": {
            "name": "CanMV 1.0/1.1",
            "description": "CanMV K230 1.0/1.1",
            "image_url": "https://www.kendryte.com/api/imagecdn/zh/sdk/canmv_k230_sdk/images/CanMV-K230_front.png",
            "variants": { "linux" :{"latest": {}, "history": []},"debian" : {"latest": {}, "history": [] },"ubuntu" : {"latest": {}, "history": [] },"micropython" : {"latest": {}, "history": []} }
        },
        "k230_canmv_01studio_emmc_defconfig": {
            "name": "01studio emmc ",
            "description": "01studio canmv base emmc board",
            "image_url": "https://www.kendryte.com/api/post/attachment?id=835",
            "variants": { "linux" :{"latest": {}, "history": []},"debian" : {"latest": {}, "history": [] },"ubuntu" : {"latest": {}, "history": [] },"micropython" : {"latest": {}, "history": []} }
        },
        "BPI-CanMV-K230D-Zero_defconfig": {
            "name": "BPI-CanMV-K230D",
            "description": "BPI-CanMV-K230D",
            "image_url": "https://www.kendryte.com/api/post/attachment?id=482",
            "variants": { "linux" :{"latest": {}, "history": []},"debian" : {"latest": {}, "history": [] },"ubuntu" : {"latest": {}, "history": [] },"micropython" : {"latest": {}, "history": []} }
        },
        "k230_canmv_rtt_evb_defconfig": {
            "name": "rtt evb",
            "description": "k230_canmv_rtt_evb_defconfig",
            "image_url": "https://www.kendryte.com/api/post/attachment?id=577",
            "variants": { "linux" :{"latest": {}, "history": []},"debian" : {"latest": {}, "history": [] },"ubuntu" : {"latest": {}, "history": [] },"micropython" : {"latest": {}, "history": []} }
        },
        "k230d_canmv_atk_dnk230d_defconfig": {
            "name": "正点原子",
            "description": "k230d_canmv_atk_dnk230d_defconfig",
            "image_url": "https://www.kendryte.com/api/post/attachment?id=577",
            "variants": { "linux" :{"latest": {}, "history": []},"debian" : {"latest": {}, "history": [] },"ubuntu" : {"latest": {}, "history": [] },"micropython" : {"latest": {}, "history": []} }
        },
        "k230_canmv_hiwonder_defconfig": {
            "name": "hiwonder",
            "description": "k230_canmv_hiwonder_defconfig",
            "image_url": "https://www.kendryte.com/api/post/attachment?id=577",
            "variants": { "linux" :{"latest": {}, "history": []},"debian" : {"latest": {}, "history": [] },"ubuntu" : {"latest": {}, "history": [] },"micropython" : {"latest": {}, "history": []} }
        },
        "k230_canmv_yahboom_defconfig": {
            "name": "亚博",
            "description": "k230_canmv_yahboom_defconfig",
            "image_url": "https://www.kendryte.com/api/post/attachment?id=577",
            "variants": { "linux" :{"latest": {}, "history": []},"debian" : {"latest": {}, "history": [] },"ubuntu" : {"latest": {}, "history": [] },"micropython" : {"latest": {}, "history": []} }
        },
        "k230_canmv_gt6700_defconfig": {
            "name": "银杏",
            "description": "k230_canmv_gt6700_defconfig",
            "image_url": "https://www.kendryte.com/api/post/attachment?id=577",
            "variants": { "linux" :{"latest": {}, "history": []},"debian" : {"latest": {}, "history": [] },"ubuntu" : {"latest": {}, "history": [] },"micropython" : {"latest": {}, "history": []} }
        },
        "k230d_canmv_junroc_ai_cam_defconfig": {
            "name": "隽鹏Junroc",
            "description": "隽鹏Junroc AI Cam",
            "image_url": "https://www.kendryte.com/img/junpeng.15e0c9ac.webp",
            "variants": { "linux" :{"latest": {}, "history": []},"debian" : {"latest": {}, "history": [] },"ubuntu" : {"latest": {}, "history": [] },"micropython" : {"latest": {}, "history": []} }
        },
        "k230_canmv_mrt_defconfig": {
            "name": "韩端",
            "description": "韩端",
            "image_url": "https://www.kendryte.com/api/imagecdn/zh/sdk/k230_linux_sdk_docs/screenshot_20260730_182637.png",
            "variants": { "linux" :{"latest": {}, "history": []},"debian" : {"latest": {}, "history": [] },"ubuntu" : {"latest": {}, "history": [] },"micropython" : {"latest": {}, "history": []} }
        },
        "k230d_canmv_labplus_ai_camera_defconfig": {
            "name": "labplus",
            "description": "k230d_canmv_labplus_ai_camera_defconfig",
            "image_url": "https://www.kendryte.com/api/post/attachment?id=577",
            "variants": { "linux" :{"latest": {}, "history": []},"debian" : {"latest": {}, "history": [] },"ubuntu" : {"latest": {}, "history": [] },"micropython" : {"latest": {}, "history": []} }
        }
    }
}
'''

def open_json():
    # Python 3 中 json.loads 处理 unicode 字符串很稳健
    data = json.loads(json_str, object_pairs_hook=OrderedDict)
    return data.get('products', {})

def save_json(products, file_path):
    for product in list(products.keys()):
        variants = products[product].get("variants", {})
        variants_to_delete = []
        for v_name, v_info in variants.items():
            latest = v_info.get("latest", {})
            history = v_info.get("history", [])
            if not latest and not history:
                variants_to_delete.append(v_name)

        for v_name in variants_to_delete:
            variants.pop(v_name, None)
        if variants:
            products[product]["variants"] = variants
        else:
            products.pop(product, None)

    try:
        # 使用 io.open 并指定 utf-8，这是解决 Python 2/3 乱码问题的通用方法
        with io.open(file_path, 'w', encoding='utf-8') as f:
            output = json.dumps(
                { "time" : get_china_time(), "products": products},
                indent=4,
                ensure_ascii=False,
                sort_keys=False
            )
            # Python 2 中 dumps 返回的是 str(utf-8)，Python 3 返回的是 unicode
            # io.open 在 Python 2 中期望输入 unicode，所以需要转换
            if sys.version_info[0] < 3 and isinstance(output, str):
                output = output.decode('utf-8')
            f.write(output)
    except Exception as e:
        print("save error: {0}".format(str(e)))
        raise

def update_products_json():
    products = open_json()
    # 路径根据实际环境可能需要调整
    sdk_release_dirs_2_json("/data/kendryte-download/k230/release/linux_sdk_images", products, 20)
    #sdk_release_dirs_2_json("/data/kendryte-download/developer/releases/canmv_k230_micropython", products, 10)
    sdk_release_dirs_2_json("/data1/k230/release/linux_sdk_images/", products, 20)
    save_json(products, "products.json")

if __name__ == "__main__":
    update_products_json()
