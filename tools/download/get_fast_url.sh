#!/bin/bash

#./t.sh "https://163.com" "https://baidusdfdsf.com" | cut -d',' -f1

# https://ai.b-bug.org/k230/downloads/dl
# https://download.kendryte.com/k230/downloads/dl

URL1="${1:-https://ai.b-bug.org/k230/downloads/dl}"
URL2="${2:-https://download.kendryte.com/k230/downloads/dl}"

test_url() {
    url="$1"
    sum=0
    success_count=0
    count=1

    for i in $(seq 1 $count); do
        response=$(curl -L -o /dev/null -s -w "%{http_code}|%{time_starttransfer}" \
                       -A "Mozilla/5.0" --connect-timeout 5 --max-time 10 "$url" 2>/dev/null)
        http_code="${response%|*}"
        time_val="${response#*|}"

        # 只计算HTTP 200-299 的成功请求
        if [ "$http_code" -ge 200 ] 2>/dev/null && [ "$http_code" -lt 300 ] 2>/dev/null; then
            sum=$(echo "$sum + $time_val" | bc -l)
            success_count=$((success_count + 1))
        fi
    done

    if [ "$success_count" -gt 0 ]; then
        avg=$(echo "scale=3; $sum / $success_count" | bc -l)
        echo "$avg|$success_count"
    else
        echo "failed|0"
    fi
}







result1=$(test_url "$URL1")
LAT1=$(echo "$result1" | cut -d'|' -f1)
count1=$(echo "$result1" | cut -d'|' -f2)

if [ "$LAT1" = "failed" ]; then
    echo "${URL2},${URL1} failed"
    exit 0
fi


result2=$(test_url "$URL2")
LAT2=$(echo "$result2" | cut -d'|' -f1)
count2=$(echo "$result2" | cut -d'|' -f2)

if [ "$LAT2" = "failed" ]; then
    echo "${URL1}, ${URL2} failed"
    exit 0
fi



RESULT=$(echo "$LAT1 < $LAT2" | bc -l)
if [ "$RESULT" -eq 1 ]; then
    echo "${URL1},${count1},${LAT1},${count2},${LAT2}"
else
    echo "${URL2},${count1},${LAT1},${count2},${LAT2}"
fi
