# Q-DAP



[![License](https://img.shields.io/github/license/ma6254/qdap.svg)](https://raw.githubusercontent.com/ma6254/qdap/master/LICENSE)
[![release_version](https://img.shields.io/github/release/ma6254/qdap.svg)](https://github.com/ma6254/qdap/releases)
[![last-commit](https://img.shields.io/github/last-commit/ma6254/qdap.svg)](https://github.com/ma6254/qdap/commits)

![qt_version](https://img.shields.io/badge/Qt-5.15.2_MinGW_64Bit-41cd52.svg)
![qt_make](https://img.shields.io/badge/Qt-cmake-green.svg)

[![QQ 群](https://img.shields.io/badge/QQ%E7%BE%A4-495477288-orange.svg)](http://qm.qq.com/cgi-bin/qm/qr?_wv=1027&k=DkzYlCZ9VSQEq6CqUtqGiqYBZh1V5CKK&authKey=btu30mBqaqx6GSVS3futp%2BhYitMfhtAltmp%2B84Kob9xS%2F6J5yQkd0dSeozzxbclT&noverify=0&group_code=495477288)

[DAP-Link](https://github.com/ARM-software/CMSIS-DAP) upper by [Qt](http://qt.io)
 
chip device libray: <https://github.com/ma6254/qdap_chips>

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
- <https://github.com/XIVN1987/DAPProg>
- <https://github.com/tthe207/DP-SW>
- <https://www.keil.arm.com/devices/>
- <https://en.wikipedia.org/wiki/Executable_and_Linkable_Format>

assets icons: <https://fonts.google.com/icons>

[glic_elf.h](./src/glibc_elf.h) from <https://github.com/lattera/glibc/blob/master/elf/elf.h>

[FlashOS.h](./src/FlashOS.h) from <https://github.com/ARM-software/CMSIS_4/blob/master/Device/_Template_Flash/FlashOS.h>
