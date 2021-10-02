#!/bin/bash

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

echo "    FPGA Init:"
echo 1     > $MC_QSPI_SOC_EN_GPIO/value

echo "    -> Reset Spartan7 FPGA"
echo 0     > $MC_BOOT_FPGA_GPIO/value
sleep 0.1
echo 1     > $MC_BOOT_FPGA_GPIO/value

echo "    -> Reset Zynq SOC"
echo 0     > $MC_BOOT_SOC_GPIO/value
sleep 0.1
echo 1     > $MC_BOOT_SOC_GPIO/value

counter=0
while [ $(cat $MC_FPGA_DONE_GPIO/value) -eq 0 ]
do
  sleep 0.1
  let counter++
  if [ $counter -eq 20 ]; then
    echo "    -> Spartan7 FPGA Timed Out"
    break
  fi
done

counter=0
while [ $(cat $MC_SOC_DONE_PIN/value) -eq 0 ]
do
  sleep 0.1
  let counter++
  if [ $counter -eq 20 ]; then
    echo "    -> Zynq Timed Out"
    break
  fi
done

