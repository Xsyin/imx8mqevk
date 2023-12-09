# imx8mqevk docker 安装

> imx8mqevk 环境构建见[前一篇博客](https://blog.csdn.net/xsyin/article/details/111932935)

## yocto环境

```
$ repo init -u https://source.codeaurora.org/external/imx/imx-manifest -b imx-linux-warrior -m imx-4.19.35-1.1.2.xml
$ repo sync
$ EULA=1 MACHINE=imx8mqevk DISTRO=fsl-imx-xwayland source ./fsl-setup-release.sh -b build

```

## docker

1. 添加virtualization层

```
$ cd sources
$ git clone http://git.yoctoproject.org/cgit/cgit.cgi/meta-virtualization -b warrior
```

2. 修改构建配置文件 

```
// build/bblayer.conf最后添加
BBLAYERS += "${BSPDIR}/sources/meta-virtualization"

//  build/local.conf 添加
IMAGE_INSTALL_append = "  docker docker-contrib connman connman-client"
DISTRO_FEATURES_append = " systemd"
DISTRO_FEATURES_append = " aufs"
DISTRO_FEATURES_append = " virtualization"
VIRTUAL-RUNTIME_init_manager = "systemd"
DISTRO_FEATURES_BACKFILL_CONSIDERED = "sysvinit"
VIRTUAL-RUNTIME_initscripts = "systemd-compat-units"
```

## 编译

```
$ bitbake fsl-image-validation-imx
// 遇到下载链接失效警告时，停止编译，手动修改为有效的下载链接
```

## 测试

```
// 修改uuu.auto，烧写
$sudo uuu -d uuu.auto
// imx8 console中执行
$docker version
```

![Selection_003](https://gitee.com/xsyin/pci-bed/raw/master/Selection_003.png)

## 参考文献

[TipsAndTricks/DockerOnImage](https://wiki.yoctoproject.org/wiki/TipsAndTricks/DockerOnImage)

