#!/usr/bin/env python
# coding=utf-8
'''
Copyright (c) CompanyNameMagicTag 2012-2021. All rights reserved.
Description: point trans
'''

import os
import re
import sys
import time
import struct
import platform
import subprocess

py_version = platform.python_version()


def debug_print(output):
    # print output
    return


def help():
    print("**************************************************************")
    print("*pointTrans version is Ver. 1.0.0    2016/11/21              *")
    print("*pointTrans help:                                            *")
    print("*    PointTrans CFile FileIDFile                             *")
    print("*    PointTrans Directory FileIDFile                         *")
    print("*    PointTrans merge Directory                              *")
    print("*    PointTrans parse wdkFile                                *")
    print("**************************************************************")


####################################################################################################
#
# 日志内容提取函数
#
####################################################################################################
# 日志状态机处理流程：
# LOG_ENTRY_FINDING              -- 查找日志接口
# LOG_ENTRY_FOUNDED              -- 日志接口匹配，查找日志内容
# LOG_CONT_ENTRY_FINDING         -- 日志内容入口匹配，以“号开关，以”号结束
# LOG_CONT_END_FOUNING           -- 日志内容查找
# LOG_EXIT_FINDING               -- 日志线束符匹配

g_newWdkFlag = 0x7e00ff7e  # 新格式标志位
g_FsmStatus = "LOG_ENTRY_FINDING"
g_logKeyFind = 0  # （状态位）判断是否查找到日志关键字，0否1是
g_logStr = ""
g_logEntry = (
    "OAM_+[INFO|WARNING|ERROR]+_LOG[0-4]+(\s*)\(",
    "[info|warning|error]+_log[0-4]+(\s*)\(",
    "log_print[0-4]+(\s*)\(",
    "[print|log]+_alter+(\s*)\(",
    "common_log_dbg[0-4]+(\s*)\(",
    "log[0-4]+(\s*)\(")  # 日志关键字规则
g_logFullCont = '\"(.*)\"'
g_logEntryOrEndFlag = -1  # 用于记录日志行数，日志多行时可能是第一行，有可能是最后一行； 0 表示第一行  1表示最后一行


# 根据日志状态机流程逐行解析日志内容
def parse_log_from_line(line):
    global g_FsmStatus
    global g_logStr
    global g_logEntry
    global g_logFullCont
    global g_logEntryOrEndFlag
    global g_logKeyFind

    findkey = 0
    # 匹配日志接口
    if "LOG_ENTRY_FINDING" == g_FsmStatus:
        g_logEntryOrEndFlag = -1
        g_logStr = ""
        for i in range(len(g_logEntry)):
            m = re.search(g_logEntry[i], line.strip())
            if m is not None:
                # print "line:%s, key:%s" % (line, g_logEntry[i])
                findkey = 1
                break
        if 1 == findkey:
            g_FsmStatus = "LOG_ENTRY_FOUNDED"
            g_logEntryOrEndFlag = 0
            g_logKeyFind = 1
            debug_print("log entry founded:%s" % (m.group()))
            parse_log_from_line(line)
        else:
            return

    # 日志接口匹配，确认是否完整日志；确认日志是否正常结束
    elif "LOG_ENTRY_FOUNDED" == g_FsmStatus:
        m1 = re.search(g_logFullCont, line)  # 完整日志格式
        m2 = re.search("\"(.+)", line)  # 部分日志格式 ，分号开头
        if m1 is not None:
            g_logStr = m1.groups()[0] + '\0'
            debug_print("g_logFullCont：%s" % (g_logStr))
            g_FsmStatus = "LOG_EXIT_FINDING"
            parse_log_from_line(line)
        elif m2 is not None:
            g_FsmStatus = "LOG_CONT_END_FINDING"
            g_logStr = m2.groups()[0]  # 分号后面的内容为日志内容
            debug_print("g_logPartCont：:%s" % (g_logStr))

    # 匹配部分日志接口
    elif "LOG_CONT_END_FINDING" == g_FsmStatus:
        m = re.search("[\s]+(.+)\"", line)
        if m is not None:
            debug_print("logContEnd:%s" % (m.groups()[0]))
            g_logStr += (m.groups()[0] + '\0')
            debug_print("g_logStr=%s" % (g_logStr))
            g_FsmStatus = "LOG_EXIT_FINDING"
            parse_log_from_line(line)
        else:
            g_logStr += line.strip()  # 纯日志内容 ，继续找日志
            debug_print("logCont:%s" % (g_logStr))

    # 匹配日志结束符
    elif "LOG_EXIT_FINDING" == g_FsmStatus:
        # debug_print("LOG_EXIT_FINDING:%s" % line)
        m = re.search("\)[\s]*\;", line.strip())
        if m is not None:
            g_FsmStatus = "LOG_ENTRY_FINDING"  # 继续逐行扫描下一条日志入口
            g_logEntryOrEndFlag = 1


