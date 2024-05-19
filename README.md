# Q-DAP

[DAP-Link](https://github.com/ARM-software/CMSIS-DAP) upper by [Qt](http://qt.io)

## UI Preview

![main_window](./doc/assets/main_window.png)

![menu_target_chip](./doc/assets/menu_target_chip.png)

![enum_device_list](./doc/assets/enum_device_list.png)

## Build

```bash
git clone --recursive https://github.com/ma6254/QDAP.git
```

1. Download and install QT: [qt-unified-windows-x64-online.exe](https://qtproject.mirror.liquidtelecom.com/official_releases/online_installers/qt-unified-windows-x64-online.exe)
2. 文件 -> 打开文件或项目 -> CMakeLists.txt
3. 等待工程加载完成
4. 点击`运行(Ctrl+R)`

## Reference

- <https://github.com/libusb/hidapi>
- <https://github.com/openocd-org/openocd>
- <https://arm-software.github.io/CMSIS_5/DAP/html/index.html>
- <https://github.com/x893/CMSIS-DAP>
- <https://gitee.com/synwit-co-ltd/SWMProg>
- <https://gitee.com/synwit-co-ltd/DAPProg>
- <https://github.com/tthe207/DP-SW>
- <https://www.keil.arm.com/devices/>
- <https://en.wikipedia.org/wiki/Executable_and_Linkable_Format>

[glic_elf.h](./src/glibc_elf.h) from <https://github.com/lattera/glibc/blob/master/elf/elf.h>

[FlashOS.h](./src/FlashOS.h) from <https://github.com/ARM-software/CMSIS_4/blob/master/Device/_Template_Flash/FlashOS.h>
