# device_soc_hpmicro

## 简介

本仓库为先楫半导体公司的HPM6700系列芯片相关代码仓库，用于存放与SoC芯片相关的SDK及适配代码。
使用同一系列SoC，开发不同的device或board时，可共用该仓库代码进行开发。

### hpm6700功能概述

![hpm6700功能框图](figures/hpm6750_soc.png)

HPM6700/6400 系列 MCU 是来自上海先楫半导体科技有限公司的高性能实时 RISC-V 微控制器，为工业自动化及边缘计算应用提供了极大的算力、高效的控制能力及丰富的多媒体功能。

**性能**

- RISC-V 内核支持双精度浮点运算及强大的 DSP 扩展，主频高达 816 MHz。
- 32KB 高速缓存 (I/D Cache) 和高达 512KB 的零等待指令和数据本地存储器 (ILM / DLM)，加上 1MB 内置SRAM，极大避免了低速外部存储器引发的性能损失。

**外扩存储**

- 2 个串行总线控制器，支持 NOR Flash / HyperFlash / PSRAM / HyperRAM，支持 NOR Flash 在线加密执行，提供扩展性和兼容性极高的程序空间。
- SDRAM 控制器，支持 166MHz 的 16/32 位SDRAM/LPSDR。
- 2 个 SD/eMMC 控制器，可用于同时连接 WiFi 模块及大容量存储。

**多媒体**

- LCD 显示控制器，支持高达 1366×768 60 fps 显示。
- 图形加速器，支持图形缩放、叠加、旋转等硬件加速。
- JPEG 硬件编解码器。
- 双目摄像头，提高 AI 图像识别能力。
- 4 个 8 通道全双工 I2S 和 1 个数字音频输出。
- 多路语音和数字麦克风接口，便于实现高性能的语音识别应用。

**丰富外设**

- 多种通讯接口：2个千兆实时以太网，支持 IEEE1588，2个内置 PHY 的高速 USB，多达4路 CAN/CAN-FD 及丰富的 UART，SPI，I2C 等外设。
- 4 组共 32 路精度达 2.5ns 的 PWM。 
- 3 个 12 位高速 ADC 5MSPS，1 个 16 位高精度 ADC 2MSPS，4 个模拟比较器，多达 28 个模拟输入通道。
- 多达 36 路 32 位定时器，5 个看门狗和 RTC。

**安全**

- 集成 AES-128/256, SHA-1/256 加速引擎和硬件密钥管理器。支持固件软件签名认证、加密启动和加密执行，可防止非法的代码替换、篡改或复制。
- 基于芯片生命周期的安全管理，以及多种攻击的检测，进一步保护敏感信息。
- 内建 Boot ROM，可以通过 USB 或者 UART 对固件安全下载和升级。

**电源系统**
- 集成高效率 DCDC 转换器和 LDO, 支持系统单电源供电，可动态调节输出电压实现性能-功耗平衡，兼顾了电源的灵活性，易用性和效率。
- 多电源域设计，灵活支持各种低功耗模式。

