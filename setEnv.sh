CC_PREFIX=/opt/toolchain-loongarch64-linux-gnu-gcc8-host-x86_64-2022-07-18
export PATH=$CC_PREFIX/bin:$PATH
export LD_LIBRARY_PATH=$CC_PREFIX/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$CC_PREFIX/loongarch64-linux-gnu/lib64:$LD_LIBRARY_PATH
export ARCH=loongarch
export CROSS_COMPILE=loongarch64-linux-gnu-
export MY_KDIR=/home/$(whoami)/loongarch/kernel