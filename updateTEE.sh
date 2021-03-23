#!/bin/bash

CURDIR=`pwd`
MOUNTDIR=/media/yin/rootfs

cd ${CURDIR}/imx-optee-client/out/export
cp bin/* ${MOUNTDIR}/usr/bin
cp include/* ${MOUNTDIR}/usr/include/linux
cp lib/* ${MOUNTDIR}/usr/lib

#mkdir ${CURDIR}/lib/optee_armtz
cd ${CURDIR}/optee_examples/out
cp ca/* ${MOUNTDIR}/usr/bin
if [ ! -d "${MOUNTDIR}/lib/optee_armtz" ]; then
  mkdir ${MOUNTDIR}/lib/optee_armtz
fi
cp ta/* ${MOUNTDIR}/lib/optee_armtz

cd ${CURDIR}/imx-optee-test/out
cp xtest/xtest ${MOUNTDIR}/usr/bin
find ta -name \*.ta | xargs -i cp -a {} ${MOUNTDIR}/lib/optee_armtz 


