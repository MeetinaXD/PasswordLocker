# PasswordLocker
 pin code password locker.

## Attention

项目运行在`WAVGAT 328p (5v 16Mhz)`，此处可以看到相关的链接：
[WAVGAT Pro Mini ATMEGA328P 328 Mini ATMEGA328 5V 16MHz for arduino Nano Microcontrol Micro Control Board](https://www.aliexpress.com/item/32954774902.html)

但实际上，这是一个伪劣假冒商品，其宣称运行在`16Mhz`上，但实际运行在`4Mhz`，按照Arduino 官方提供的`Arduino pro mini`开发板烧写程序会造成一些奇怪的问题。

因此，请使用假冒厂商提供的驱动程序：

[WAVGAT 328p Driver - Google Drive](https://drive.google.com/file/d/10gwrG9uTDwaEO-7EudsmBkfgdcyrcABI/view)

或在`Driver`文件夹找到该驱动。

### 驱动使用方法

在`macOS`下，将驱动放置在以下目录：

```
// for 'libraries' folder
/Applications/Arduino.app/Contents/Java/libraries

// for 'hardware' folder
/Applications/Arduino.app/Contents/Java/hardware
```

使用Windows请自己百度。

> 忠告：不要购买伪劣或假冒商品
