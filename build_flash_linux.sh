#!/bin/sh

make linux-imx
cd out
./update_rootfs.sh core-image-minimal-imx8mqevk-20210204131857.rootfs.wic
sudo uuu -d uuu.auto
