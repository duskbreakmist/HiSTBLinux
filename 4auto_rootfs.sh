#!/bin/bash
# =====================================================
# 自动构建 rootfs 镜像脚本（无须输入）
# 功能：
#   1. 自动计算 rootfs 大小或默认512MB
#   2. 自动命名 rootfs_<时间>.img
#   3. 格式化、拷贝、压缩镜像
# 作者：Ray（自动增强版）
# =====================================================
# 将rootbox打包，make build得到的ext4是安卓稀疏格式

set -e
set -u

# ---- 基本配置 ----
SRC_DIR="/home/ray/workplace/git_download/HiSTB_origin/HiSTBLinux/out/hi3798mv100/hi3798mv100_out/rootbox/"               # 源 rootfs 目录
DATE_TAG=$(date +"%Y%m%d_%H%M")
IMG_NAME="/home/ray/workplace/git_download/HiSTB_origin/HiSTBLinux/out/hi3798mv100/hi3798mv100_out/rootfs_${DATE_TAG}.img"
MNT_DIR="mnt_rootfs"

# ---- 检查目录 ----
if [ ! -d "$SRC_DIR" ]; then
    echo "❌ 源目录不存在: $SRC_DIR"
    exit 1
fi

# ---- 自动计算大小 ----
echo "📏 正在计算 rootfs 大小..."
DIR_SIZE_MB=$(du -sm "$SRC_DIR" | awk '{print $1}')
if [ -z "$DIR_SIZE_MB" ]; then
    DIR_SIZE_MB=512
fi

# 预留额外空间（1.2倍）
IMG_SIZE_MB=$(( DIR_SIZE_MB + 64 ))
# # 最小 400MB
# if [ "$IMG_SIZE_MB" -lt 400 ]; then
#     IMG_SIZE_MB=400
# fi

echo "🧮 源目录约 ${DIR_SIZE_MB}MB，将创建 ${IMG_SIZE_MB}MB 镜像。"

# ---- 创建镜像文件 ----
echo "📦 创建 rootfs 镜像: ${IMG_NAME} (${IMG_SIZE_MB}MB)"
dd if=/dev/zero of=$IMG_NAME bs=1M count=$IMG_SIZE_MB status=none
echo "✅ 镜像文件创建完成。"

# ---- 格式化 ----
echo "🧱 格式化为 ext4..."
mkfs.ext4 -q -L rootfs -O ^has_journal $IMG_NAME

# ---- 挂载 ----
echo "📁 挂载镜像..."
mkdir -p $MNT_DIR
sudo mount $IMG_NAME $MNT_DIR

# ---- 拷贝文件 ----
echo "📂 拷贝 rootfs 文件..."
sudo chown -R root:root "$SRC_DIR"
sudo cp -a ${SRC_DIR}/* $MNT_DIR/

echo "✅ 同步数据..."
sync

# ---- 卸载 ----
echo "🔌 卸载镜像..."
sudo umount $MNT_DIR
rmdir $MNT_DIR

# ---- 检查 & 压缩 ----
echo "🔍 检查文件系统..."
e2fsck -p -f $IMG_NAME >/dev/null || true

echo "📏 压缩镜像到最小大小..."
resize2fs -M $IMG_NAME >/dev/null

# ---- 完成 ----
echo "🎉 完成！"
ls -lh $IMG_NAME
echo "📁 镜像文件名：$IMG_NAME"
