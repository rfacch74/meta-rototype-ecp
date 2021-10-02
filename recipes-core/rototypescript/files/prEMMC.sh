#!/bin/bash

FILE_BOOT="/home/root/update/imx-boot.bin"
FILE_KERNEL="/home/root/update/Image.bin"
FILE_DTB="/home/root/update/imx8mq-ecp.dtb"
FILE_ROOTFS="/home/root/update/rootfs.tar.bz2"

umount /dev/mmcblk0p1
umount /dev/mmcblk0p2

echo "    Partitioning..."
/home/root/script/mksdcard.sh /dev/mmcblk0
echo "    clear u-boot arg..."
dd if=/dev/zero of=/dev/mmcblk0 bs=1k seek=4096 conv=fsync count=8
echo "    access boot partition 1..."
echo 0 > /sys/block/mmcblk0boot0/force_ro
echo "    write U-Boot to sd card..."
dd if=$FILE_BOOT of=/dev/mmcblk0boot0 bs=1k seek=33
echo "    re-enable read-only access..."
echo 1 > /sys/block/mmcblk0boot0/force_ro
echo "    enable boot partion 1 to boot..."
mmc bootpart enable 1 1 /dev/mmcblk0 
echo "    Waiting for the partition ready..."
while [ ! -e /dev/mmcblk0p1 ]; do sleep 1; echo waiting...; done 
echo "    Formatting rootfs partition..."
mkfs.vfat /dev/mmcblk0p1
mkdir -p /mnt/mmcblk0p1
mount -t vfat /dev/mmcblk0p1 /mnt/mmcblk0p1
echo "    write kernel image to sd card..."
cp $FILE_KERNEL /mnt/mmcblk0p1/Image
echo "    write device tree to sd card..."
cp $FILE_DTB /mnt/mmcblk0p1/imx8mq-ecp.dtb
echo "    Unmounting vfat partition..."
umount /mnt/mmcblk0p1
echo "    Formatting rootfs partition..."
mkfs.ext4 -F -E nodiscard /dev/mmcblk0p2
mkdir -p /mnt/mmcblk0p2
mount -t ext4 /dev/mmcblk0p2 /mnt/mmcblk0p2
echo "    writting rootfs..."
tar -jxvf $FILE_ROOTFS -C /mnt/mmcblk0p2
sync
echo "    Unmounting rootfs partition..."
umount /mnt/mmcblk0p2
echo "    echo Update Rootfs Complete!..."

