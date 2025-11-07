#在make build后，将sample编译出的加载到rootbox文件夹下

SRC_sample="/home/ray/workplace/git_download/HiSTB_origin/HiSTBLinux/out/hi3798mv100/hi3798mv100_out/obj/sample"
DST="/home/ray/workplace/git_download/HiSTB_origin/HiSTBLinux/out/hi3798mv100/hi3798mv100_out/rootbox/sample"

copy_tree_safe_ab() {
    local SRC="$1"
    local DST="$2"

    SRC="$(realpath "$SRC")"
    [[ "${SRC}" != */ ]] && SRC="${SRC}/"

    if [ ! -d "$SRC" ]; then
        echo "⚠️ 源目录不存在: $SRC"
        return
    fi

    echo "📁 从 $SRC 复制到 $DST （排除 .o 文件）"

    # 创建目录结构
    find "$SRC" -type d | while read -r d; do
        mkdir -p "$DST/${d#$SRC/}"
    done

    # 复制文件（排除 .o 文件，不覆盖已有的）
    find "$SRC" \( -type f -o -type l \) ! -name '*.o' | while read -r f; do
        local rel="${f#$SRC/}"
        local target="$DST/$rel"
        if [ -e "$target" ]; then
            echo "⚠️ 已存在: $rel （跳过）"
        else
            cp -a "$f" "$target"
            echo "✅ 新增: $rel"
        fi
    done
}

copy_tree_safe_ab "$SRC_sample" "$DST"
