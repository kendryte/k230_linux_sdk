#!/bin/bash

#./t.sh "https://163.com" "https://baidusdfdsf.com" | cut -d',' -f1

# https://ai.b-bug.org/k230/downloads/dl
# https://download.kendryte.com/k230/downloads/dl

URL1="${1:-https://ai.b-bug.org/k230/downloads/dl}"
URL2="${2:-https://download.kendryte.com/k230/downloads/dl}"

# 用首字节延迟选镜像不靠谱：延迟低不代表吞吐高。改成实测下载 3 MiB 的吞吐（字节/秒）。
# 探测对象固定用 gcc 工具链压缩包（两个镜像站都有，且远大于 3 MiB），用 Range 只取前 3 MiB。
PROBE_PATH="gcc/Xuantie-900-gcc-linux-6.6.0-glibc-x86_64-V3.0.2-20250410.tar.gz"
PROBE_BYTES=$((3 * 1024 * 1024))

test_url() {
    url="$1/$PROBE_PATH"

    response=$(curl -L -o /dev/null -s -w "%{http_code}|%{speed_download}" \
                   -A "Mozilla/5.0" --connect-timeout 5 --max-time 15 \
                   --range "0-$((PROBE_BYTES - 1))" "$url" 2>/dev/null)
    http_code="${response%|*}"
    speed_val="${response#*|}"

    # Range 请求成功是 206；完整 200 也认（服务器不支持 Range 时退化成整档下载）
    if { [ "$http_code" = "206" ] || [ "$http_code" = "200" ]; } 2>/dev/null; then
        echo "$speed_val|1"
    else
        echo "failed|0"
    fi
}







result1=$(test_url "$URL1")
SPD1=$(echo "$result1" | cut -d'|' -f1)
count1=$(echo "$result1" | cut -d'|' -f2)

if [ "$SPD1" = "failed" ]; then
    echo "${URL2},${URL1} failed"
    exit 0
fi


result2=$(test_url "$URL2")
SPD2=$(echo "$result2" | cut -d'|' -f1)
count2=$(echo "$result2" | cut -d'|' -f2)

if [ "$SPD2" = "failed" ]; then
    echo "${URL1}, ${URL2} failed"
    exit 0
fi



# 吞吐（字节/秒）越大越好，跟原来比延迟的方向相反
RESULT=$(echo "$SPD1 > $SPD2" | bc -l)
if [ "$RESULT" -eq 1 ]; then
    echo "${URL1},${count1},${SPD1},${count2},${SPD2}"
else
    echo "${URL2},${count1},${SPD1},${count2},${SPD2}"
fi
