# Loongson-2k500
龙芯2k500开发板驱动开发实例

[龙芯技术交流社区](https://bbs.elecfans.com/group_1650)

## 索引
* [环境搭建](#环境搭建)
  * 环境工具
  * 环境配置
  * 程序编译
* [开发实例](#开发实例)

***

## 环境搭建

### 1. 环境工具

本技术文档默认使用以下环境进行编译：

`Linux内核` linux-5.10-2k500-cbd-src

`交叉编译链` loongarch64-linux-gnu-gcc8-host-x86_64-2022-07-18

请在[龙芯2K500开发板技术资料](https://bbs.elecfans.com/jishu_2310652_1_1.html)中获取工具文件及数据文档。

在任意位置解压Linux内核和交叉编译链的压缩包，后续环境配置会使用到解压出来的同名目录（可凭个人喜好更改，注意后续配置中目录名要相同）。

### 2. 环境配置

在进行内核空间和用户空间的程序编译前，需要配置交叉编译链的环境变量，配置内容如下，有两种配置方式，可自行选择

```
CC_PREFIX=/从根目录到交叉编译链目录的绝对路径/toolchain-loongarch64-linux-gnu-gcc8-host-x86_64-2022-07-18
export PATH=$CC_PREFIX/bin:$PATH
export LD_LIBRARY_PATH=$CC_PREFIX/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$CC_PREFIX/loongarch64-linux-gnu/lib64:$LD_LIBRARY_PATH
export ARCH=loongarch
export CROSS_COMPILE=loongarch64-linux-gnu-
```

**1) 脚本配置（每次编译前均需运行）**

在任意位置创建脚本文件setEnv.sh，写入上面的配置内容后保存退出，赋予可执行权限，执行指令如下：

```
sudo chmod 777 ./setEnv.sh
```

之后每次编译前，使用的终端都需要使用source命令使该配置生效，且该配置仅作用于本终端，更换终端后需再次执行该指令。

```
source ./setEnv.sh
```

**2) 永久配置**

使用如下命令配置用户的终端环境，在文件最上方添加配置内容后保存退出。

```
vim ~/.bashrc
```

该配置会在系统开启时载入，也可以用下面的指令使配置立即生效，配置完成后便可以在任意位置进行编译，而无需配置交叉编译链环境
```
source ~/.bashrc
```

### 3. 程序编译

通常使用Makefile进行编译，对于内核模块代码和用户空间代码的Makefile文件不同，本节将分别进行介绍。

**1) 内核模块Makefile**

内核模块代码编译使用的Makefile文件格式如下：

```
obj-m :=module_name.o

KDIR :=/root/loongarch/linux-5.10-2k500-cbd-src

all:
	make -C $(KDIR) M=$(PWD) modules
clean:
	rm -f *.ko *.o *.mod.o *.mod.c *.symvers
```

obj-m为编译内核模块代码生成的.o文件，需要本目录下存在module_name.c源文件，需要根据源文件名修改obj-m。  
KDIR为Linux内核源码目录，需要改为第一步中解压得到的linux-5.10-2k500-cbd-src目录所在位置。  

**2) 用户空间Makefile**

用户空间编译较为简单，无需配置内核目录，具体Makefile文件内容如下：

```
all: test 
clean: 
	rm -rf test_ch422g        
test: 
	loongarch64-linux-gnu-gcc -O2 test.c -o test
```

如使用多线程编程，可添加编译选项`-lpthread`，其余编译选项请参考交叉编译链相关文档。

***

## 开发实例

1. [aht20 温湿度](./aht20/README.md)

2. [ch422 数码管](./ch422/README.md)

3. [ms1112 ad转换](./ms1112/README.md)

4. [ms5541 da转换](./ms5541/README.md)

5. [my_beep 蜂鸣器](./my_beep/README.md)

6. [my_key 键盘](./my_key/README.md)

7. [sd2068 RTC](./sd2068/README.md)

8. [stk8ba 陀螺仪](./stk8ba/README.md)

9. [IR_drv 红外遥控](./IR_drv/README.md)

10. [aip1944 LED矩阵模块显示](./aip1944/README.md)

11. [motor_ctrl 电机控制](./motor_ctrl/README.md)

12. [rfid 智能卡](./rfid/README.md)

13. [led led灯控制](./led/README.md)