*更多信息可以访问*[*先楫半导体官方网站*](http://www.hpmicro.com/)


## 开发环境

### 推荐采用Windows+Ubuntu环境进行开发：

-   Windows环境用于编写代码、下载程序和烧入固件等，系统要求：Windows 10 64位系统。

-   Linux环境用于代码下载、编译工程和生成固件等，系统要求：Ubuntu 20.04 64位及以上版本。

    - 若不确定所使用的Linux设备的系统及版本，请在bash中运行如下命令查看：

    ```bash
    lsb_release -a
    ```

    - 请在确认Linux系统不低于`Ubuntu 20.04.XX LTS`的情况下执行后续的步骤；否则，请升级或更换合适的Liunx设备。

> 若需要支持在Linux与Windows之间的文件共享以及编辑，请在Linux设备上适当地安装和配置samba、vim等常用软件。

### OpenHarmony开发环境搭建


- [准备开发环境](https://gitee.com/openharmony/docs/blob/master/zh-cn/device-dev/quick-start/quickstart-pkg-prepare.md)
- [安装库和工具集](https://gitee.com/openharmony/docs/blob/master/zh-cn/device-dev/quick-start/quickstart-pkg-install-package.md)

### 编译工具安装

- [gcc工具下载](https://repo.huaweicloud.com/harmonyos/compiler/gcc_riscv32/7.3.0/linux/gcc_riscv32-linux-7.3.0.tar.gz)


**设置环境变量**

说明： 如果直接采用编译好的riscv32 gcc包，请先执行以下命令将压缩包解压到用户的home(也可以是其他目录，注意使用实际的PATH即可)：

```bash
tar -xvf gcc_riscv32-linux-7.3.0.tar.gz -C ~
```

将以下命令拷贝到`.bashrc`文件的最后一行，保存并退出。

```bash
export PATH=~/gcc_riscv32/bin:$PATH
```

执行下面命令使环境变量生效:

```bash
source ~/.bashrc
```
Shell命令行中输入如下命令`riscv32-unknown-elf-gcc -v`，如果能正确显示编译器版本号，表明编译器安装成功。

```bash
Using built-in specs.
COLLECT_GCC=riscv32-unknown-elf-gcc
COLLECT_LTO_WRAPPER=/home/hhp/ohos/tools/gcc_riscv32/bin/../libexec/gcc/riscv32-unknown-elf/7.3.0/lto-wrapper
Target: riscv32-unknown-elf
Configured with: ../riscv-gcc/configure --prefix=/home/yuanwenhong/gcc_compiler_riscv/gcc_riscv32/gcc_riscv32 --target=riscv32-unknown-elf --with-arch=rv32imc --with-abi=ilp32 --disable-__cxa_atexit --disable-libgomp --disable-libmudflap --enable-libssp --disable-libstdcxx-pch --disable-nls --disable-shared --disable-threads --disable-multilib --enable-poison-system-directories --enable-languages=c,c++ --with-gnu-as --with-gnu-ld --with-newlib --with-system-zlib CFLAGS='-fstack-protector-strong -O2 -D_FORTIFY_SOURCE=2 -Wl,-z,relro,-z,now,-z,noexecstack -fPIE' CXXFLAGS='-fstack-protector-strong -O2 -D_FORTIFY_SOURCE=2 -Wl,-z,relro,-z,now,-z,noexecstack -fPIE' LDFLAGS=-Wl,-z,relro,-z,now,-z,noexecstack 'CXXFLAGS_FOR_TARGET=-Os -mcmodel=medlow -Wall -fstack-protector-strong -Wl,-z,relro,-z,now,-z,noexecstack -Wtrampolines -fno-short-enums -fno-short-wchar' 'CFLAGS_FOR_TARGET=-Os -mcmodel=medlow -Wall -fstack-protector-strong -Wl,-z,relro,-z,now,-z,noexecstack -Wtrampolines -fno-short-enums -fno-short-wchar' --with-headers=/home/yuanwenhong/gcc_compiler_riscv/gcc_riscv32/gcc-riscv32/riscv32-unknown-elf/include --with-mpc=/usr/local/mpc-1.1.0 --with-gmp=/usr/local/gmp-6.1.2 --with-mpfr=/usr/local/mpfr-4.0.2
Thread model: single
gcc version 7.3.0 (GCC)
```

### 源码获取

- [获取源码](https://gitee.com/openharmony/docs/blob/master/zh-cn/device-dev/quick-start/quickstart-pkg-sourcecode.md)
- [安装hb工具](https://gitee.com/openharmony/docs/blob/master/zh-cn/device-dev/quick-start/quickstart-pkg-install-tool.md)

**注意**

> 默认下载的是master分支版本，如果想下载其他分支版本请将`-b master`改为需要下载的分支。

**比如下载OpenHarmony-4.0-Release:**

```
repo init -u git@gitee.com:openharmony/manifest.git -b OpenHarmony-4.0-Release --no-repo-verify
```

**hb 安装报错**

```
  WARNING: The scripts alldefconfig, allmodconfig, allnoconfig, allyesconfig, defconfig, genconfig, guiconfig, listnewconfig, menuconfig, oldconfig, olddefconfig, savedefconfig and setconfig are installed in '/home/xxx/.local/bin' which is not on PATH.
  Consider adding this directory to PATH or, if you prefer to suppress this warning, use --no-warn-script-location.
  WARNING: The script hb is installed in '/home/xxx/.local/bin' which is not on PATH.
  Consider adding this directory to PATH or, if you prefer to suppress this warning, use --no-warn-script-location.```
```

解决办法：

1. 将以下命令拷贝到.bashrc文件的最后一行，保存并退出。

```
export PATH=~/.local/bin:$PATH
```

2. 执行如下命令更新环境变量

```
source ~/.bashrc
python3 -m pip uninstall ohos-build   
python3 -m pip install --user build/hb
```


## 工程编译

### 选择目标工程

执行`hb set`选择`hpmicro`下的`hpm6750evk2`:

![hpm6750evk2_hbset](figures/hbset.png)

### 也可以直接通过命令指定

```
hb set -p hpm6750evk2
```

### 编译工程

执行`hb build -f`进行工程编译，编译成功后显示如下内容：

![编译成功](figures/build_success.png)


### 打开xts功能

执行`hb build -f --gn-args="build_xts=true"`命令进行xts编译。

## 镜像烧录

### 下载安装烧录工具(windows)

点击[先楫系列资料](https://pan.baidu.com/s/1RaYHOD7xk7fnotmgLpoAlA?pwd=xk2n)下载`sdk/HPMicro_Manufacturing_Tool_vx.x.x.zip`

下载后解压HPMicro_Manufacturing_Tool_vx.x.x.zip文件到任意目录。

找到`hpm_manufacturing_gui.exe`，双击执行程序：

![程序初始界面](figures/HPMProgrammer_start.png)

### 下载程序到开发板

1. 将开发板的`USB2UART0`接口连接至PC，正确连接后，PC可识别到一路usb串口，hpm6750evk2板载了ch340 usb转串口芯片。

2. 将BOOT拨码开关设置为:

    ```bash
    boot0: 0 
    boot1: 1
    ``` 

    > 拨码开关设置好后，点击开发板的`RESTN`按钮，进行复位，复位后进入到isp模式。

3. 配置hpm_manufacturing_gui的`类型为：UART`并选择选择步骤1中对用的串口，点击`连接`：
![连接开发板](figures/connecting.png)

连接成功：

![连接成功](figures/connected.png)

4. 拷贝编译好的固件程序`out/hpm6750evk2/hpm6750evk2/OHOS_Image.bin`到windows。

5. 选择固件程序，点击`烧写`进行下载:

![固件下载](figures/downloading.png)

6. 设置拨码开关为正常模式

    ```
    boot0: 0 
    boot1: 0 
    ```

7. 打开一个串口终端`MobaXterm`，重启开发板，串口打印启动信息

![启动信息](figures/boot_info.png)

8. hpm_manufacturing_gui更多用法，请参考：`HPMicro_Manufacturing_Tool_vx.x.x\doc\user_manual.html`


### openocd进行调试镜像

1. 下载openocd

```
git clone git@gitee.com:hpmicro/riscv-openocd.git -b riscv-hpmicro
```
2. 安装依赖工具

```
sudo apt install libtool libusb-1.0-0-dev libhidapi-dev libftdi-dev
```

3. 安装

进入源码目录，并执行下面的命令

```
./bootstrap
./configure
make -j16
sudo make install
sudo cp contrib/60-openocd.rules /etc/udev/rules.d/

```

4. 连接烧写和目标板

5. 启动openocd

进入到ohos源码`device/soc/hpmicro/sdk/hpm_sdk/boards/openocd`,执行如下命令：

```
$ openocd -s . -f probes/cmsis_dap.cfg -f soc/hpm6750-single-core.cfg -f boards/hpm6750evk2.cfg
Open On-Chip Debugger 0.11.0+dev (2024-01-08-14:39)
Licensed under GNU GPL v2
For bug reports, read
	http://openocd.org/doc/doxygen/bugs.html
srst_only separate srst_gates_jtag srst_open_drain connect_deassert_srst

Info : Listening on port 6666 for tcl connections
Info : Listening on port 4444 for telnet connections
Info : CMSIS-DAP: SWD supported
Info : CMSIS-DAP: JTAG supported
Info : CMSIS-DAP: Atomic commands supported
Info : CMSIS-DAP: Test domain timer supported
Info : CMSIS-DAP: FW Version = 2.0.0
Info : CMSIS-DAP: Interface Initialised (JTAG)
Info : SWCLK/TCK = 0 SWDIO/TMS = 1 TDI = 0 TDO = 1 nTRST = 0 nRESET = 0
Info : CMSIS-DAP: Interface ready
Info : clock speed 8000 kHz
Info : cmsis-dap JTAG TLR_RESET
Info : cmsis-dap JTAG TLR_RESET
Info : JTAG tap: hpm6750.cpu tap/device found: 0x1000563d (mfg: 0x31e (Andes Technology Corporation), part: 0x0005, ver: 0x1)
Info : [hpm6750.cpu0] datacount=4 progbufsize=8
Info : Examined RISC-V core; found 2 harts
Info :  hart 0: XLEN=32, misa=0x4094112d
[hpm6750.cpu0] Target successfully examined.
Info : starting gdb server for hpm6750.cpu0 on 3333
Info : Listening on port 3333 for gdb connections

```

6. 打开另一个终端,并进入ohos源码目录，启动gdb

```
$ riscv32-unknown-elf-gdb
GNU gdb (GDB) 8.1.50.20180718-git
Copyright (C) 2018 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.
Type "show copying" and "show warranty" for details.
This GDB was configured as "--host=x86_64-pc-linux-gnu --target=riscv32-unknown-elf".
Type "show configuration" for configuration details.
For bug reporting instructions, please see:
<http://www.gnu.org/software/gdb/bugs/>.
Find the GDB manual and other documentation resources online at:
    <http://www.gnu.org/software/gdb/documentation/>.

For help, type "help".
Type "apropos word" to search for commands related to "word".
(gdb)
```

7. 选择需要调试的镜像，并进行加载调试

```
(gdb) file out/hpm6750evk2/hpm6750evk2/OHOS_Image
Reading symbols from out/hpm6750evk2/hpm6750evk2/OHOS_Image...done.
(gdb) target extended-remote :3333
Remote debugging using :3333
warning: Target-supplied registers are not supported by the current architecture
_start () at ../../../device/soc/hpmicro/hpm6700/liteos_m/los_start.S:26
26	    la gp, __global_pointer$
(gdb) load
Loading section .nor_cfg_option, size 0x10 lma 0x80000400
Loading section .boot_header, size 0x90 lma 0x80001000
Loading section .start, size 0x2e lma 0x80003000
Loading section .vectors, size 0x3c0 lma 0x8000302e
Loading section .text, size 0x5876e lma 0x800033ee
Loading section .data, size 0x1158 lma 0x8005bb5c
Start address 0x80003000, load size 367956
Transfer rate: 6 KB/sec, 13141 bytes/write.
(gdb) c

```

## 相关仓库

[vendor_hpmicro](https://gitee.com/openharmony/vendor_hpmicro)

[device_soc_hpmicro](https://gitee.com/openharmony/device_soc_hpmicro)

[device_board_hpmicro](https://gitee.com/openharmony/device_board_hpmicro)

## 联系

如果您在开发过程中有问题，请在仓库[issues](https://gitee.com/openharmony/device_soc_hpmicro/issues)提问。





