# -*- coding: utf-8 -*-
import os
import getopt
import sys
import codecs
# hash需要调用的库
import hashlib, binascii
import struct


# 对固件进行hash256
def hash_256(message):
    """
    当不需要对固件进行加密运算时，脚本会计算固件明文的hash值
    :param message: 固件明文
    :return: 固件明文的hash
    """
    mesg_hash = hashlib.sha256(message).digest()
    #print('the length of mesg_hash: ', len(mesg_hash))
    print('mesg_hash: ', binascii.hexlify(mesg_hash))
    return mesg_hash


# 没有经过加密和签名的固件头格式
def hash_format(patch_otp, hash_data):
    """
    根据bootrom的固件头格式，生成没有经过加密和签名的固件头格式
    :param patch_otp: 生成的新固件
    :param hash_data: 固件明文的hash值
    :return: 无
    """
    # 写hash数据，32字节
    patch_otp.write(hash_data)
    # 保留（516-32）字节
    reverse_value = bytes(516-32)
    #print(len(reverse_value))
    patch_otp.write(reverse_value)


if __name__ == "__main__":
    inputfile = ''
    outputfile = ''
    try:
        opts, args = getopt.getopt(sys.argv[1:], 'hi:o:asn', ['ifile=', 'ofile=', 'aes', 'sm4', 'non-encryption'])
    except getopt.GetoptError:
        print('rsa+noaes.py -i <inputfile> -o <outputfile> -a <aes> -s <sm4> -n <non-encryption>')
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            print('rsa+noaes.py -i <inputfile> -o <outputfile> -a(aes) -s(sm4) -n(non-encryption)')
            sys.exit()
        elif opt in ('-i', '--ifile'):
            inputfile = arg
        elif opt in ('-o', '--ofile'):
            outputfile = arg
    # 在firmware中添加版本号version
    with open(inputfile, 'rb+') as f:
        origin_content = f.read()
        f.seek(0, 0)
        # write version
        version = b'\x00\x00\x00\x00'
        f.write(version + origin_content)
        f.close()

    input = open(inputfile, 'rb')
    patch_otp = open(outputfile, 'wb')
    input_data = input.read()
    """
    # file length
    file_len = len(input_data)
    data_len = file_len
    """
    # construct file header
    # write Magic
    magic = b'\x4b\x32\x33\x30'
    print('the magic is: ', magic)
    patch_otp.write(magic)
    """
    # write length
    length = struct.pack('I', data_len)
    print('the length of firmware: ', data_len)
    patch_otp.write(length)
    """
    # 全局变量
    message = input_data
    # 判断生成哪种形式的固件头
    for opt, arg in opts:
        if opt in ('-a', '--aes'):
            print('----- AES-GCM + RSA-2048 ----- not support')

        elif opt in ('-s', '--sm4'):
            print('----- SM4-CBC + SM2 ----- not support')

        elif opt in ('-n', '--non-encryption'):
            print('----- NO ENCRYPTION + HASH-256 -----')
            # 写长度: （version+固件明文）
            data_len = len(input_data)
            data_len_byte = data_len.to_bytes(4, byteorder=sys.byteorder, signed=True)  # int convert 4 bytes
            patch_otp.write(data_len_byte)
            # write encryption type
            encrypto_type = 0
            encrypto_type_b = encrypto_type.to_bytes(4, byteorder=sys.byteorder, signed=True)  # int convert bytes
            print('the encryption type: ', encrypto_type)
            patch_otp.write(encrypto_type_b)
            # 对明文做hash256
            hash_data = hash_256(input_data)
            # 写hash头
            hash_format(patch_otp, hash_data)

    # write firmware
    patch_otp.write(message)
    patch_otp.close()
    input.close()
