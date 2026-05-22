#!/bin/sh
set -e
# PWM test script
# Usage: pwm-test.sh [chip] [channel] [frequency] [duty]
#  Example: pwm-test.sh 0 2 1000 50  - Output 1kHz 50% duty on pwmchip0 channel 2

usage() {
    echo "K230 PWM Test Script"
    echo "Usage: $0 [chip] [channel] [frequency] [duty]"
    echo ""
    echo "Arguments:"
    echo "  chip     - PWM chip number (0 or 3)"
    echo "  channel  - PWM channel number (0, 1, 2)"
    echo "  frequency- Frequency in Hz, default 1000"
    echo "  duty     - Duty cycle (0-100), can be decimal, default 50"
    echo ""
    echo "Examples:"
    echo "  $0           # Default: pwmchip0 channel 0, 1kHz, 50%"
    echo "  $0 0 2 1000 50   # pwmchip0 channel 2, 1kHz, 50%"
    echo "  $0 3 0 10000 25.5  # pwmchip3 channel 0, 10kHz, 25.5%"
    exit 1
}

# Check help argument
case "$1" in
    -h|--help|help)
        usage
        ;;
esac

CHIP=${1:-0}
CHANNEL=${2:-2}
FREQUENCY=${3:-1000}  # Hz
DUTY=${4:-50}         # Percentage

PWM_PATH="/sys/class/pwm/pwmchip${CHIP}/pwm${CHANNEL}"

# Export PWM - only if PWM path exists
if [ -d "${PWM_PATH}" ]; then
    echo $CHANNEL > /sys/class/pwm/pwmchip${CHIP}/unexport
fi
echo $CHANNEL > /sys/class/pwm/pwmchip${CHIP}/export

sleep 0.5

# Configure parameters
# Calculate period in ns
PERIOD_NS=$(awk -v freq="$FREQUENCY" 'BEGIN { printf "%.0f", 1000000000 / freq }')

# Calculate duty cycle in ns with decimal support
DUTY_NS=$(awk -v period="$PERIOD_NS" -v duty="$DUTY" 'BEGIN { printf "%.0f", period * duty / 100 }')

echo 1 > ${PWM_PATH}/enable
echo "inversed" > ${PWM_PATH}/polarity

echo ${PERIOD_NS} > ${PWM_PATH}/period
echo ${DUTY_NS} > ${PWM_PATH}/duty_cycle


echo "PWM ${CHIP}:${CHANNEL} configured: ${FREQUENCY}Hz, ${DUTY}% duty"

# Show current status
echo "--- Current Status ---"
cat ${PWM_PATH}/period
cat ${PWM_PATH}/duty_cycle
cat ${PWM_PATH}/polarity
cat ${PWM_PATH}/enable
