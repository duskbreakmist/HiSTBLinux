#请执行 source ./1make_linux.sh 
#而不是直接运行此脚本，否则环境变量不会
echo "source ./env.sh并开始编译"
# mkdir -p ./mylogs
source ./env.sh 
FORCE_UNSAFE_CONFIGURE=1 make -j 8 KCFLAGS="-Wno-error" linux 2>&1 | tee ./mylogs/linux-$(date +%Y%m%d-%H%M).log ; echo -e "\a"
echo "编译kernel结束"
