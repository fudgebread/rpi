#!/bin/bash
#
#
# This exists in the top level

# Toolchain for rpi v1
. /opt/sdks/rpi/2.3.20170903/environment-setup-arm1176jzfshf-vfp-poky-linux-gnueabi

# This is needed to build kernel modules
export KERNEL_SRC=/opt/sdks/rpi/2.3.20170903/sysroots/arm1176jzfshf-vfp-poky-linux-gnueabi/usr/src/kernel

# Check if the package number exists in the env
if [ -z ${BUILD_NUM+1} ]; then
  # No package number means we need to export it
  if [ -e buildnum.txt ]; then
    export BUILD_NUM=`cat buildnum.txt`
  else
    echo 1 > buildnum.txt
    export BUILD_NUM=1
  fi
  
  # No package number means we need to export it
  if [ -e package.txt ]; then
    export PACKAGE_NAME=`cat package.txt`
  else
    echo "rpidev" > package.txt
    export PACKAGE_NAME=rpidev
  fi
fi
