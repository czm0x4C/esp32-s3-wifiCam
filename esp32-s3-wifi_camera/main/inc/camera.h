#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_sntp.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_sntp.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"

#include "esp_camera.h"

#include "led.h"

typedef struct 
{
    char picFormat[20];                     /* 图片的格式 */
    unsigned int picWidth;                  /* 图片的长 */
    unsigned int picHeight;                 /* 图片的宽 */
    unsigned int scheduledDeletion;         /* 定时删除时间 */
    char deviceID[50];                      /* 设备ID */
    int jpegQuality;                        /* JPEG图像质量 */
    char UDP_serverIP[20];                  /* udp ip地址 */
    unsigned int UDP_serverPort;            /* udp 端口  */
}_deviceInfo;

extern _deviceInfo deviceAttributeInfo;

void sntp_Init(void);/* 获取实时网络时间 */
void cameraSetConfig(void);
void appInit(void);
#endif /* __CAMERA_H__ */
