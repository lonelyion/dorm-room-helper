# ESP32-Unlock-Door

这是一个用来控制宿舍门禁的程序，硬件基于ESP32 + PN532.

## Dependencies

这个项目使用了以下库：

+ [Arduino-ESP32](https://github.com/espressif/arduino-esp32)
+ [elechouse/PN532](https://github.com/elechouse/PN532)

## Build

这个项目使用的开发平台是[PlatformIO](https://platformio.org/)，因此需要配置好PlatformIO Core或者PlatformIO IDE(其实就是一个VS Code插件)，本部分请参阅官方文档，此处不再赘述。

首先需要在`include`目录下新建一个文件`secrets.h`，然后写入以下内容并将数组内容替换为将要允许放行的NFC UIDs。NFC卡片的UID可以通过打开NFC类的show_debug_info在串口输出里面得到，也可以通过其他读卡器及其配套软件得到。

```cpp
#ifndef _SECRETS_H
#define _SECRETS_H

//NFC 允许放行的白名单列表
#define _ALLOW_LIST {   \
    {0x00, 0x00, 0x00, 0x01}, {0x00, 0x00, 0x00, 0x02}, {0x00, 0x00, 0x00, 0x03},    \
    {0x00, 0x00, 0x00, 0x04}, {0x00, 0x00, 0x00, 0x05}  \
}

//WiFi SSID和连接密码
#define _WIFI_SSID "ssid"
#define _WIFI_PASSWORD "password"

//是否开启夜间省电（默认00:30~07:00）
#define _ENABLE_SLEEP true

#endif
```

之后点击VS Code下方的Build或在CLI中执行`pio run`，再点击Upload或者CLI中执行`pio run --target upload`就可以将编译好的程序烧录到板子上了。

## License

[MIT License](/LICENSE)