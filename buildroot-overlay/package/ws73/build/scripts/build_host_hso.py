#!/usr/bin/env python3
# coding=utf-8
# Copyright (c) CompanyNameMagicTag 2021-2022. All rights reserved.


import argparse
import os
import re
import sys
import configparser
import shutil
import copy
import xml.etree.ElementTree as ET


def print_verbose(verbose: bool, *args, **kw):
    '''print verbose '''
    if verbose:
        print(*args, **kw)


def execute_script(script_path: str, *args) -> int:
    '''exec python scripts'''
    return os.system('{} {} {}'.format(sys.executable, script_path, " ".join(args)))


def resort_xml_element_l3(src_xml_path, sort_keys=("ID", "STRUCTURE", "NAME")):
    """
    sort xml in forward 3 rank
    """
    src_tree = ET.parse(src_xml_path)
    src_root = src_tree.getroot()
    for sub_system in src_root:
        for i, msg_log in enumerate(sub_system):
            msg_ls = list()
            for msg in msg_log:
                msg_ls.append(msg)
            msg_ls.sort(key=lambda x: [x.attrib.get(j, None) for j in sort_keys])
            msg_log_new = copy.deepcopy(msg_log)
            msg_log_new.clear()
            msg_log_new.tail = msg_log.tail
            msg_log_new.text = msg_log.text
            for msg_k in msg_ls:
                msg_log_new.append(msg_k)
            sub_system[i] = msg_log_new

    src_tree.write(src_xml_path, encoding='US-ASCII', xml_declaration=True, method="xml", short_empty_elements=True)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="build host hso databases!")
    parser.add_argument('-c', '--config-base', help="Specify the base config file path(init file type)", required=True)
    parser.add_argument('-v', '--verbose', help="Print more information", action='store_true')

    args = parser.parse_args()

    verbose = args.verbose

    # parse config file
    config = configparser.ConfigParser()
    config.read(args.config_base)
    point_script = config['scripts']['point_script']
    step_script = config['scripts']['generate_wdk_script']
    merge_script = config['scripts']['merge_script']
    wdk_header = config['base config']['wdk_header']
    base_xml_file = config['base config']['base_xml']
    host_xml_file = config['base config']['host_xml']
    device_xml_file = config['base config']['device_xml']
    merge_xml_file = config['base config']['merge_xml']
    host_hso_wdk_dir = config['base config']['host_hso_wdk_dir']

    scan_dirs = config['host_scan']['scan_dirs'].split('\n')
    re_excludes = re.compile(config['host_scan']['re_exclued_path']) if config['host_scan']['re_exclued_path'] else None
    re_includes = re.compile(config['host_scan']['re_include_path']) if config['host_scan']['re_include_path'] else None

    # check path
    path_list = [point_script, step_script, merge_script, wdk_header, base_xml_file, device_xml_file]
    for path_i in path_list:
        if not os.path.exists(path_i):
            print('[-] Path `{}` is not exists!!'.format(path_i))
            exit(1)

    # set hso temp dir
    if not os.path.exists(host_hso_wdk_dir):
        os.makedirs(host_hso_wdk_dir)
    else:
        # clean wdk files
        for fname in os.listdir(host_hso_wdk_dir):
            if fname.endswith('.wdk'):
                os.remove(os.path.join(host_hso_wdk_dir, fname))

    os.environ['HSO_WDK_DIR'] = host_hso_wdk_dir

    # scan dirs for generate wdk files
    for dpath in scan_dirs:
        for dirpath, dirnames, filenames in os.walk(dpath):
            if re_excludes and re_excludes.search(dirpath):
                print_verbose(verbose, '[*] Skip excludes file `{}`.'.format(dirpath))
                continue
            for fname in filenames:
                fpath = os.path.join(dirpath, fname)
                if re_excludes and re_excludes.search(fpath) or re_includes and not re_includes.search(fpath):
                    print_verbose(verbose, '[*] Skip excludes file `{}`.'.format(fpath))
                    continue
                ex_result = execute_script(point_script, fpath, wdk_header)
                if ex_result != 0:
                    print("[-] Generate wdk for {} failed!!".format(fpath))
                    exit(1)

                print_verbose(verbose, "[+] Generate wdk for {}.".format(fpath))

    # merge wdk files
    print_verbose(verbose, "[*] Merge wdk files in {}.".format(host_hso_wdk_dir))
    ex_result = execute_script(point_script, 'MERGE', host_hso_wdk_dir)
    if ex_result != 0:
        print("[-] Merge wdk files in {} failed!!".format(host_hso_wdk_dir))
        exit(1)

    # generate host xml form wdk file
    # the xml file is the hso databse file
    print_verbose(verbose, "[*] Generate xml from wdk files.")
    ex_result = execute_script(step_script, host_hso_wdk_dir, 'wifi_core', base_xml_file, host_xml_file)
    if ex_result != 0:
        print("[-] Generate xml from wdk files failed!!")
        exit(1)
    print('[+] Generate host hso xml file `{}`!'.format(host_xml_file))

    # merge host and device xml file
    print_verbose(verbose, "[*] Merge host and device xml file.")
    ex_result = execute_script(merge_script, device_xml_file, host_xml_file, merge_xml_file)
    if ex_result != 0:
        print("[-] Merge host and device xml file failed!!")
        exit(1)
    print('[+] Generate hso xml file `{}`!'.format(merge_xml_file))
    # resort xml 
    resort_xml_element_l3(device_xml_file)
    resort_xml_element_l3(host_xml_file)
    resort_xml_element_l3(merge_xml_file)
