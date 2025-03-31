#!/usr/bin/env python3
# coding=utf-8
# Copyright (c) CompanyNameMagicTag 2024-2024. All rights reserved.
"""
将sdk中的/output/bin目录的.ini文件按.config配置文件进行更新
使配置项按Key=NewValue 方式将在.config配置项的值更新在.ini文件中
"""

import csv
import sys
import os
import shutil
import argparse
from hconfig import read_config_file


class IniMapping:
    def __init__(self, config_name="", ini_key_name="", note=""):
        self.config_name = config_name
        self.ini_key_name = ini_key_name
        self.note = note


def refresh_ini(config_path, ini_map_file, ini_file):
    """
    读取.config文件,按照csv对照表,更新ini文件
    """
    # csv对照表
    ini_re_mapping = {}
    if os.path.isfile(ini_map_file):
        with open(ini_map_file, newline="", encoding="gb2312") as f:
            reader = csv.reader(f)
            for row in reader:
                if reader.line_num == 1:
                    continue
                ini_re_mapping[row[1].strip()] = IniMapping(row[0].strip(), row[1].strip(), row[2].strip())

    # 读取.config文件
    conf_dict = read_config_file(config_path)

    if not os.path.isfile(ini_file):
        return
    with open(ini_file, encoding="utf-8", mode="r") as ini_f:
        lines = ini_f.readlines()
        # 预更新的ini文件内容
    new_lines = []
    for line in lines:
        line_sp = line.strip()
        if not line_sp or line.count("=") != 1:
            new_lines.append(line_sp)
            continue
        k, v = [i.strip() for i in line.split("=")]
        if not k or not v or k.startswith("#") or k not in ini_re_mapping:
            new_lines.append(line_sp)
            continue
        if ini_re_mapping[k].config_name not in conf_dict:
            print("the item %s in ini file isnot in the autoconfig.h file." % line_sp)
            new_lines.append(line_sp)
        else:
            bb = conf_dict[ini_re_mapping[k].config_name]
            new_lines.append(k + "=" + str(bb))
    # 更新ini文件
    with open(ini_file, encoding="utf-8", mode="w") as new_ini_f:
        new_ini_f.write("\n".join(new_lines))


def read_config_h(config_h_path):
    """
    读取autoconfig.h中的各项值
    """
    conf_dict = {}
    if not os.path.isfile(config_h_path):
        return conf_dict
    try:
        with open(config_h_path, encoding="utf-8", mode="r") as con_f:
            conf_lines = con_f.readlines()
    except UnicodeEncodeError:
        raise Exception("ini file has encoding error, check it.")
    except Exception:
        raise Exception("ini file has error, check it.")

    for line in conf_lines:
        line_sp = line.strip()
        if not line_sp:
            continue
        if line_sp.count(" ") < 2 or not line_sp.startswith("#define"):
            print("the item %s in autoconfig.h is not valid." % line_sp)
            continue
        sp_index = line_sp[8:].index(" ")
        k, v = line_sp[8: sp_index + 8].strip(), line_sp[sp_index + 8:].strip()
        if not k or not v:
            print("the item %s in autoconfig.h is not startswith #define." % line_sp)
            continue
        if v.startswith("0x") or v.startswith("0X"):
            conf_dict[k] = v
            continue
        v_int = v
        try:
            v_int = int(v)
        except (TypeError, ValueError):
            pass
        conf_dict[k] = v_int
    return conf_dict


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Modify the configuration items in menuconfig to ini parameters")
    parser.add_argument('ini_map_file',
                        help="The mapping relationship file between menuconfig configuration and ini parameter name(csv file)")
    parser.add_argument('kconfig_file', help="Kconfig file")
    parser.add_argument('kconfig_config_file', help="Menuconfig config file to read")
    parser.add_argument('ini_file', help="ini file to write")

    args = parser.parse_args()
    INI_MAP_FILE = args.ini_map_file
    # kconfig_file： args.kconfig_file
    KCONFIG_CONFIG_FILE = args.kconfig_config_file
    INI_FILE = args.ini_file

    refresh_ini(KCONFIG_CONFIG_FILE, INI_MAP_FILE, INI_FILE)
