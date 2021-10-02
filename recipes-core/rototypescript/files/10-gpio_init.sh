#!/bin/bash

echo "    GPIO Init"

export MC_BOOT_FPGA_PIN=1
export MC_BOOT_SOC_PIN=3
export MC_QSPI_FPGA_EN_PIN=5
export MC_QSPI_SOC_EN_PIN=15
export MC_SOC_DONE_PIN=6
export MC_FPGA_DONE_PIN=12

export MC_BOOT_FPGA_GPIO="/sys/class/gpio/gpio"$MC_BOOT_FPGA_PIN
export MC_BOOT_SOC_GPIO="/sys/class/gpio/gpio"$MC_BOOT_SOC_PIN
export MC_QSPI_FPGA_EN_GPIO="/sys/class/gpio/gpio"$MC_QSPI_FPGA_EN_PIN
export MC_QSPI_SOC_EN_GPIO="/sys/class/gpio/gpio"$MC_QSPI_SOC_EN_PIN
export MC_SOC_DONE_GPIO="/sys/class/gpio/gpio"$MC_SOC_DONE_PIN
export MC_FPGA_DONE_GPIO="/sys/class/gpio/gpio"$MC_FPGA_DONE_PIN

echo $MC_BOOT_FPGA_PIN     > /sys/class/gpio/export
echo $MC_BOOT_SOC_PIN      > /sys/class/gpio/export
echo $MC_QSPI_FPGA_EN_PIN  > /sys/class/gpio/export
echo $MC_QSPI_SOC_EN_PIN   > /sys/class/gpio/export
echo $MC_SOC_DONE_PIN      > /sys/class/gpio/export
echo $MC_FPGA_DONE_PIN     > /sys/class/gpio/export


echo out   > $MC_BOOT_FPGA_GPIO/direction
echo out   > $MC_BOOT_SOC_GPIO/direction
echo out   > $MC_QSPI_FPGA_EN_GPIO/direction
echo out   > $MC_QSPI_SOC_EN_GPIO/direction
echo in    > $MC_SOC_DONE_GPIO/direction
echo in    > $MC_FPGA_DONE_GPIO/direction

echo 1     > $MC_BOOT_FPGA_GPIO/value
echo 1     > $MC_BOOT_SOC_GPIO/value
echo 1     > $MC_QSPI_FPGA_EN_GPIO/value
echo 1     > $MC_QSPI_SOC_EN_GPIO/value

sleep 0.1

