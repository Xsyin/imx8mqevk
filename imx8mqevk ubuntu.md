# imx8mq-evk ubuntu 构建

1. 下载ubuntu 18.04

```
$ mkdir -p ~/rootfs/ubuntu-rootfs
$ cd rootfs
$ wget http://cdimage.ubuntu.com/ubuntu-base/releases/18.04.5/release/ubuntu-base-18.04.5-base-ar64.tar.gz
$ tar vxf ubuntu-base-18.04.5-base-arm64.tar.gz -C ubuntu-rootfs
$ vim ch-mount.sh
```

2. ch-mount.sh内容：

```
#!/bin/bash
#
function mnt() {
        echo "MOUNTING"
        sudo mount -t proc /proc ${2}proc
        sudo mount -t sysfs /sys ${2}sys
        sudo mount -o bind /dev ${2}dev
        sudo mount -o bind /dev/pts ${2}dev/pts
        sudo chroot ${2}
}

function umnt() {
        echo "UNMOUNTING"
        sudo umount ${2}proc
        sudo umount ${2}sys
        sudo umount ${2}dev/pts
        sudo umount ${2}dev
}

if [ "$1" == "-m" ] && [ -n "$2" ] ;
then
        mnt $1 $2
elif [ "$1" == "-u" ] && [ -n "$2" ];
then
        umnt $1 $2
else
        echo ""
        echo "Either 1'st, 2'nd or both parameters were missing"
        echo ""
        echo "1'st parameter can be one of these: -m(mount) OR -u(umount)"
        echo "2'nd parameter is the full path of rootfs directory(with trailing '/')"
        echo ""
        echo "For example: ch-mount -m /media/sdcard/"
        echo ""
        echo 1st parameter : ${1}
        echo 2nd parameter : ${2}
fi
```

```
$ sudo chmod +x ch-mount.sh
$ sudo apt install qemu-user-static
$ cp /usr/bin/qemu-aarch64-static rootfs/usr/bin
$ sudo ./ch-mount.sh -m ubuntu-rootfs/

```

3. qemu环境内

```
# echo nameserver 114.114.114.114 > /etc/resolv.conf
# chmod 777 /tmp
# apt update
# apt install language-pack-en-base sudo ssh net-tools \
network-manager iputils-ping rsyslog \
bash-completion htop resolvconf dialog \
vim nano build-essential alsa-utils git gcc \
less resolvconf autoconf autopoint libtool \
bison flex  libpango1.0-dev libatk1.0-dev kmod pciutils isc-dhcp-server -y

```

4. 添加用户

```
# useradd -s '/bin/bash' -m -G adm,sudo yin
# passwd yin      // 输入ubuntu
# passwd root    //输入ubuntu
# echo 'dev' > /etc/hostname
```

5. DNS配置

```
# dpkg-reconfigure resolvconf
//退出
#exit
// 一定要执行
$ sudo ./ch-mount.sh -u ubuntu-rootfs/
```

6. （可选）从imx-yocto项目中拷贝：（host中操作）

```
$ cp -Pra ~/imx-yocto-4.19/build-docker/tmp/work/aarch64-mx8m-poky-linux/libdrm/2.4.99.imx-r0/image/* ~/rootfs/ubuntu-rootfs
$ cp -Pra ~/imx-yocto-4.19/build-docker/tmp/work/aarch64-mx8m-poky-linux/imx-gpu-viv/1_6.4.0.p1.0-aarch64-r0/image/* ~/rootfs/ubuntu-rootfs
$ cp -Pra ~/imx-yocto-4.19/build-docker/tmp/work/all-poky-linux/firmware-imx/1_8.5-r0/image/* ~/rootfs/ubuntu-rootfs
$ cp -Pra ~/imx-yocto-4.19/build-docker/tmp/work/imx8mqevk-poky-linux/systemd-serialgetty/1.0-r5/image/* ~/rootfs/ubuntu-rootfs

```

7. 挂载imx-yocto项目中的rootfs（如fsl-image-validation-imx-imx8mqevk-20210907060444.rootfs.wic）并替换rootfs为ubuntu(也可替换官方镜像的rootfs，官方镜像下载地址：https://www.nxp.com/search?keyword=L4.19.35_1.0.0_MX8MQ&start=0&category=software)：

```
$ sudo losetup --partscan /dev/loop41 fsl-image-validation-imx-imx8mqevk-20210907060444.rootfs.wic
$ sudo mount /dev/loop41p1 $MOUNTDIR_BOOT
$ sudo mount /dev/loop41p2 $MOUNTDIR_ROOTFS
$ cd $MOUNTDIR_ROOTFS
$ sudo rm -rf *
$ cp -rf ~/rootfs/ubuntu-rootfs/. .
// 卸载
$ sudo umount $MOUNTDIR_ROOTFS
$ sudo umount $MOUNTDIR_BOOT
$ sudo losetup -d /dev/loop41
//烧写
sudo uuu -d uuu.auto
```

8. 正常启动后配置

   8.1 根分区内存显示只有3G

   ```
   # cfdisk /dev/mmcblk0
   //调整分区/dev/mmcblk0p2为15G
   # resize2fs -p /dev/mmcblk0p2
   ```

   8.2 eth0网卡配置

   ```
   // ifconfig看不见eth0执行
   # dhclient eth0
   ```

   8.3 替换内核
   
   ```
   # mount -t vfat /dev/mmcblk0p1 /mnt
   # cp Image /mnt
   ```
   
   8.4 创建虚拟设备
   
   ```
   dd if=/dev/zero of=disk1.img bs=1M count=20
   mkfs.ext3 disk1.img
   ```
   
   



