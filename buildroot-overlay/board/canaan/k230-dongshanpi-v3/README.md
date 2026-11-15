# DongshanPi K230 CanMV V3 Linux configuration

```sh
make dongshanpi_v3_defconfig
make
```

The configuration is `buildroot-overlay/configs/dongshanpi_v3_defconfig`.
It selects `canaan/k230-canmv-dongshanpi-v3` for Linux and
`k230_canmv_dongshanpi_v3_defconfig` for U-Boot. The original DongshanPi
configuration remains available.

Hardware defaults:

- LPDDR3-1866 and 512 MiB memory, using the existing DongshanPi DDR implementation.
- eMMC on SDIO0 with the existing DongshanPi 1.8 V settings. SDIO1 is dedicated
  to onboard RTL8733BS Wi-Fi, using REG_ON GPIO6 and the standard simple MMC
  power sequencer. The existing RTL8733BS package supplies its driver.
- GC2093 on physical CSI2 / VVCAM logical slot0, mode 0, I2C4, reset GPIO21,
  and SoC MCLK1 on GPIO13. I2C aliases retain their hardware bus numbers.
  Other MIPI slots are disabled.
- The existing ST7701 480 x 640 panel sequence/timings, two DSI lanes, reset
  GPIO22 and active-low backlight GPIO25. Applications can use existing DRM
  rotation for landscape output; the RTOS/MicroPython display defaults are
  not Linux APIs.
- The existing FT5306-compatible touch binding at I2C3 address 0x38, interrupt
  GPIO23, reset GPIO24 and a 480 x 640 coordinate range.

The default camera intentionally follows the upstream-supported CSI2 GC2093
configuration. This does not add the RTOS OV13850 four-lane modes to Linux.
No LT6911C, camera/ISP, Wi-Fi or panel driver implementation changes are included.
Only board DTS/configuration, a kernel configuration fragment, the selector note
and this documentation are added or changed.

The toolchain path follows the official SDK convention under `/opt/toolchain`;
select another path with `make menuconfig` if needed. Use the SDK's normal eMMC
flashing workflow. Linux boot, camera/display/touch operation and Wi-Fi
association require hardware validation; RT-Smart test results are not Linux
validation results.
