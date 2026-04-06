#!/bin/bash
set -e

# ===== 用户配置 =====
ROOTFS="../debian12_rootfs/workroot"
SDKOUT="./out/hi3798mv100/hi3798mv100_out/rootbox/"
SDKOUTSAMPLE="./out/hi3798mv100/hi3798mv100_out/obj/sample/"


# ===== 检查 =====
[ -d "$ROOTFS" ] || { echo "ROOTFS 不存在"; exit 1; }
[ -d "$SDKOUT" ] || { echo "SDKOUT 不存在"; exit 1; }
[ -d "$SDKOUTSAMPLE" ] || { echo "SDKOUTSAMPLE 不存在"; exit 1; }

echo "ROOTFS: $ROOTFS"

echo "SDKOUT: $SDKOUT"
echo "SDKOUTSAMPLE: $SDKOUTSAMPLE"

# ===== 1. 拷贝 kmod =====
if [ -d "$SDKOUT/kmod" ]; then
    echo "copy kmod..."
    sudo cp -an "$SDKOUT/kmod" "$ROOTFS/"
fi

# ===== 2. 拷贝 sample =====
if [ -d "$SDKOUTSAMPLE" ]; then
    echo "copy sample..."
    sudo cp -an "$SDKOUTSAMPLE" "$ROOTFS/"
fi

# ===== 3. 处理 extern / share / static =====
sudo rsync -av --ignore-existing \
    "$SDKOUT/usr/lib/" \
    "$ROOTFS/usr/lib/"

echo "done."
