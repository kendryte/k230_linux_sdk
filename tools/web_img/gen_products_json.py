#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import os
import json
import hashlib
import re
from datetime import datetime

def get_md5_from_file(file_path):
    md5_file = file_path + ".md5"
    if os.path.exists(md5_file):
        with open(md5_file, 'r') as f:
            return f.read().split()[0]
    return "N/A"

def get_dir_creation_time(dir_path):
    if not os.path.exists(dir_path):
        return f"dir '{dir_path}' not exist"

    dir_stat = os.stat(dir_path)

    try:
        create_timestamp = dir_stat.st_birthtime
    except AttributeError:
        create_timestamp = dir_stat.st_ctime

    create_datetime = datetime.fromtimestamp(create_timestamp)
    return f"{create_datetime.strftime('%Y-%m-%d %H:%M:%S')}"

def get_version_from_filename(filename:str)->str:
        if "micropython" in filename:
            pattern = r'v\d+\.\d+\.\d+(?:-\w+)?'
        else:
            pattern = r'v\d+\.\d+(?:\.\d+)?'

        m = re.search(pattern, filename)
        return m.group(0) if m else "unknown"

def get_product_name_from_filepath(file_path:str)->str:
    if "linux_sdk_images" in file_path:
        return os.path.basename(os.path.dirname(file_path))
    elif "micropython" in file_path:
        filename = os.path.basename(file_path)
        product_ = filename.split("_micropython")[0]
        product_dict={
            "CanMV_K230_01Studio"        :  "k230_canmv_01studio_defconfig" , 
            "CanMV_K230_LCKFB"           :  "k230_canmv_lckfb_defconfig"   ,
            "CanMV_K230_V1P0_P1"         :  "k230_canmv_v1p0p1_defconfig"  ,
            "CanMV_K230_V3P0"            :  "k230_canmv_v3p0_defconfig"
        }
        return product_dict[product_] if product_ in product_dict else ""
    


def update_file_to_json(file=".", products={}):
    filename = os.path.basename(file)
    if not file.endswith(".img.gz"):
        return

    product_name = get_product_name_from_filepath(file)
    if product_name not in products:
        #print(f"产品 {product_name} 不在 products 列表中，跳过。")
        return

    variant_key = "linux" if "linux" in filename else "debian" if "debian" in filename \
                                else "micropython" if "micropython" in filename else "unknown"
    variants = products[product_name].get("variants", {})        
    if variant_key not in variants:
        #print(f"变体 {variant_key} 不在产品 {product_name} 的变体列表中，跳过。")
        return
    #version_type = "latest" if "daily_build" in filename else "history" daily_build
    version_type = "latest" if ("latest" in file or "daily_build" in file) else "history"
    #print(f"proc: {file}  {product_name}, : {variant_key} {version_type}")

    if version_type == "latest":
        variants[variant_key][version_type] = {
            "version": get_version_from_filename(filename),
            "date": get_dir_creation_time(os.path.dirname(file)),
            "url": f"./linux_sdk_images/v0.6.5/{product_name}/{filename}",
            "md5": get_md5_from_file(file)
        }
    else:
        ver_info = {
            "version": get_version_from_filename(filename),
            "date": get_dir_creation_time(os.path.dirname(file)),
            "url": f"./linux_sdk_images/v0.6.5/{product_name}/{filename}",
            "md5": get_md5_from_file(file)
        }
        variants[variant_key][version_type].append(ver_info)

    products[product_name]["variants"] = variants

def update_directory_to_json(dir=".", products={}):
    file_info_list = []
    for root, _, files in os.walk(dir):
        for f in files:
            file_path = os.path.join(root, f)
            file_mtime = os.path.getmtime(file_path)
            file_info_list.append((file_mtime, file_path))
    #sort files by mtime descending
    file_info_list.sort(key=lambda x: x[0], reverse=False)

    for _, file_path in file_info_list:
        update_file_to_json(file_path, products)

def sdk_release_dirs_2_json(base_dir=".", products={}, max_dirs=3):
    excludefiles =[] # ["latest"]
    subdirs = [
        os.path.join(base_dir, d)
        for d in os.listdir(base_dir)
        if d not in excludefiles  and os.path.isdir(os.path.join(base_dir, d))
    ]
    subdirs.sort(key=os.path.getmtime, reverse=True)

    for d in subdirs[:max_dirs]:
        update_directory_to_json(d, products)

def open_json(file_path:str)->dict:
    try:
        with open(file_path, encoding='utf-8') as f:
            data = json.load(f)
        #print(f"type of is : {type(data.get('products', {}))}")
        return data.get('products', {})
    except (OSError, json.JSONDecodeError) as e:
        print(f"read  {file_path} failed: {e}")
        exit(1)

def save_json(products, file_path):
    try:
        with open(file_path, 'w', encoding='utf-8') as f:
            json.dump(
                {"products": products},
                f,
                indent=4,
                ensure_ascii=False
            )
    except PermissionError:
        raise PermissionError(f"save error no perment  {file_path}")
    except Exception as e:
        raise Exception(f"save error {str(e)}") from e


def update_products_json(target_dir="."):
    products = open_json(".mod_products.json")
    #sdk_release_dirs_2_json("/data1/k230/release/linux_sdk_images/",products, 5)
    sdk_release_dirs_2_json("micropython",products, 5)
    save_json(products ,"products.json")


if __name__ == "__main__":
    update_products_json("")