# #####################################################################################
#
# 打点文件生成功能
#
# #####################################################################################
g_findFlag = 0  # 文件存在标识  0  不存在  1 存在
g_fileIdDict = {}  # fileIdStr  fileIdNum
g_fileCount = 0  # 打点文件遍历数目
g_lastFileIdNum = 0

g_strWdkHeaderFile = "oam_wdk.h"  # OAM_FILE_ID_XXX枚举文件
g_srcPath = " "
g_imagePath = " "
g_wdkPath = " "


# 写入指定数据与字节数至文件，长度不足填充0x00
def writeAsciiToFile(fd, fmt, val, bytes=None):
    byte_val = struct.pack("<%s" % fmt, val)
    fd.write(byte_val)
    return
    # print(val)
    if py_version.startswith("3"):
        for i in range(0, bytes):
            temp = chr((val & (0x000000FF << (i * 8))) >> (i * 8)).encode()
            # print("temp", temp)
            fd.write(temp)
    elif py_version.startswith("2"):
        for i in range(0, bytes):
            fd.write(chr((val & (0x000000FF << (i * 8))) >> (i * 8)))


def getFileIDfromwdk(fd):
    buffer = fd.read(4)
    fileID, = struct.unpack('i', buffer)
    return fileID


# 记录日志： 文件号 行号 日志内容
def writeLogToFile(fd, fileId, lineNum, logStr):
    writeAsciiToFile(fd, "i", lineNum, 4)
    logTemp = '%s,%s,%s' % (fileId, lineNum, logStr)
    loglenth = len(logTemp)
    writeAsciiToFile(fd, "i", loglenth, 4)

    if py_version.startswith("3"):
        fd.write(logTemp.encode())
    elif py_version.startswith("2"):
        fd.write(logTemp)


