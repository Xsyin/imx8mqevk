#!/bin/bash

CURDIR=`pwd`
MOUNTDIR=/media/yin/rootfs

cd ${CURDIR}/imx-optee-client/out/export
sudo cp bin/* ${MOUNTDIR}/usr/bin
sudo cp include/* ${MOUNTDIR}/usr/include/linux
sudo cp lib/* ${MOUNTDIR}/usr/lib

#sudo mkdir ${CURDIR}/lib/optee_armtz
cd ${CURDIR}/optee_examples/out
sudo cp ca/* ${MOUNTDIR}/usr/bin
sudo cp ta/* ${MOUNTDIR}/lib/optee_armtz

cd ${CURDIR}/imx-optee-test/out
sudo cp xtest/xtest ${MOUNTDIR}/usr/bin
find ta -name \*.ta | xargs -i sudo cp -a {} ${MOUNTDIR}/lib/optee_armtz 


