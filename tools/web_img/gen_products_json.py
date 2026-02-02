#!/usr/bin/env python
# -*- coding: utf-8 -*-
import os
import json
import hashlib
import re
import sys
from datetime import datetime
from collections import OrderedDict

reload(sys)
sys.setdefaultencoding('utf-8')


json_str = '''
{
    "products": {
        "k230_canmv_01studio_defconfig": {
            "name": "01studio",
            "description": "01studio canmv",
            "image_url": "https://www.kendryte.com/api/post/attachment?id=833",
            "variants": { "linux" :{"latest": {}, "history": []},"debian" : {"latest": {}, "history": [] },"micropython" : {"latest": {}, "history": []} }
        },
        "k230_canmv_lckfb_defconfig": {
            "name": "嘉立创k230",
            "description": "k230_canmv_lckfb",
            "image_url": "https://www.kendryte.com/api/post/attachment?id=832",
            "variants": { "linux" :{"latest": {}, "history": []},"debian" : { "latest": {}, "history": []},"micropython" : {"latest": {}, "history": []} }
        },
        "k230_canmv_v3_defconfig": {
            "name": "CanMV V3.0",
            "description": "CanMV K230 V3.0(创乐博)",
            "image_url": "https://www.kendryte.com/api/post/attachment?id=834",
            "variants": { "linux" :{"latest": {}, "history": []},"debian" : {"latest": {}, "history": [] },"micropython" : {"latest": {}, "history": []} }
        },
        "k230_canmv_dongshanpi_defconfig": {
            "name": "dongshanpai",
            "description": "百问网，东山派",
            "image_url": "https://eai.100ask.net/assets/images/image-20240729155648319-504e2c8f5bc5607d6e03148239c45087.png",
            "variants": { "linux" :{"latest": {}, "history": []},"debian" : {"latest": {}, "history": [] },"micropython" : {"latest": {}, "history": []} }
        },
        "k230_canmv_defconfig": {
            "name": "CanMV 1.0/1.1",
            "description": "CanMV K230 1.0/1.1",
            "image_url": "https://www.kendryte.com/k230_canmv/main/_images/CanMV-K230_front.png",
            "variants": { "linux" :{"latest": {}, "history": []},"debian" : {"latest": {}, "history": [] },"micropython" : {"latest": {}, "history": []} }
        },
        "BPI-CanMV-K230D-Zero_defconfig": {
            "name": "BPI-CanMV-K230D",
            "description": "BPI-CanMV-K230D",
            "image_url": "https://www.kendryte.com/api/post/attachment?id=482",
            "variants": { "linux" :{"latest": {}, "history": []},"debian" : {"latest": {}, "history": [] },"micropython" : {"latest": {}, "history": []} }
        },
        "k230_canmv_01studio_emmc_defconfig": {
            "name": "01studio emmc ",
            "description": "01studio canmv base emmc board",
            "image_url": "https://www.kendryte.com/api/post/attachment?id=835",
            "variants": { "linux" :{"latest": {}, "history": []},"debian" : {"latest": {}, "history": [] },"micropython" : {"latest": {}, "history": []} }
        }
    }
}
'''


def get_md5_from_file(file_path):
    md5_file = file_path + ".md5"
    if os.path.exists(md5_file):
        with open(md5_file, 'r') as f:
            return f.read().split()[0]
    return "N/A"

def get_dir_creation_time(dir_path):
    if not os.path.exists(dir_path):
        return "dir '{0}' not exist".format(dir_path)

    dir_stat = os.stat(dir_path)

    try:
        # Python 2.7 在某些平台上可能没有 st_birthtime
        create_timestamp = getattr(dir_stat, 'st_birthtime', dir_stat.st_ctime)
    except AttributeError:
        create_timestamp = dir_stat.st_ctime

    create_datetime = datetime.fromtimestamp(create_timestamp)
    return create_datetime.strftime('%Y-%m-%d %H:%M:%S')

