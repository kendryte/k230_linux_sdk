#!/usr/bin/env python3
# coding=utf-8
'''
Copyright (c) CompanyNameMagicTag 2012-2021. All rights reserved.
Description: mk
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

g_params = {}
g_core_name = ""
g_prim_id_set = set()
g_mk_params = []


def mk_param_dic():
    global g_core_name
    global g_mk_params
    key='default_key'
    key_val=''
    i=0
    while i < len(sys.argv):
        res = re.search("(.*_core$)", sys.argv[i])
        if res is not None:
            g_core_name = res.groups()[0]
        if sys.argv[i]=='_PYTHON_ARG_':
            if key!='default_key':
                g_params[key]=key_val
            else:
                pass

            key = sys.argv[i+1]
            key_val = ''
            i=i+1
        else:
            key_val = '%s%s '%(key_val, sys.argv[i])
        i=i+1

    if key!='default_key':
        g_params[key]=key_val

    i = 0
    while i < len(sys.argv):
        g_mk_params.append(sys.argv[i])
        i += 1
    if sys.argv[2] == 'platform':
        g_core_name = 'platform'


def get_msg_root(subsystem):
    if subsystem is None:
        return
    msg_root_element = ET.Element('')
    for child in subsystem:
        if child.tag == 'MSG_LOG':
            msg_root_element = child
            break
    if  msg_root_element.tag == 'MSG_LOG':
        temp_attrib = msg_root_element.attrib
        temp_text = msg_root_element.text
        temp_tail = msg_root_element.tail
        msg_root_element.attrib = temp_attrib
        msg_root_element.text = temp_text
        msg_root_element.tail = temp_tail
    return msg_root_element


def add_content_to_xml(file_name, msg_root_element):
    if msg_root_element is None:
        return
    file_name = os.path.abspath(file_name)
    with open(file_name, 'r', encoding='UTF-8') as src_fp:
        for line in src_fp:
            match_st = re.search('_PRIM_ST', line)
            match_pri = re.search(', _PRIM_PRI_ = ', line)
            match_id =  re.search(', _PRIM_ID_ = ', line)
            match_sz = re.search(', _PRIM_SZ_ = ', line)
            match_line = re.search(', _PRIM_LINE_ = ', line)
            match_file = re.search(', _PRIM_FILE_ = ', line)
            match_file_id = re.search(', _PRIM_FILE_ID_ = ', line)
            match_type = re.search(', _PRIM_TYPE_ = ', line)
            match_end = re.search(', _PRIM_END_', line)
            if match_st and match_pri and match_id and match_sz and \
                match_line and match_file and match_file_id and \
                match_type and match_end:

                prim_pri  = line[match_pri.end():match_id.start()]
                prim_id = line[match_id.end():match_sz.start()]
                prim_sz = line[match_sz.end():match_line.start()].strip(r'"')
                prim_line = line[match_line.end():match_file.start()]
                prim_file = line[match_file.end():match_file_id.start()]
                prim_file_id = line[match_file_id.end():match_type.start()]
                prim_match_type = line[match_type.end():match_end.start()].strip(r'"')

                if int(prim_pri) == 2:
                    real_pri = int(prim_pri)
                    pri_str = 'ERROR'
                elif int(prim_pri) == 1:
                    real_pri = int(prim_pri)
                    pri_str = 'WARNING'
                elif int(prim_pri) == 0:
                    real_pri = int(prim_pri)
                    pri_str = 'INFO'
                elif int(prim_pri) == 3:
                    real_pri = int(0)
                    pri_str = 'BUF'

                if prim_id.isdigit():
                    if int(prim_id) == 0:
                        real_id = int(prim_line)
                    else:
                        real_id = int(prim_id)
                        # 去重
                        if real_id in g_prim_id_set:
                            continue
                        else:
                            g_prim_id_set.add(real_id)

                else:
                    i = 1
                    while 1:
                        if not prim_id[-i:].isdigit():
                            break
                        i = i + 1

                    if(i == 1):
                        real_id = 0
                    else:
                        i = i - 1
                        real_id = int(prim_id[-i:])
                xml_id = \
                    (int(prim_file_id) << 18) |  (real_id << 4) | (real_pri)
                new_element = ET.Element('')
                new_element.tag = 'MSG'
                new_element.attrib['STRUCTURE'] = prim_match_type
                new_element.attrib['NAME'] = \
                    prim_sz+' @'+prim_file+'('+prim_line+'),'+pri_str
                new_element.attrib['ID'] = hex(xml_id)
                new_element.tail = '\n\t\t\t'
                msg_root_element.append(new_element)
            else:
                pass


def write_prim_xml_tree(tree, dst_xml_file):
    dst_xml_file_dir = os.path.dirname(dst_xml_file)
    if not os.path.exists(dst_xml_file_dir):
        os.makedirs(dst_xml_file_dir)
    tree.write(dst_xml_file, encoding="US-ASCII", \
        xml_declaration = '<?xml version="1.0" encoding="US-ASCII" ?>',\
        method="xml", short_empty_elements=True)


def get_subsystem_by_name(tree, name):
    root = tree.getroot()
    for child in root:
        if child.attrib["NAME"] == name:
            return child


def write_to_target(tree_target, subsystem, name):
    root = tree_target.getroot()
    for child in root:
        if child.attrib["NAME"] == name:
            root.remove(child)
            root.append(subsystem)
            break


def mk_prim_xml_main():
    print('enter mk_prim_xml_main in xml step2')
    mk_param_dic()

    log_file_name = g_params['LOG_FILE_NAME'].strip()
    log_file_dir = os.path.dirname(log_file_name)
    if not os.path.isdir(log_file_dir):
        os.makedirs(log_file_dir)
    src_xml_file = g_params['PRIM_XML_SRC_XML_FILE'].strip()
    dst_xml_file = g_params['PRIM_XML_DST_XML_FILE'].strip()
    dst_xml_dir = os.path.dirname(dst_xml_file)
    if not os.path.isdir(dst_xml_dir):
        os.makedirs(dst_xml_dir)

    if not os.path.exists(dst_xml_file):
        shutil.copy(src_xml_file, dst_xml_file)

    with open(log_file_name, 'w+') as log_file_fp:
        last_print_target = sys.stdout
        sys.stdout = log_file_fp
        tree_src = ET.parse(src_xml_file)
        subsystem = get_subsystem_by_name(tree_src, g_core_name)
        msg_root_element = get_msg_root(subsystem)
        base_file_name = os.path.join(g_params['PRIM_XML_TEMP_BASE_ROOT_DIR'].strip(), g_core_name + ".cfg")
        add_content_to_xml(base_file_name, msg_root_element)
        tree_target = ET.parse(dst_xml_file)
        write_to_target(tree_target, subsystem, g_core_name)
        write_prim_xml_tree(tree_target, dst_xml_file)
        sys.stdout = last_print_target

    # copy mss_cmd_db.xml to output\hdb\hdbcfg
    if not os.path.exists(g_params["CMD_XML_DST_XML_FILE"].strip()):
        shutil.copy(g_params["CMD_XML_SRC_XML_FILE"].strip(), g_params["CMD_XML_DST_XML_FILE"].strip())
    exit(0)
g_strFlag                = ("[%d|%u|%x|%02x|%p|%08x|%ld|%2x|%2X]")  #日志关键字规则
g_strAppendFlag = ["ERROR", "WARNING", "INFO"]
g_primIds = [0x102, 0x103, 0x105]
WDK_HEAD_LEN           = 2
LOG_LINE_CNT_VAL_LEN   = 4
LOG_NUM_VAL_LEN        = 4
LOG_STR_LEN_VAL_LEN    = 4
LOG_TIME_VAL_LEN       = 8
FILE_ID_VAL_LEN        = 4
WDK_OFFSET_VAL_LEN     = 4
WDK_LEN_VAL_LEN        = 4
FILE_NAME_MAX_LEN      = 256
WDK_INFO_LEN           = FILE_ID_VAL_LEN + WDK_OFFSET_VAL_LEN + WDK_LEN_VAL_LEN  #共12字节
# 将二进制的wdk文件解析成database的xml格式
def parseWdkFileToXml(wdkFilePath, xmlDstPath, xmlSrcPath):
    if os.path.exists(wdkFilePath):
        with open(wdkFilePath, 'rb') as wdkFilefd:
            if not wdkFilefd:
                print('parseWdkFile::open wdk file [%s] fail.'%(wdkFilePath))
                return -1
            tree_src = ET.parse(xmlSrcPath)
            subsystem = get_subsystem_by_name(tree_src, "host_core")
            msg_root_element = get_msg_root(subsystem)

            # 创建txt文件
            pos = wdkFilePath.rfind('.')
            #debugPrint(pos)
            wdkTextFilePath = xmlSrcPath
            # 获取WdkFileNumbers
            fileNum = struct.unpack("h", wdkFilefd.read(WDK_HEAD_LEN))[0]
            # 获取文件名偏移值
            wdkFilefd.seek(WDK_HEAD_LEN + (fileNum * WDK_INFO_LEN))
            fileNameOffset = struct.unpack("i", wdkFilefd.read(WDK_OFFSET_VAL_LEN))[0]
            # debugPrint(fileNameOffset)
            for i in range(0, fileNum):
                # 获取fileID和wdkOffset
                wdkFilefd.seek(WDK_HEAD_LEN + (WDK_INFO_LEN * i))
                fileID, wdkOffset = struct.unpack("ii",wdkFilefd.read(FILE_ID_VAL_LEN + WDK_OFFSET_VAL_LEN))

                # 获取fileName
                wdkFilefd.seek(fileNameOffset + (FILE_NAME_MAX_LEN * i))
                fileNameStr = wdkFilefd.read(FILE_NAME_MAX_LEN).decode('utf8', "ignore").strip("\0")
                # 获取单WDK的log总数
                wdkFilefd.seek(wdkOffset + FILE_ID_VAL_LEN + LOG_TIME_VAL_LEN)
                logNum = struct.unpack("i", wdkFilefd.read(LOG_NUM_VAL_LEN))[0]
                # 循环获取每条log的具体打印内容
                singleWdkReadOffset = wdkOffset + FILE_ID_VAL_LEN + LOG_TIME_VAL_LEN + LOG_NUM_VAL_LEN
                for j in range(0, logNum):
                    # 获取单log的字符串长度
                    wdkFilefd.seek(singleWdkReadOffset + LOG_LINE_CNT_VAL_LEN)
                    logStrLen = struct.unpack("i", wdkFilefd.read(LOG_STR_LEN_VAL_LEN))[0]
                    # 获取单log的字符串
                    logStr = wdkFilefd.read(logStrLen).decode('utf8', "ignore")
                    singleWdkReadOffset += (LOG_LINE_CNT_VAL_LEN + LOG_STR_LEN_VAL_LEN + logStrLen)
                    fileId, lineNum, string = logStr.split(',', 2)

                    try:
                        primIdInfo = (int(fileId) << 18) |  ((int(lineNum) & 0xffff) << 4) | (5)
                        primIdWarning = (int(fileId) << 18) |  ((int(lineNum) & 0xffff) << 4) | (3)
                        primIdError = (int(fileId) << 18) |  ((int(lineNum) & 0xffff) << 4) | (2)
                        g_primIds[2] = primIdInfo
                        g_primIds[1] = primIdWarning
                        g_primIds[0] = primIdError
                    except ValueError as e:
                        print("fileId={} lineNum={}".format(fileId, lineNum))

                    preCount = logStr.replace("%%", "").count('%')
                    structName = "diag_log_msg" + str(preCount)
                    index = 0
                    for item in g_strAppendFlag:
                        new_element = ET.Element('')
                        new_element.tag = 'MSG'
                        new_element.attrib['STRUCTURE'] = structName
                        new_element.attrib['NAME'] = \
                            string.strip("\0") +' @'+fileNameStr+'('+lineNum+'),'+item
                        new_element.attrib['ID'] = hex(g_primIds[index])
                        new_element.tail = '\n\t\t\t'
                        index += 1
                        msg_root_element.append(new_element)
            write_to_target(tree_src, subsystem, "host_core")
            write_prim_xml_tree(tree_src, xmlDstPath)
        return 0
    else:
        print('parseWdkFile::wdk file %s did not exist.'%(wdkFilePath))
        return -1

    return 0

if __name__ == '__main__':
    # 1: hso output dir
    # 2: core name
    # 3: src xml file
    # 4: hso output xml file
    mk_param_dic()
    if g_core_name == 'platform' or g_core_name == "wifi_core" or g_core_name == "bt_core":
        print(os.path.abspath(g_mk_params[1]))
        # parse wdk to xml功能分支
        print('Start parsing wdk file')
        if 0 == parseWdkFileToXml(g_mk_params[1] + '/Total.wdk', g_mk_params[4], g_mk_params[3]):
            print('wdk Parse successful.')
        else:
            print('wdk Parse fail.')

    else:
        mk_prim_xml_main()