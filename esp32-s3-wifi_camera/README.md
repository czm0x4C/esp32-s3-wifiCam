# esp32-camera组件导入
因为涉及使用到OV2640，为了方便这里使用开源的ESP摄像头调用库，它支持很多型号，任君挑选，详见[esp32-camera](https://github.com/espressif/esp32-camera.git)

# ESP-IDF对芯片的优化（重要）
由于需要传输大量的图像数据，所以要对芯片设置一些额外设置

因为用到WEB配网，所以
图片: [外链图片转存失败,源站可能有防盗链机制,建议将图片保存下来直接上传(img-U1yXUwJs-1687833148310)(https://github.com/FENGYUQWQ/esp32-s3-wifiCam/blob/main/image/ESP-IDF-HTTP%E9%85%8D%E7%BD%AE.png)]




# 已知BUG
硬件：不支持超过24M的时钟，暂时不知原因，没有设置自定义的按键

软件：对异常处理的机制不完善，程序容易死机，还需要优化


	