# ############################################################################################################################
# 遍历.c文件生成单个打点文件
# 格式：
# 头部部分（64个字节）：文件个数（2个字节）+FileID（4个字节）+位置偏移64（4个字节）+文件大小（4个字节）+文件名偏移（4个字节）
# 内容部分：【FileID（4个字节）+LastTime（8个字节）+打印行数（4个字节）】+打印内容（文件号，行号，内容）
#
# ############################################################################################################################
def createSingleWdkFile(cfilepath, destpath=''):
    global g_logKeyFind
    global g_newWdkFlag
    global g_logStr
    global g_FsmStatus

    normalFileFlag = 0  # 普通c文件标识
    currLineNum = 0  # 当前行行号
    logEndLineNum = 0  # 日志接口最后一行行号
    logCommentCnt = 0  # 日志总条数
    wdklenth = 16  # wdk长度（日志头长度（16）+所有日志长度）
    wdkFileName = ""  # 生成wdk的绝对路径
    lineNumSave = 1  # 保存查找到日志关键字的首行
    wdklogList = []  # 日志信息列表，【文件号， 行号， 日志内容】
    fileIdNum = 0  # 本wdk文件号

    m = re.search("\w.[cC|hH]", cfilepath)
    if m is None:
        print("Error!%s is not C or H File" % cfilepath)
        return

    # 增加对文件中换行符的处理
    destfile = cfilepath + "_temp"
    modify_newline_on_linux(cfilepath, destfile)
    # 打开源文件，读取文件内容
    with open(destfile, 'r', encoding='gbk', errors='replace') as fdSrc:
        if fdSrc is None:
            print("open file %s failed." % cfilepath)
            return
        datalines = fdSrc.readlines()
    os.system("rm %s" % destfile)

    filepath, filename = os.path.split(cfilepath)
    # print "filepath=%s, filename=%s" %(filepath, filename)
    normalFileFlag = 0
    currLineNum = 0

    # 遍历行内容
    for line in datalines:
        # 行号加1
        currLineNum += 1

        m = re.search('^\/\/', line.strip())
        if m is not None:
            continue
        # 判断是否可识别的普通日志文件
        if 0 == normalFileFlag:
            # 根据FILE_ID_XXX获取fileidNum， 若文件中的OAM_FILE_ID_XXX 宏不存在，忽略该错误
            m = re.search("#define[\s]+THIS_FILE_ID[\s]", line.strip())
            if m is not None:
                fileIdStr = line.replace("#define", "")
                fileIdStr = fileIdStr.replace("THIS_FILE_ID", "")
                fileIdStr = re.sub(r'\/\*.*\*+\/', '', fileIdStr)
                fileIdStr = fileIdStr.strip()
            else:
                continue

            try:
                fileIdNum = g_fileIdDict[fileIdStr]
            except KeyError as e:
                print("warning:%s not defined. fileName[%s]" % (fileIdStr, filename))
                raise

            # print("Notice:%s defined. fileName[%s]"%(fileIdStr, filename))
            normalFileFlag = 1
            logCommentCnt = 0
            g_FsmStatus = "LOG_ENTRY_FINDING"
        else:
            parse_log_from_line(line)
            if 1 == g_logKeyFind:
                lineNumSave = currLineNum
                g_logKeyFind = 0
            # 解析日志，判断是否找到，如果找到写入文件
            if 1 == g_logEntryOrEndFlag:
                # 如果日志多行，则所有行都将算进日志行
                for i in range(currLineNum - lineNumSave + 1):
                    g_logStr = g_logStr.replace('\\r', '')  # 清除\r，\n，空格
                    g_logStr = g_logStr.replace('\\n', '')
                    g_logStr.strip()
                    logstr = "%d,%d,%s" % (fileIdNum, lineNumSave + i, g_logStr)
                    # print("logstr", logstr)
                    wdklogList.append([fileIdNum, lineNumSave + i, g_logStr])
                    wdklenth += (8 + len(logstr))  # 每条日志前有8个字节日志头
                    g_logKeyFind = 0
                    logCommentCnt += 1

    # 如果未查找到文件号返回
    if 0 == normalFileFlag:
        # print "FileID define Error!"
        return

    # 如果无日志返回
    if logCommentCnt == 0:
        # print "c file have no log"
        return

    # 创建源文件相应的.wdk文件
    if os.path.isdir(destpath) and len(destpath) > 0:
        wdkFileName = os.path.join(destpath, filename) + "_%d.wdk" % fileIdNum
    else:
        wdkFileName = cfilepath + "_%d.wdk" % fileIdNum
    with open(wdkFileName, 'wb') as foutwdk:
        if foutwdk is None:
            print("wdk file open failed")
            return
        # 写入文件个数
        writeAsciiToFile(foutwdk, "h", 1)

        # 写入文件号
        writeAsciiToFile(foutwdk, "i", fileIdNum)

        # 写入日志内容偏移
        writeAsciiToFile(foutwdk, "i", 64)

        # 写入日志长度
        writeAsciiToFile(foutwdk, "i", wdklenth)

        foutwdk.seek(64, 0)  # 偏移到日志内容位置
        # 写入文件号
        writeAsciiToFile(foutwdk, "i", fileIdNum)

        # 写入时间，临时写入为0
        writeAsciiToFile(foutwdk, "q", 0)

        # 写入日志内容总行数
        writeAsciiToFile(foutwdk, "i", logCommentCnt)

        for i in range(len(wdklogList)):
            writeLogToFile(foutwdk, wdklogList[i][0], wdklogList[i][1], wdklogList[i][2])  # 写入日志内容

        filenameOffset = ((foutwdk.tell() + 80) / 16) * 16
        filenameOffset = int(filenameOffset)

        foutwdk.seek(2 + 12, 0)  # 偏移到写入文件名偏移的位置
        writeAsciiToFile(foutwdk, "i", filenameOffset)  # 写入文件名偏移
        writeAsciiToFile(foutwdk, "i", (filenameOffset + 256))  # 写入新格式偏移
        foutwdk.seek(filenameOffset, 0)  # 偏移到文件名位置
        if py_version.startswith("3"):
            foutwdk.write(filename.encode())  # 写入文件名
        elif py_version.startswith("2"):
            foutwdk.write(filename)
        foutwdk.seek(filenameOffset + 256, 0)  # 偏移到新格式标志位置
        writeAsciiToFile(foutwdk, "i", g_newWdkFlag)  # 写入新格式标志


