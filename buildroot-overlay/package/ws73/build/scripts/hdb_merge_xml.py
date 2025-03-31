#!/usr/bin/env python3
# coding=utf-8
'''
Copyright (c) CompanyNameMagicTag 2012-2021. All rights reserved.
Description: merge hdb
'''

import os
import time
import string
import re
import shutil
import hashlib
import binascii
import sys
import xml.etree.ElementTree as ET
import struct
import platform
import subprocess


def merge_xml(src_xml_path, dst_xml_path, out_xml_path):
    if not os.path.exists(src_xml_path):
        print('SrcFile is not Exist', src_xml_path)
        return

    if not os.path.exists(dst_xml_path):
        print('DstFile is not Exist', dst_xml_path)
        return

    src_tree = ET.parse(src_xml_path)
    dst_tree = ET.parse(dst_xml_path)
    src_root = src_tree.getroot()
    dst_root = dst_tree.getroot()

    target_list = list()

    for child in src_root:
        # print(child.tag, child.attrib)
        if (child.get('NAME') == 'ws73_mcore'):
            for subchild in child:
                for target in subchild:
                    target_list.append(target)
                break

    for child in dst_root:
        # print(child.tag, child.attrib)
        if (child.get('NAME') == 'host_core'):
            for subchild in child:
                for target in target_list:
                    subchild.append(target)
                break

    dst_tree.write(out_xml_path, encoding='US-ASCII', xml_declaration=True)
    print('=== Merge hdb XML Success! ===')
    return


def help():
    print("**************************************************************")
    print("*hdb_merge_xml version is Ver. 1.0.0    2022/11/28           *")
    print("*hdb_merge_xml help:                                         *")
    print("*    hdb_merge_xml SrcFile DstFile OutFile                   *")
    print("**************************************************************")


if __name__ == '__main__':
    print('=== Merge hdb XML Start: ===')

    if 4 > len(sys.argv) or 4 < len(sys.argv):
        help()
        print("merge xml script argc is error! argc:", len(sys.argv))

    # 源文件路径 for 'device_mss_prim_db.xml'
    src_xml_path = sys.argv[1]

    # 合并目标文件路径 for 'wifi_core_host_mss_prim_db.xml'
    dst_xml_path = sys.argv[2]

    # 生成文件路径 for 'mss_prim_db.xml'
    out_xml_path = sys.argv[3]

    print(' SrcFile:', sys.argv[1])
    print(' DstFile:', sys.argv[2])
    print(' OutFile:', sys.argv[3])

    merge_xml(src_xml_path, dst_xml_path, out_xml_path)
