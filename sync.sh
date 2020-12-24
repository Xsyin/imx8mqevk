#!/bin/sh

#sudo apt-get install gawk wget git-core diffstat unzip texinfo gcc-multilib \
#build-essential chrpath socat libsdl1.2-dev

git clone https://source.codeaurora.org/external/imx/uboot-imx -b imx_v2019.04_4.19.35_1.1.0
git clone https://source.codeaurora.org/external/imx/imx-atf -b imx_4.19.35_1.1.0
git clone https://source.codeaurora.org/external/imx/imx-optee-os -b imx_4.19.35_1.1.0
git clone https://source.codeaurora.org/external/imx/linux-imx -b imx_4.19.35_1.1.0
git clone https://source.codeaurora.org/external/imx/imx-mkimage -b imx_4.19.35_1.1.0
git clone https://source.codeaurora.org/external/imx/imx-optee-client -b imx_4.19.35_1.1.0
git clone https://source.codeaurora.org/external/imx/imx-optee-test -b imx_4.19.35_1.1.0
git clone https://github.com/linaro-swg/optee_examples -b 3.2.0

wget https://www.nxp.com/lgfiles/NMG/MAD/YOCTO/firmware-imx-8.5.bin
sudo chmod +x firmware-imx-8.5.bin
./firmware-imx-8.5.bin --auto-accept