# 将目录下所有源文件进行打点
def createWdkFileFromDir(dirPath):
    for root, dirs, files in os.walk(dirPath, followlinks=True):
        for name in files:
            m = re.search("(\w.+\.[cC|hH]$)", name)
            if m is not None:
                filepath = os.path.join(root, name)
                createSingleWdkFile(filepath, dirPath)


# #########################################################################################################################
# 整合目录内所有wdk文件生成Total.wdk文件
# 头部部分（64个字节）：文件个数（2个字节）+FileID（4个字节）+位置偏移64（4个字节）+文件大小（4个字节）+文件名偏移（4个字节）
# 内容部分：【FileID（4个字节）+LastTime（8个字节）+打印行数（4个字节）】+打印内容（文件号，行号，内容）
#
# 单个wdk文件至15k偏移位置开始写入
# 偏移80字节 16的整数倍   依次记录单个文件的文件名（256字节*文件个数）
# #########################################################################################################################
def createTotalWdkFile(mergeDirecPath, totalwdkName="Total"):
    wdkOffset = 15 * 1024  # 日志内容偏移15K
    fileCount = 0  # 合并wdk总数
    wdkfileList = []  # wdk文件信息列表【文件号，文件名，单wdk内容】

    # 遍历文件夹中所有wdk文件
    filelist = os.listdir(mergeDirecPath)
    for filename in filelist:
        m = re.search("(\w)(\.wdk)", filename)
        if m is not None:
            filepath = os.path.join(mergeDirecPath, filename)
            # 判断文件大小，默认文件最小为64个字节
            filelenth = os.path.getsize(filepath)
            if filelenth < 64:
                print("file %s is empty" % filepath)
                continue
            # 打开当前轮替wdk文件
            with open(filepath, 'rb') as fSingleWdk:

                if fSingleWdk is None:
                    print("open wdk file %s failed" % filepath)
                # 读取此wdk文件个数,计入总数
                fileNum = (list(struct.unpack('h', fSingleWdk.read(2))))[0]
                fileCount += fileNum
                # 获取文件名位置偏移
                fSingleWdk.seek(2 + fileNum * 12, 0)
                filenameOffset = (list(struct.unpack('i', fSingleWdk.read(4))))[0]

                # 遍历当前文件的wdk日志
                for i in range(fileNum):
                    fSingleWdk.seek(2 + i * 12, 0)
                    fileID, logoffset, loglenth = struct.unpack('iii', fSingleWdk.read(12))
                    fSingleWdk.seek(logoffset, 0)
                    logcmt = fSingleWdk.read(loglenth)
                    debug_print("logcmt lenth = %d" % len(logcmt))

                    fSingleWdk.seek(filenameOffset + i * 256, 0)
                    filename = fSingleWdk.read(256)
                    # 判断当前wdk内容是否已在列表中存在
                    for j in range(len(wdkfileList)):
                        if fileID == wdkfileList[j][0]:
                            print("merge warning:file repeat!")
                            print("New File:FileID=%d,FileName=%s" % (fileID, filename))
                            print("Old File:FileID=%d,FileName=%s" % (wdkfileList[j][0], wdkfileList[j][1]))
                            continue
                    debug_print("filename = %s" % filename)
                    wdkfileList.append([fileID, filename, logcmt])

    with open(os.path.join(mergeDirecPath, totalwdkName + ".wdk"), 'wb') as fout_totalwdk:
        writeAsciiToFile(fout_totalwdk, "h", fileCount)
        debug_print("filecount=%d" % fileCount)
        for i in range(fileCount):
            fout_totalwdk.seek(2 + i * 12, 0)
            loglenth = len(wdkfileList[i][2])
            wdkdata = struct.pack('iii', wdkfileList[i][0], wdkOffset, loglenth)
            fout_totalwdk.write(wdkdata)
            fout_totalwdk.seek(wdkOffset, 0)
            fout_totalwdk.write(wdkfileList[i][2])
            wdkOffset += loglenth

        wdkOffset += 80
        ulFileNameOffset = (wdkOffset // 16) * 16
        fout_totalwdk.seek(2 + 12 * fileCount, 0)
        writeAsciiToFile(fout_totalwdk, "i", ulFileNameOffset)  # 写入文件名偏移
        for i in range(fileCount):
            fout_totalwdk.seek(ulFileNameOffset + i * 256, 0)
            fout_totalwdk.write(wdkfileList[i][1])
    print("merge wdk succ!")


# 保存FILD_ID
def saveFileIdDict(line):
    global g_fileIdDict
    global g_lastFileIdNum
    fileIdStr = ''
    fileIdNum = 0

    m2 = re.search("[^A-Za-z0-9_=, ]", line.strip())
    if m2 is None:
        m1 = re.search("^(\S*).+=\s+(\d*)", line.strip())  # XXXXXXX = dddd,
        if m1 is not None:
            fileIdStr = m1.groups()[0]  # 获取file id字符串
            fileIdNum = m1.groups()[1]
            # debug_print(fileIdNum)
            g_fileIdDict[fileIdStr] = int(fileIdNum)
            g_lastFileIdNum = int(fileIdNum)
        elif "" != line.strip():
            fileIdStr = line.strip(" ,")
            g_fileIdDict[fileIdStr] = g_lastFileIdNum
            g_lastFileIdNum += 1
    else:
        # print('SaveFileIdDict search file id failed.')
        return -1

    return 0


# 获取FILE_ID枚举值
def getFileIdEnum(hFilePath):
    m = re.search('\w+\.(h|txt)$', hFilePath)
    if m is None:
        print('Wrong type file, please input head file path.')
        return -1
    if os.path.exists(hFilePath):
        with open(hFilePath, 'r', encoding='gbk') as fd:
            if not fd:
                print('Open head file [%s] fail.' % (hFilePath))
                return -1
            lines = fd.readlines()  # 读取全部内容，快速但比较耗内存
    else:
        print('Head file [%s] did not exist.' % (hFilePath))
        return -1

    fsmStatus = 0

    for line in lines:  # 遍历每一行，查获
        if 0 == fsmStatus:
            m = re.search("^typedef enum {", line.strip())
            if m is not None:
                fsmStatus = 1
        elif 1 == fsmStatus:
            if "" != line.strip():
                m = re.search("^}", line.strip())
                if m is not None:
                    fsmStatus = 2  # 结束
                else:
                    saveFileIdDict(line.strip())
        elif 2 == fsmStatus:
            break

    return 0


# 打印fileid的映射字典
def printFileIdDict():
    print("start to print FileIdDict::")
    for key in g_fileIdDict:
        print("key:%s value:%d" % (key, g_fileIdDict[key]))


##########################################################################################################################
WDK_HEAD_LEN = 2
LOG_LINE_CNT_VAL_LEN = 4
LOG_NUM_VAL_LEN = 4
LOG_STR_LEN_VAL_LEN = 4
LOG_TIME_VAL_LEN = 8
FILE_ID_VAL_LEN = 4
WDK_OFFSET_VAL_LEN = 4
WDK_LEN_VAL_LEN = 4
FILE_NAME_MAX_LEN = 256
WDK_INFO_LEN = FILE_ID_VAL_LEN + WDK_OFFSET_VAL_LEN + WDK_LEN_VAL_LEN  # 共12字节


# 将二进制的wdk文件解析成txt格式可读文件
def parseWdkFile(wdkFilePath):
    if os.path.exists(wdkFilePath):
        with open(wdkFilePath, 'rb') as wdkFilefd:
            if not wdkFilefd:
                print('parseWdkFile::open wdk file [%s] fail.' % (wdkFilePath))
                return -1

            # 创建txt文件
            pos = wdkFilePath.rfind('.')
            # debugPrint(pos)
            wdkTextFilePath = wdkFilePath[0:pos] + ".txt"
            with open(wdkTextFilePath, 'w') as wdkTextFilefd:
                if not os.path.exists(wdkTextFilePath):
                    print('parseWdkFile::create wdk text file [%s] fail.' % (wdkTextFilePath))
                    return -1
                # 获取WdkFileNumbers
                fileNum = struct.unpack("h", wdkFilefd.read(WDK_HEAD_LEN))[0]
                # debugPrint(fileNum)
                wdkTextFilefd.write(
                    '------------------------------------ Total File Num is %-6d -----------------------------------\n\n' % (
                        fileNum))
                # 获取文件名偏移值
                wdkFilefd.seek(WDK_HEAD_LEN + (fileNum * WDK_INFO_LEN))
                fileNameOffset = struct.unpack("i", wdkFilefd.read(WDK_OFFSET_VAL_LEN))[0]
                # print("fileNameOffset", fileNameOffset)
                # debugPrint(fileNameOffset)
                for i in range(0, fileNum):
                    # 获取fileID和wdkOffset
                    wdkFilefd.seek(WDK_HEAD_LEN + (WDK_INFO_LEN * i))
                    fileID, wdkOffset = struct.unpack("ii", wdkFilefd.read(FILE_ID_VAL_LEN + WDK_OFFSET_VAL_LEN))
                    # 获取fileName
                    wdkFilefd.seek(fileNameOffset + (FILE_NAME_MAX_LEN * i))
                    fileNameStr = wdkFilefd.read(FILE_NAME_MAX_LEN).strip("\0")
                    wdkTextFilefd.write(
                        '[FileNo.%.6d]          [FileID:%6d]          [FileName: %s]\n' % (i + 1, fileID, fileNameStr))
                    wdkTextFilefd.write(
                        '-------------------------------------------------------------------------------------------------\n')
                    # 获取单WDK的log总数
                    wdkFilefd.seek(wdkOffset + FILE_ID_VAL_LEN + LOG_TIME_VAL_LEN)
                    logNum = struct.unpack("i", wdkFilefd.read(LOG_NUM_VAL_LEN))[0]
                    # 循环获取每条log的具体打印内容
                    singleWdkReadOffset = wdkOffset + FILE_ID_VAL_LEN + LOG_TIME_VAL_LEN + LOG_NUM_VAL_LEN
                    # print("log_num", logNum)
                    for j in range(0, logNum):
                        # 获取单log的字符串长度
                        wdkFilefd.seek(singleWdkReadOffset + LOG_LINE_CNT_VAL_LEN)
                        temp = wdkFilefd.read(LOG_STR_LEN_VAL_LEN)
                        # print(temp)
                        logStrLen = struct.unpack("i", temp)[0]
                        # print(logStrLen)
                        # 获取单log的字符串
                        logStr = wdkFilefd.read(logStrLen)
                        singleWdkReadOffset += (LOG_LINE_CNT_VAL_LEN + LOG_STR_LEN_VAL_LEN + logStrLen)
                        wdkTextFilefd.write(logStr.strip("\0") + "\n")
                    wdkTextFilefd.write(
                        '-------------------------------------------------------------------------------------------------\n\n')
        return 0
    else:
        print('parseWdkFile::wdk file %s did not exist.' % (wdkFilePath))
        return -1

    return 0


# 文件换行符处理
def modify_newline_on_linux(srcfile, destfile):
    with open(srcfile, 'rb') as f:
        with open(destfile, 'wb') as f2:
            while 1:
                cS = f.read(1)
                if len(cS) > 0:
                    if cS == "\r":
                        f2.write(cS)
                        cS = f.read(1)
                        if cS != "\n":
                            f2.write("\n")
                            f2.write(cS)
                        else:
                            f2.write(cS)
                    else:
                        f2.write(cS)
                else:
                    break


# main
if __name__ == "__main__":
    returnFlag = 0
    single_output_dir = os.environ.get('HSO_WDK_DIR', '')

    if 3 > len(sys.argv) or 5 < len(sys.argv):
        print("PT tool arg is error!")
        help()

    if 3 == len(sys.argv):
        argv1 = sys.argv[1]
        argv2 = sys.argv[2]

        # parse wdk功能分支
        m1 = re.match("[p|P][a|A][r|R][s|S][e|E]", argv1)
        m2 = re.match("\S+\.wdk$", argv2)
        if m1 is not None and m2 is not None:
            print('Start parsing wdk file %s' % (argv2))
            if 0 == parseWdkFile(argv2):
                print('wdk Parse successful.')
            else:
                print('wdk Parse fail.')
            returnFlag = 1

        # merge Total.wdk功能分支
        m1 = re.match("[m|M][e|E][r|R][g|G][e|E]", argv1)
        if m1 is not None:
            createTotalWdkFile(argv2, "Total")
            returnFlag = 1

        # 源文件打点功能分支
        # m1 = re.match("[\w]+.[cC]", argv1)
        # m2 = re.match("[\w]+.[hH]", argv2)
        # 单源文件打点
        if os.path.isfile(argv1) and os.path.isfile(argv2):
            m = re.search("(\w.+\.(h|H|txt)$)", argv2)
            if m is not None:
                if 0 == getFileIdEnum(argv2):
                    createSingleWdkFile(argv1, single_output_dir)
                    returnFlag = 1
        # 目录下所有源文件打点
        elif os.path.isdir(argv1) and os.path.isfile(argv2):
            m = re.search("(\w.+\.(h|H|txt)$)", argv2)
            if m is not None:
                if 0 == getFileIdEnum(argv2):
                    createWdkFileFromDir(argv1)
                    returnFlag = 1

        if 0 == returnFlag:
            print('Please input right parameters.')

    elif 4 == len(sys.argv):
        argv1 = sys.argv[1]
        argv2 = sys.argv[2]
        argv3 = sys.argv[3]
        print(argv1, argv2, argv3)
        print(os.path.isdir(argv2))
        m = re.match("[m|M][e|E][r|R][g|G][e|E]", argv1)
        if m is not None and os.path.isdir(argv2):
            if ("" == argv3):
                argv3 = "Total"
            createTotalWdkFile(argv2, argv3)
            print('Merge success')
        else:
            print('Please input right parameters.')
