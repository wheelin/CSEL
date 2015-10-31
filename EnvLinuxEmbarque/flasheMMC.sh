#! /bin/bash

if [ -z $1 ]; then
	echo "Please specify the device to flash. Example : $0 /dev/sdb"
	exit 1
fi
device=$1

sudo umount $device*

echo "initialize first 64Mo"
sudo dd if=/dev/zero of=$device bs=4k count=32768
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
sync

echo
echo "First sector : MSDOS"
sudo parted $device mklabel msdos
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

echo
echo "First partition : rootfs - Start: 64MB, length: 256MB"
sudo parted $device mkpart primary ext4 131072s 655359s
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

echo
echo "Second partition : usrfs - Start: 320MB, length: 256MB"
sudo parted $device mkpart primary ext4 655360s 1179647s
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

echo
echo "Format rootfs and usrfs"
sudo mkfs.ext4 ${device}1 -L rootfs
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
sudo mkfs.ext4 ${device}2 -L usrfs
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
sync

echo
echo "Copying bl1.bin"
sudo dd if=~/workspace/xu3/buildroot/output/images/xu3-bl1.bin of=$device bs=512 seek=1
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

echo
echo "Copying bl2.bin"
sudo dd if=~/workspace/xu3/buildroot/output/images/xu3-bl2.bin of=$device bs=512 seek=31
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

echo
echo "Copying tzsw.bin"
sudo dd if=~/workspace/xu3/buildroot/output/images/xu3-tzsw.bin of=$device bs=512 seek=719
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

echo
echo "Copying u-boot"
sudo dd if=~/workspace/xu3/buildroot/output/images/xu3-u-boot.bin of=$device bs=512 seek=63
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

echo
echo "Copying kernel"
sudo dd if=~/workspace/xu3/buildroot/output/images/xu3-uImage of=$device bs=512 seek=1263
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

echo
echo "Copying flattened device tree"
sudo dd if=~/workspace/xu3/buildroot/output/images/exynos5422-odroidxu3.dtb of=$device bs=512 seek=17647
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

echo
echo "Copying rootfs"
sudo dd if=~/workspace/xu3/buildroot/output/images/xu3-rootfs.ext4 of=$device bs=512 seek=131072
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

