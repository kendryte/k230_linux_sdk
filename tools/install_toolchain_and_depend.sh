#!/bin/bash

print_red()
{
    COLOR_NONE="\033[0m"
    RED="\033[1;31;40m"
    BLUE="\033[1;34;40m"
    GREEN="\033[1;32;40m"
    YELLOW="\033[1;33;40m"
    echo -e ${RED}$*${COLOR_NONE}
}

function install_toolcahin()
{
    GCC_INSTALL_DIR=/opt/toolchain
    GCC_FILE=Xuantie-900-gcc-linux-6.6.0-glibc-x86_64-V3.0.2-20250410
    GCC_FILE_MD5=8cefc7e94f760eaecc3620ffb238bf4a


    if $(curl --output /dev/null --silent --head --fail https://ai.b-bug.org/k230/downloads/dl/gcc ) ;then
        DOWN_URI="https://ai.b-bug.org/k230/downloads/dl/gcc"
    else
        DOWN_URI="https://kendryte-download.canaan-creative.com/k230/downloads/dl/gcc"
    fi



    mkdir -p ${GCC_INSTALL_DIR}
    wget  ${DOWN_URI}/${GCC_FILE}.tar.gz  -O  ${GCC_INSTALL_DIR}/${GCC_FILE}.tar.gz

    [ "${GCC_FILE_MD5}" = "$(md5sum ${GCC_INSTALL_DIR}/${GCC_FILE}.tar.gz | cut -d' ' -f1 )" ]  || (print_red " ${GCC_INSTALL_DIR}/${GCC_FILE}.tar.gz error !" ;exit 1)

    mkdir -p /opt/toolchain
    tar -xf   ${GCC_INSTALL_DIR}/${GCC_FILE}.tar.gz  -C  ${GCC_INSTALL_DIR}
    rm -rf ${GCC_INSTALL_DIR}/${GCC_FILE}.tar.gz


}
function install_dependes()
{
    apt-get update;
    apt-get install -y   git sed make binutils build-essential diffutils gcc  g++ bash patch gzip \
        bzip2 perl  tar cpio unzip rsync file  bc findutils wget  libncurses-dev python3  \
        libssl-dev gawk cmake bison flex  bash-completion parted curl  xz-utils;

    #python3-pcpp;
    if [ -f /etc/os-release ]; then
        . /etc/os-release
    else
        echo "unknown os version"
        #exit 1
    fi
    VERSION_ID=${VERSION_ID%%.*}
    if [ "$VERSION_ID" -ge 24 ]; then
        apt-get install -y python3-pcpp;
    else
        python3 -m pip install pcpp;
    fi
}

if [ "$(id -u)" -ne 0 ]; then
    print_red "permission denied,you need root privileges"
    exit 1;
fi
set -ex
install_dependes
install_toolcahin
