# imx8mq-evk  OPTEE 全系统构建

## 源码下载

```
git clone https://source.codeaurora.org/external/imx/uboot-imx -b imx_v2019.04_4.19.35_1.0.0
git clone https://source.codeaurora.org/external/imx/imx-atf -b imx_4.19.35_1.0.0
git clone https://source.codeaurora.org/external/imx/imx-optee-os -b imx_4.19.35_1.0.0
git clone https://source.codeaurora.org/external/imx/linux-imx -b imx_4.19.35_1.0.0
git clone https://source.codeaurora.org/external/imx/imx-mkimage -b imx_4.19.35_1.0.0
git clone https://source.codeaurora.org/external/imx/imx-optee-client -b imx_4.19.35_1.0.0
git clone https://source.codeaurora.org/external/imx/imx-optee-test -b imx_4.19.35_1.0.0
git clone git@github.com:linaro-swg/optee_examples.git -b 3.2.0


optee os patch:  gcc.mk

+libgcc$(sm)  	:= $(shell $(CC$(sm)) $(LIBGCC_LOCATE_CFLAGS) $(CFLAGS$(arch-bits-$(sm))) $(comp-cflags$(sm)) \
 			-print-libgcc-file-name 2> /dev/null)
 			
 			
 wget https://www.nxp.com/lgfiles/NMG/MAD/YOCTO/firmware-imx-8.5.bin
 chmod +x firmware-imx-8.5.bin
 ./firmware-imx-8.5.bin
```

## 环境搭建

```
sudo apt install libssl-dev autoconf automake bc bison build-essential ccache cscope curl device-tree-compiler expect flex ftp-upload gdisk iasl libattr1-dev libcap-dev libfdt-dev libftdi-dev libglib2.0-dev libhidapi-dev libncurses5-dev libpixman-1-dev libssl-dev libtool make mtools netcat python-crypto python3-crypto python-pyelftools  python3-pycryptodome python3-pyelftools python-serial python3-serial rsync unzip uuid-dev xdg-utils xterm xz-utils zlib1g-dev
```

## 工具链

使用aarch64-poky-linux-  工具链：

```
# aarch64-poky-linux
mkdir imx-yocto-bsp && cd imx-yocto-bsp
repo init -u https://source.codeaurora.org/external/imx/imx-manifest -b imx-linux-thud -m imx-4.19.35-1.0.0.xml
repo sync
DISTRO=fsl-imx-wayland MACHINE=imx8mqevk source fsl-setup-release.sh -b imx8mqevk_wayland
bitbake core-image-minimal -c populate_sdk
cd ~/imx-yocto-bsp/imx8mqevk_wayland
mkdir sdk
cd ~/imx-yocto-bsp/imx8mqevk_wayland/tmp/deploy/sdk
./fsl-imx-wayland-glibc-x86_64-core-image-minimal-aarch64-toolchain-4.19-thud.sh
# 安装目录： ~/imx-yocto-bsp/imx8mqevk_wayland/sdk
# 使用该工具链时
source ~/imx-yocto-bsp/imx8mqevk_wayland/sdk/environment-setup-aarch64-poky-linux
```

也可使用aarch64-linux-gnu- 工具链

## 编译

```

make all
```

## rootfs制作

两种方式：

### 1. 完全自制

 1.1 制作 boot

```
ls linux-imx/arch/arm64/boot/dts/freescale | grep "fsl-imx8mq-evk.*dtb" | xargs -i cp linux-imx/arch/arm64/boot/dts/freescale/{} ~/imx8mqevk/boot
cp linux-imx/arch/arm64/boot/Image ~/imx8mqevk/boot

```

1.2 制作rootfs:

​     参考 [树莓派3](https://a-delacruz.github.io/ubuntu/rpi3-setup-filesystem.html]) 和 [ROC](http://wiki.t-firefly.com/en/ROC-RK3399-PC/linux_build_ubuntu_rootfs.html)  

1.3 合并制作镜像

```

dd if=/dev/zero of=imx8rootfs.sdcard bs=1M count=3072
losetup --partscan /dev/loop0  imx8rootfs.sdcard

sudo fdisk /dev/loop0
Type the following parameters (each followed by <ENTER>):
p
d [lists the current partitions]
[to delete existing partitions. Repeat this until no unnecessary partitions
are reported by the 'p' command to start fresh.]
n
p
1
16384
147455
p [create a new partition]
[create a primary partition - use for both partitions]
[the first partition]
[starting at offset sector]
[size for the first partition to be used for the boot images]
[to check the partitions]
n
p
2
147456
<enter>
p
w
[starting at offset sector, which leaves enough space for the kernel,
the bootloader and its configuration data]
[using the default value will create a partition that extends to
the last sector of the media]
[to check the partitions]
[this writes the partition table to the media and fdisk exits]


#挂载 /dev/loop0
mount  /dev/loop0p1 /media/yin/boot
mount  /dev/loop0p2 /media/yin/rootfs
#将 ~/imx8mqevk/boot 复制至 /media/yin/bootboot
#将自制的rootfs复制至rootfs
```

### 2. 官方镜像修改

使用[官方镜像](https://www.nxp.com/search?keyword=L4.19.35_1.0.0_MX&start=0&category=software) 

```
#挂载
sudo losetup --partscan /dev/loop0 fsl-image-validation-imx-imx8mqevk.sdcard
sudo mount /dev/loop0p1 /media/yin/boot
sudo mount /dev/loop0p2 /media/yin/rootfs
# 安装自己的内核模块
make linux-rootfs  
# 将自己编译的内核Image dtb替换原有的
```

## 刷写

下载 [uuu 工具](https://github.com/NXPmicro/mfgtools)

修改官方镜像的uuu.auto

```
拨动sw802开关至 D1 OFF    D2  ON
uuu -d uuu.auto
```



## 参考文档

[imx8mq evk get start](https://www.nxp.com/document/guide/get-started-with-the-mcimx8m-evk:GS-MCIMX8M-EVK)

[imx8mq - bootloader编译过程](https://blog.csdn.net/weixin_42264572/article/details/90490362)

[imx8mq 官方文档](https://www.nxp.com/search?keyword=L4.19.35_&start=0&category=documents)