def get_version_from_filename(filename):
    if "micropython" in filename:
        pattern = r'v.\d+\.\d+(?:-\w+)?'
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
            "CanMV_K230_V1P0_P1": "k230_canmv_v1p0p1_defconfig",
            "CanMV_K230_V3P0": "k230_canmv_v3p0_defconfig"
        }
        return product_dict.get(product_, "")
    return ""

def get_url(file_path):
    a = file_path.replace("/data/kendryte-download", "https://kendryte-download.canaan-creative.com")
    return a.replace("/data1/k230/release/", "https://ai.b-bug.org/k230/release/")

def update_file_to_json(file_path, products):
    filename = os.path.basename(file_path)
    if not file_path.endswith(".img.gz"):
        return

    product_name = get_product_name_from_filepath(file_path)
    if product_name not in products:
        return

    if "linux" in filename:
        variant_key = "linux"
    elif "debian" in filename:
        variant_key = "debian"
    elif "micropython" in filename:
        variant_key = "micropython"
    else:
        variant_key = "unknown"

    variants = products[product_name].get("variants", {})
    if variant_key not in variants:
        return

    version_type = "latest" if ("latest" in file_path or "daily_build" in file_path) else "history"

    ver_info = {
        "version": get_version_from_filename(filename),
        "date": get_dir_creation_time(os.path.dirname(file_path)),
        "url": get_url(file_path),
        "md5": get_md5_from_file(file_path)
    }

    if version_type == "latest":
        variants[variant_key][version_type] = ver_info
    else:
        # 确保 history 是列表
        if not isinstance(variants[variant_key].get(version_type), list):
            variants[variant_key][version_type] = []
        variants[variant_key][version_type].append(ver_info)

    products[product_name]["variants"] = variants

def update_directory_to_json(directory, products):
    file_info_list = []
    for root, _, files in os.walk(directory):
        for f in files:
            file_path = os.path.join(root, f)
            file_mtime = os.path.getmtime(file_path)
            file_info_list.append((file_mtime, file_path))

    # 排序
    file_info_list.sort(key=lambda x: x[0], reverse=False)

    for _, file_path in file_info_list:
        update_file_to_json(file_path, products)

def sdk_release_dirs_2_json(base_dir, products, max_dirs=3):
    if not os.path.isdir(base_dir):
        return
    excludefiles = []
    subdirs = [
        os.path.join(base_dir, d)
        for d in os.listdir(base_dir)
        if d not in excludefiles and os.path.isdir(os.path.join(base_dir, d))
    ]
    subdirs.sort(key=os.path.getmtime, reverse=True)

    for d in subdirs[:max_dirs]:
        update_directory_to_json(d, products)

def open_json():
    data = json.loads(json_str,object_pairs_hook=OrderedDict)
    return data.get('products', {})

def save_json(products, file_path):
    # 清理空的 variants
    for product in products:
        variants = products[product].get("variants", {})
        variants_to_delete = []
        for variant in variants:
            variant_info = variants[variant]
            latest = variant_info.get("latest", {})
            history = variant_info.get("history", [])

            if not latest and not history:
                variants_to_delete.append(variant)

        for variant in variants_to_delete:
            variants.pop(variant, None)

        products[product]["variants"] = variants

    try:
        with open(file_path, 'w') as f:
            # ensure_ascii=False 在 Python 2 中处理 unicode 很有用
            json_str = json.dumps(
                {"products": products},
                indent=4,
                ensure_ascii=False
            )
            f.write(json_str)
    except Exception as e:
        print("save error: {0}".format(str(e)))
        raise

def update_products_json():
    # 注意：确保 .mod_products.json 文件存在
    products = open_json()
    sdk_release_dirs_2_json("/data/kendryte-download/k230/release/linux_sdk_images", products, 5)
    sdk_release_dirs_2_json("/data/kendryte-download/developer/releases/canmv_k230_micropython", products, 5)
    sdk_release_dirs_2_json("/data1/k230/release/linux_sdk_images/", products, 5)
    save_json(products, "products.json")

if __name__ == "__main__":
    update_products_json()
