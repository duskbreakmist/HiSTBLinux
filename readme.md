# 编译
## 安装必要包
``` sh
sudo apt-get install gcc make gettext bison flex bc zlib1g-dev libncurses5-dev lzma bzip2 libbz2-dev
```
## 编译全部
``` sh
在项目根目录
./2make_build.sh
```

## 只编译kernel

``` sh
在项目根目录
./1make_linux.sh 
```

# 配置kernel
``` sh
cd ./source/kernel/linux-4.4.y/
./1start_menuconfig.sh 
进行配置后

./3start_make_cp.sh 
```
