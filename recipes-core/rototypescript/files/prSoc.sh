#!/bin/bash

file_soc="/home/root/update/soc.bin"

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

if [ -f "$file_soc" ] 
then
	rmmod spi-fsl-qspi
	echo 0     > $MC_BOOT_SOC_GPIO/value
	echo 1     > $MC_QSPI_FPGA_EN_GPIO/value
	echo 0     > $MC_QSPI_SOC_EN_GPIO/value
	modprobe spi-fsl-qspi
	echo "update boot system.."
	flash_erase /dev/mtd0 0 0
	flashcp -v $file_soc /dev/mtd0
	rmmod spi-fsl-qspi
	echo 1     > $MC_QSPI_FPGA_EN_GPIO/value
	echo 1     > $MC_QSPI_SOC_EN_GPIO/value
	echo 1     > $MC_BOOT_SOC_GPIO/value
fi


