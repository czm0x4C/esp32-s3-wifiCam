[TOC]
---
# esp32-camera组件导入
因为涉及使用到OV2640，为了方便这里使用开源的ESP摄像头调用库，它支持很多型号，任君挑选，详见esp32-camera

ESP-IDF对芯片的优化（重要）
由于需要传输大量的图像数据，所以要对芯片设置一些额外设置

因为用到WEB配网，所以[![](https://github.com/FENGYUQWQ/esp32-s3-wifiCam/blob/main/image/ESP-IDF-HTTP%E9%85%8D%E7%BD%AE.png)](https://github.com/FENGYUQWQ/esp32-s3-wifiCam/blob/main/image/ESP-IDF-HTTP%E9%85%8D%E7%BD%AE.png)

# 已知BUG
硬件：不支持超过24M的时钟，暂时不知原因，没有设置自定义的按键

软件：对异常处理的机制不完善，程序容易死机，还需要优化

