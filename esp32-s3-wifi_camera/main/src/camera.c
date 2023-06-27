#include "camera.h"

#define UDP_SEND_MAX_LEN ( 60 * 1024 ) /* 设置UDP一包发送的最大数量 */

_deviceInfo deviceAttributeInfo;

nvs_handle_t nvsHandle = {0};   /* NVS读写数据句柄 */
struct tm timeinfo = {0};
static const char *UDP = "UDP";
static const char udpCmd_readDeviceAttribute[] = "readDeviceAttribute"; 
static const char udpCmd_setDeviceAttribute[] = "setDeviceAttribute"; 

TaskHandle_t cameraTaskHandle;  /* 摄像头采任务的句柄 */
void udpReceive_task(void *pvParameters);                   /* udp数据接收任务 */
void camera_task(void *pvParameters);
/* socket使用的相关变量 */
int addr_family = 0;
int ip_protocol = 0;
struct sockaddr_in dest_addr;
int udpSock = 0;
/* 时间戳变量 */
unsigned long int sysStartTimeStamp = 0;
unsigned long int nowTimeStamp = 0; 
/* 系统指示灯变量 */
unsigned char netOnlineFlag;
unsigned char workFlag;

#define CAM_PIN_PWDN    (45)
#define CAM_PIN_RESET   (48) //software reset will be performed
#define CAM_PIN_XCLK    (47)
#define CAM_PIN_SIOD    (18)
#define CAM_PIN_SIOC    ( 8)

#define CAM_PIN_D7      (12)
#define CAM_PIN_D6      (11)
#define CAM_PIN_D5      (10)
#define CAM_PIN_D4      ( 9)
#define CAM_PIN_D3      (46)
#define CAM_PIN_D2      ( 3)
#define CAM_PIN_D1      (20)
#define CAM_PIN_D0      (19)
#define CAM_PIN_VSYNC   (13)
#define CAM_PIN_HREF    (21)
#define CAM_PIN_PCLK    (14)
static const char *camera = "camera";

static camera_config_t camera_config = {
    .pin_pwdn = CAM_PIN_PWDN,
    .pin_reset = CAM_PIN_RESET,
    .pin_xclk = CAM_PIN_XCLK,
    .pin_sccb_sda = CAM_PIN_SIOD,
    .pin_sccb_scl = CAM_PIN_SIOC,

    .pin_d7 = CAM_PIN_D7,
    .pin_d6 = CAM_PIN_D6,
    .pin_d5 = CAM_PIN_D5,
    .pin_d4 = CAM_PIN_D4,
    .pin_d3 = CAM_PIN_D3,
    .pin_d2 = CAM_PIN_D2,
    .pin_d1 = CAM_PIN_D1,
    .pin_d0 = CAM_PIN_D0,
    .pin_vsync = CAM_PIN_VSYNC,
    .pin_href = CAM_PIN_HREF,
    .pin_pclk = CAM_PIN_PCLK,

    //XCLK 20MHz or 10MHz for OV2640 double FPS (Experimental)
    .xclk_freq_hz = 22.5 * 1000 * 1000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_JPEG,     //YUV422,GRAYSCALE,RGB565,JPEG
    .frame_size = FRAMESIZE_UXGA,       //QQVGA-UXGA, For ESP32, do not use sizes above QVGA when not JPEG. The performance of the ESP32-S series has improved a lot, but JPEG mode always gives better frame rates.
    //FRAMESIZE_QVGA FRAMESIZE_UXGA
    .jpeg_quality = 8,                  //0-63, for OV series camera sensors, lower number means higher quality
    .fb_count = 2,                      //When jpeg mode is used, if fb_count more than one, the driver will work in continuous mode.
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
};

void sntp_Init(void)/* 获取实时网络时间 */
{
    /* 使用多个sntp服务器时，请在 make menuconfig -> Component config -> LWIP -> SNTP -> Maximum bumber of NTP servers 修改为 所使用的个数 */
    sntp_stop();
    ESP_LOGI("sntp","start.");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);    // 设置单播模式
    sntp_setservername(0, "cn.ntp.org.cn");     // 设置访问服务器
    sntp_setservername(1, "ntp1.aliyun.com");
    sntp_setservername(2, "pool.ntp.org");
    sntp_setservername(3, "210.72.145.44");     // 国家授时中心服务器 IP 地址
    setenv("TZ", "CST-8", 1);                   // 东八区
    tzset();                                    // 更新本地C库时间
    sntp_init();                                // 初始化

    // 延时等待SNTP初始化完成
    do {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        ESP_LOGI("sntp","wait for wifi sntp sync time---------------------");
    } while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET);

    // 成功获取网络时间后停止NTP请求，不然设备重启后会造成获取网络时间失败的现象
    // 大概是服务器时根据心跳时间来删除客户端的，如果不是stop结束的客户端，下次连接服务器时就会出错
	sntp_stop();
    ESP_LOGI("sntp","sntp stop.");
}

void get_time(void)
{
    time_t now;
    time(&now);                         // 获取网络时间, 64bit的秒计数 

    localtime_r(&now, &timeinfo);       // 转换成具体的时间参数
    ESP_LOGI("sntp", "%4d-%02d-%02d %02d:%02d:%02d week:%d", timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, 
        timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, timeinfo.tm_wday); 
}

/* 读取储存的摄像头信息，进行初始化配置 */
void cameraSetConfig(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) 
    {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }  
    ESP_ERROR_CHECK( err );

    ESP_ERROR_CHECK(nvs_open("deviceInfo", NVS_READWRITE, &nvsHandle));

    /* 先读NVS指定键值对中是否有数据 */
    unsigned int readNvsLen = 0;
    err = nvs_get_str(nvsHandle, "deviceState",NULL, &readNvsLen);/* 判断当前设备的状态 */
    char devideState[10];
    if(readNvsLen > 0)  /* 有数据 */
    {
        readNvsLen = 10;
        memset(devideState,0,10);
        err = nvs_get_str(nvsHandle, "deviceState",devideState, &readNvsLen); /* 读取保存在nvs中的设备状态*/
        if(strcmp(devideState,"online") == 0) /* 设备状态就绪,开始读取保存的参数数据 */
        {
            readNvsLen = 0;
            ESP_ERROR_CHECK(nvs_get_str(nvsHandle, "deviceID",NULL, &readNvsLen));/* 读取设备ID */
            if(readNvsLen > 0 )/* 有数据，那么就读取 */
            {
                readNvsLen = sizeof(deviceAttributeInfo.deviceID);
                err = nvs_get_str(nvsHandle, "deviceID",deviceAttributeInfo.deviceID, &readNvsLen); /* 读取保存在nvs中的设备ID */
                ESP_LOGI("nvs","deviceID is %s,len = %d",deviceAttributeInfo.deviceID,readNvsLen - 1);
            }

            uint32_t out_value;
            ESP_ERROR_CHECK(nvs_get_u32(nvsHandle, "setVideoTime",&out_value)); /* 读取保存在nvs中的录像时间 */
            deviceAttributeInfo.scheduledDeletion = out_value;
            ESP_LOGI("nvs","scheduledDeletion = %d.\n",deviceAttributeInfo.scheduledDeletion);

            readNvsLen = 0;
            err = nvs_get_str(nvsHandle, "picFormat",NULL, &readNvsLen);/* 读取设备ID */
            if(err != ESP_OK){
                strcpy(deviceAttributeInfo.picFormat,"PIXFORMAT_JPEG");
                camera_config.pixel_format = PIXFORMAT_JPEG;
                ESP_ERROR_CHECK(nvs_set_str(nvsHandle,"picFormat",(char *)deviceAttributeInfo.picFormat));/* 写入数据 */ 
                ESP_ERROR_CHECK(nvs_commit(nvsHandle)); /* 确认数据写入 */
            }
            else
            {
                if(readNvsLen > 0 )/* 有数据，那么就读取 */
                {
                    readNvsLen = sizeof(deviceAttributeInfo.picFormat);
                    err = nvs_get_str(nvsHandle, "picFormat",deviceAttributeInfo.picFormat, &readNvsLen); /* 读取保存在nvs中的设备ID */
                    ESP_LOGI("nvs","picFormat is %s,len = %d",deviceAttributeInfo.picFormat,readNvsLen - 1);

                    if(strcmp((char *)deviceAttributeInfo.picFormat,(char *)"PIXFORMAT_JPEG") == 0)
                    {
                        camera_config.pixel_format = PIXFORMAT_JPEG;
                    }
                    else
                    {
                        camera_config.pixel_format = PIXFORMAT_JPEG;
                    }
                }
            }

            err = nvs_get_u32(nvsHandle, "picQuality",&out_value); /* 读取保存在nvs中的JPEG图像质量 */
            if(err != ESP_OK){
                deviceAttributeInfo.jpegQuality = 20;
                camera_config.jpeg_quality = deviceAttributeInfo.jpegQuality;
                uint32_t write_value = deviceAttributeInfo.jpegQuality;
                ESP_ERROR_CHECK(nvs_set_u32(nvsHandle,"picQuality",write_value));/* 写入数据 */ 
                ESP_ERROR_CHECK(nvs_commit(nvsHandle)); /* 确认数据写入 */
            }
            else
            {
                deviceAttributeInfo.jpegQuality = out_value;
                ESP_LOGI("nvs","jpegQuality = %d.\n",deviceAttributeInfo.jpegQuality);
            }
            
            
            err = nvs_get_u32(nvsHandle, "picSize",&out_value); /* 读取保存在nvs中的图像大小 */
            if(err != ESP_OK){
                camera_config.frame_size = FRAMESIZE_UXGA;
                uint32_t write_value = camera_config.frame_size;
                ESP_ERROR_CHECK(nvs_set_u32(nvsHandle,"picSize",write_value));/* 写入数据 */ 
                ESP_ERROR_CHECK(nvs_commit(nvsHandle)); /* 确认数据写入 */
            }
            else
            {
                camera_config.frame_size = out_value;
                ESP_LOGI("nvs","frame_size = %d.\n",camera_config.frame_size);
            }

            nvs_close(nvsHandle);

            err = nvs_open("wifiInfo", NVS_READWRITE, &nvsHandle);
            if (err != ESP_OK) 
            {
                printf("Error (%s) opening NVS handle!", esp_err_to_name(err));
            }

            readNvsLen = 0;
            err = nvs_get_str(nvsHandle, "udpIP",NULL, &readNvsLen);/* 读取udp地址 */
            if(readNvsLen > 0 )/* 有数据，那么就读取 */
            {
                readNvsLen = sizeof(deviceAttributeInfo.UDP_serverIP);
                err = nvs_get_str(nvsHandle, "udpIP",deviceAttributeInfo.UDP_serverIP, &readNvsLen); /* 读取保存在nvs中的udp ip */
                ESP_LOGI("nvs","UDP_serverIP is %s,len = %d",deviceAttributeInfo.UDP_serverIP,readNvsLen - 1);
            }
            
            out_value = 0;
            err = nvs_get_u32(nvsHandle, "udpPORT",&out_value);/* 读取udp地址 */
            deviceAttributeInfo.UDP_serverPort = out_value;
            ESP_LOGI("nvs","UDP_serverPort = %d.",deviceAttributeInfo.UDP_serverPort);
            nvs_close(nvsHandle);
            
        }
        else
        {
            strcpy(devideState,"");/* 写入空数据 */
            err = nvs_set_str(nvsHandle,"deviceState",devideState);/* 写入数据 */
            if(err==ESP_OK)
            {
                ESP_LOGI("nvs","set deviceID success!\n");
            }
            else
            {
                ESP_LOGI("nvs","set deviceID fail!\n");
            }  
            err = nvs_commit(nvsHandle); /* 确认数据写入 */
            if(err != ESP_OK)
            {
                ESP_LOGI("nvs","nvs_commit Failed!");
            }
            else
            {
                ESP_LOGI("nvs","nvs_commit Done!");
            }
        }
    }
    else        /* 读取设备状态，未就绪 */
    {
        sntp_Init(); /* 利用sntp定时器获取时间 */
        get_time();/* 获取时间 */
        strcpy(devideState,"online");
        err = nvs_set_str(nvsHandle,"deviceState",devideState);/* 写入设备状态，就绪 */
        if(err==ESP_OK)
        {
            ESP_LOGI("nvs","set deviceState success!\n");
        }
        else
        {
            ESP_LOGI("nvs","set deviceState fail!\n");
        }  
        memset(deviceAttributeInfo.deviceID,0,sizeof(deviceAttributeInfo.deviceID));
        sprintf(deviceAttributeInfo.deviceID,"%04d%02d%02d%02d%02d%02d",  timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                                                                     timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);/* 年月日时分秒 */
        ESP_LOGI("nvs","set data %s .\n",deviceAttributeInfo.deviceID);
        err = nvs_open("deviceInfo", NVS_READWRITE, &nvsHandle);
        err = nvs_set_str(nvsHandle,"deviceID",deviceAttributeInfo.deviceID);/* 写入设备ID */
        if(err==ESP_OK)
        {
            ESP_LOGI("nvs","set deviceID success!\n");
        }
        else
        {
            ESP_LOGI("nvs","set deviceID fail!\n");
        }  

        deviceAttributeInfo.scheduledDeletion = 3600 * 24 * 15;
        err = nvs_set_u32(nvsHandle,"setVideoTime",deviceAttributeInfo.scheduledDeletion);/* 写入录像时间 */  
        if(err==ESP_OK)
        {
            ESP_LOGI("nvs","set setVideoTime success!\n");
        }
        else
        {
            ESP_LOGI("nvs","set setVideoTime fail!\n");
        }  

        err = nvs_commit(nvsHandle); /* 确认数据写入 */
        if(err != ESP_OK)
        {
            ESP_LOGI("nvs","nvs_commit Failed!");
        }
        else
        {
            ESP_LOGI("nvs","nvs_commit Done!");
        }
        
        nvs_close(nvsHandle);
        esp_restart();/* 重启设备 */
    }
    
    nvs_close(nvsHandle);
}


void getDeviceInfo(void)
{
    switch(camera_config.pixel_format)/* 判断设置的图片格式 */
    {
        case PIXFORMAT_RGB565:
            memset(deviceAttributeInfo.picFormat,0,20);/* 清空数组，长度20 */
            sprintf(deviceAttributeInfo.picFormat,"%s","PIXFORMAT_RGB565");
            break;
        case PIXFORMAT_YUV422:
            memset(deviceAttributeInfo.picFormat,0,20);/* 清空数组，长度20 */
            sprintf(deviceAttributeInfo.picFormat,"%s","PIXFORMAT_YUV422");
            break;            
        case PIXFORMAT_YUV420:
            memset(deviceAttributeInfo.picFormat,0,20);/* 清空数组，长度20 */
            sprintf(deviceAttributeInfo.picFormat,"%s","PIXFORMAT_YUV420");
            break;
        case PIXFORMAT_GRAYSCALE:
            memset(deviceAttributeInfo.picFormat,0,20);/* 清空数组，长度20 */
            sprintf(deviceAttributeInfo.picFormat,"%s","PIXFORMAT_GRAYSCALE");
            break;
        case PIXFORMAT_JPEG:
            memset(deviceAttributeInfo.picFormat,0,20);/* 清空数组，长度20 */
            sprintf(deviceAttributeInfo.picFormat,"%s","PIXFORMAT_JPEG");
            break;
        case PIXFORMAT_RGB888:
            memset(deviceAttributeInfo.picFormat,0,20);/* 清空数组，长度20 */
            sprintf(deviceAttributeInfo.picFormat,"%s","PIXFORMAT_RGB888");
            break;
        default:
            memset(deviceAttributeInfo.picFormat,0,20);/* 清空数组，长度20 */
            break;
    }
    switch(camera_config.frame_size)/* 判断设置的图片大小 */
    {
        case FRAMESIZE_QVGA:
            deviceAttributeInfo.picWidth  = 320;
            deviceAttributeInfo.picHeight = 240;
            break;
        case FRAMESIZE_VGA:
            deviceAttributeInfo.picWidth  = 680;
            deviceAttributeInfo.picHeight = 480;
            break;    
        case FRAMESIZE_UXGA:
            deviceAttributeInfo.picWidth  = 1600;
            deviceAttributeInfo.picHeight = 1200;
            break;           
        default:
            break;
    }
    deviceAttributeInfo.jpegQuality = camera_config.jpeg_quality;
}
void camera_task(void *pvParameters)
{
    camera_fb_t *pic;
    esp_camera_deinit();                                /* 复位摄像头的硬件设置 */
    esp_err_t err = esp_camera_init(&camera_config);    /* 按照既定参数设置摄像头 */
    if (err != ESP_OK)
    {
        ESP_LOGE(camera, "Camera Init Failed");
        return ;
    }

    getDeviceInfo();                                    /* 获取配置信息 */

    while(1)
    {
        // ESP_LOGI(camera, "Taking picture...");
        pic = esp_camera_fb_get();
        if( pic != NULL)/* 接收的图像数据正常 */
        {
            /* 使用 UDP 发送时，一个数据包大小不超过64K*/
            unsigned int preSendPicLen = pic->len;
            int sendCnt = 0;
            unsigned char * startPalce =  pic->buf;
            /* 发送图片数据数据时，首先发送一些信息 */
            /* 发送frameData,图片大小,esp32的时间戳,设备ID */
            char picInfo[100];
            sprintf(picInfo,"%s,%d,%ld,%s","frameData",pic->len,esp_log_timestamp(),deviceAttributeInfo.deviceID);
            sendto(udpSock,picInfo,strlen(picInfo), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));/* 发送图片大小数据包 */
            vTaskDelay(10 / portTICK_PERIOD_MS);
            /* 发送图片数据包 */
            if(preSendPicLen > UDP_SEND_MAX_LEN) /* 如果图片的大小大于设定的最大值，分包发送 */
            {
                // ESP_LOGI(camera,"preSendPicLen = %d \r\n",preSendPicLen);
                while(UDP_SEND_MAX_LEN * (sendCnt + 1) < preSendPicLen)
                {
                    sendto(udpSock,startPalce + (UDP_SEND_MAX_LEN * sendCnt),UDP_SEND_MAX_LEN, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
                    vTaskDelay(10 / portTICK_PERIOD_MS);
                    sendCnt++;
                }
                sendto(udpSock,startPalce + (UDP_SEND_MAX_LEN * sendCnt),preSendPicLen % UDP_SEND_MAX_LEN, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
                vTaskDelay(10 / portTICK_PERIOD_MS);
                sendCnt = 0;
            }
            else                            /* 如果图片的大小没有大于设定的最大值，直接发送 */
            {
                sendto(udpSock,startPalce,preSendPicLen, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
                vTaskDelay(1 / portTICK_PERIOD_MS);
            }

            nowTimeStamp =  esp_log_timestamp();
            if(((nowTimeStamp/1000 - sysStartTimeStamp/1000)) >= deviceAttributeInfo.scheduledDeletion)/* 如果采集图像的时间大于了定时删除的时间 */
            {
                /* 发送删除储存的录像视频的命令 */
                memset(picInfo,0,sizeof(picInfo));
                sprintf(picInfo,"%s,%s","deleteVideo",deviceAttributeInfo.deviceID);
                sendto(udpSock,picInfo,strlen(picInfo), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));/* 发送图片大小数据包 */
                vTaskDelay(10 / portTICK_PERIOD_MS);
                ESP_LOGI("nvs","sysStartTimeStamp = %ld",sysStartTimeStamp);
                ESP_LOGI("nvs","nowTimeStamp = %ld",nowTimeStamp);
                sysStartTimeStamp = nowTimeStamp;/* 更新下一次周期 */
            }
            // use pic->buf to access the image
            // ESP_LOGI(camera, "Picture taken! Its size was: %zu bytes,width: %zu,height: %zu", pic->len,pic->width,pic->height);
            esp_camera_fb_return(pic);
        }
        else
        {
            ESP_LOGI("camera","error");
            sendto(udpSock,"error",5, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
            vTaskDelay(200 / portTICK_PERIOD_MS);
            workFlag = 0;
        }
    }
}
/* 处理UDP接收的数据 */
void udpReceive_task(void *pvParameters)
{
    /*阻塞接收UDP数据*/
    static unsigned char rx_buffer[100];
    struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
    socklen_t socklen = sizeof(source_addr);

    while (1)
    {
        int len = recvfrom(udpSock, rx_buffer, sizeof(rx_buffer) - 1,0, (struct sockaddr *)&source_addr, &socklen);
        if (len > 0) /* 数据长度大于0，说明接收有数据 */
        {
            rx_buffer[len] = 0;                          // Null-terminate whatever we received and treat like a string
            ESP_LOGI(UDP, "Received %d bytes", len);
            
            if(strcmp((char *)rx_buffer, (char *)udpCmd_readDeviceAttribute) == 0) /* 如果是读取设备信息命令 */
            {
                ESP_LOGI(UDP, "%s", rx_buffer);
                char deviceAttributeString[200];
                char cmdSrting[] = "backDeviceInfo";
                sprintf(deviceAttributeString,"%s,%s,%d,%d,%d,%s,%d", 
                                            cmdSrting,
                                            deviceAttributeInfo.picFormat,      /* 图片格式 */
                                            deviceAttributeInfo.picWidth,       /* 图片宽*/
                                            deviceAttributeInfo.picHeight,      /* 图片高*/
                                            deviceAttributeInfo.scheduledDeletion,/* 设备定时删除时间 */
                                            deviceAttributeInfo.deviceID,       /* 设备ID */
                                            deviceAttributeInfo.jpegQuality     /* jpeg图像质量 */
                                            );     
                ESP_LOGI(UDP, "%s", deviceAttributeString);
                sendto(udpSock,&deviceAttributeString,strlen(deviceAttributeString), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));/* 返回设备属性信息 */
            }
            if(strstr((char *)rx_buffer, (char *)udpCmd_setDeviceAttribute) != NULL) /* 如果是设置相机参数命令 */
            {
                ESP_LOGI(UDP, "%s", rx_buffer);
                ESP_LOGI("sb", "123");
                char *tempString = strtok((char *)rx_buffer,",");
                unsigned char srtPlace = 1;/* 字段位置计数 */
                while(tempString)
                {
                    /* 获取字段的字符串 */
                    tempString = strtok(NULL,",");
                    if(srtPlace == 1)                /* 获取图片格式 */
                    {
                        memset(deviceAttributeInfo.picFormat,0,sizeof(deviceAttributeInfo.picFormat));/* 清空数组 */
                        sprintf(deviceAttributeInfo.picFormat,"%s",tempString);
                        if(strcmp((char *)deviceAttributeInfo.picFormat,(char *)"PIXFORMAT_JPEG") == 0)camera_config.pixel_format = PIXFORMAT_JPEG; /* 如果接收设置为JPEG模式 */

                        ESP_ERROR_CHECK(nvs_open("deviceInfo", NVS_READWRITE, &nvsHandle));
                        ESP_ERROR_CHECK(nvs_set_str(nvsHandle,"picFormat",(char *)deviceAttributeInfo.picFormat));/* 写入数据 */ 
                        ESP_ERROR_CHECK(nvs_commit(nvsHandle)); /* 确认数据写入 */
                        nvs_close(nvsHandle);

                        ESP_LOGI(UDP, "picFormat = %s", deviceAttributeInfo.picFormat);
                    }
                    else if(srtPlace == 2)          /* 图片宽 */
                    {
                        deviceAttributeInfo.picWidth = atoi(tempString);
                        ESP_LOGI(UDP, "%s", tempString);
                        ESP_LOGI(UDP, "picWidth = %d", deviceAttributeInfo.picWidth);
                    }
                    else if(srtPlace == 3)          /* 图片高 */
                    {
                        deviceAttributeInfo.picHeight = atoi(tempString);
                        ESP_LOGI(UDP, "%s", tempString);
                        ESP_LOGI(UDP, "picHeight = %d", deviceAttributeInfo.picHeight);
                    }
                    else if(srtPlace == 4)          /* 相机定时删除时间 */
                    {
                        deviceAttributeInfo.scheduledDeletion = (unsigned int)atoi(tempString);
                        ESP_LOGI(UDP, "%s", tempString);
                        ESP_LOGI(UDP, "scheduledDeletion = %d", deviceAttributeInfo.scheduledDeletion);
                        ESP_ERROR_CHECK(nvs_open("deviceInfo", NVS_READWRITE, &nvsHandle));
                        uint32_t write_value = deviceAttributeInfo.scheduledDeletion;
                        ESP_ERROR_CHECK(nvs_set_u32(nvsHandle,"setVideoTime",write_value));/* 写入数据 */  
                        ESP_ERROR_CHECK(nvs_commit(nvsHandle)); /* 确认数据写入 */
                        nvs_close(nvsHandle);
                    }
                    else if(srtPlace == 5)          /* 设备ID */
                    {
                        memset(deviceAttributeInfo.deviceID,0,sizeof(deviceAttributeInfo.deviceID));/* 清空数组 */
                        sprintf(deviceAttributeInfo.deviceID,"%s",tempString);
                        ESP_LOGI(UDP, "deviceID = %s", tempString);
                    }
                    else if(srtPlace == 6)          /* JPEG图片质量 */
                    {
                        deviceAttributeInfo.jpegQuality = atoi(tempString);
                        camera_config.jpeg_quality = deviceAttributeInfo.jpegQuality;

                        ESP_ERROR_CHECK(nvs_open("deviceInfo", NVS_READWRITE, &nvsHandle));
                        uint32_t write_value = deviceAttributeInfo.jpegQuality;
                        ESP_ERROR_CHECK(nvs_set_u32(nvsHandle,"picQuality",write_value));/* 写入数据 */ 
                        ESP_ERROR_CHECK(nvs_commit(nvsHandle)); /* 确认数据写入 */
                        nvs_close(nvsHandle);

                        ESP_LOGI(UDP, "%s", tempString);
                        ESP_LOGI(UDP, "jpegQuality = %d", deviceAttributeInfo.jpegQuality);
                    }
                    srtPlace++;
                }
                if((deviceAttributeInfo.picWidth == 320) && (deviceAttributeInfo.picHeight == 240))camera_config.frame_size = FRAMESIZE_QVGA;
                else if((deviceAttributeInfo.picWidth == 680) && (deviceAttributeInfo.picHeight == 480))camera_config.frame_size = FRAMESIZE_VGA;
                else if((deviceAttributeInfo.picWidth == 1600) && (deviceAttributeInfo.picHeight == 1200))camera_config.frame_size = FRAMESIZE_UXGA;

                ESP_ERROR_CHECK(nvs_open("deviceInfo", NVS_READWRITE, &nvsHandle));
                uint32_t write_value = camera_config.frame_size;
                ESP_ERROR_CHECK(nvs_set_u32(nvsHandle,"picSize",write_value));/* 写入数据 */ 
                ESP_ERROR_CHECK(nvs_commit(nvsHandle)); /* 确认数据写入 */
                nvs_close(nvsHandle);

                vTaskDelete(cameraTaskHandle);  /* 删除摄像头任务 */
                vTaskDelay(100 / portTICK_PERIOD_MS);
                xTaskCreate(camera_task, "camera_task", 1024 * 50, NULL, 3, &cameraTaskHandle); /* 重新创建摄像头任务 */
            }
        }
    }
}

void led_task(void *pvParameters)
{
    LED_Init();/* led初始化 */
    ESP_LOGE("LED", "init");
    while(1)
    {
        if(netOnlineFlag)
        {
            netLedOn();
        }
        else
        {
            netLedOff();
        }
        if(workFlag)
        {
            workLedOn();
        }
        else
        {
            workLedOff();
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);

    }
}
void appInit(void)
{
    netOnlineFlag = 1;
    workFlag = 0;

    /* 当wifi连接正常后才开始socket设置和连接 */
    dest_addr.sin_addr.s_addr = inet_addr(deviceAttributeInfo.UDP_serverIP);/* 设置目标ip地址 */
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(deviceAttributeInfo.UDP_serverPort);   /* 设置目标端口 */
    addr_family = AF_INET;
    ip_protocol = IPPROTO_IP;

    udpSock = socket(addr_family, SOCK_DGRAM, ip_protocol);
    if (udpSock < 0) {
        ESP_LOGE(UDP, "Unable to create socket: errno %d", errno);
    }

    // Set timeout
    struct timeval timeout;
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    setsockopt (udpSock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);
    ESP_LOGI(UDP, "Socket created, sending to %s:%d", deviceAttributeInfo.UDP_serverIP, deviceAttributeInfo.UDP_serverPort);

    //获取剩余的dram大小
	size_t dram = heap_caps_get_free_size(MALLOC_CAP_8BIT);

    //获取剩余的iram大小
	size_t iram = heap_caps_get_free_size(MALLOC_CAP_32BIT) - heap_caps_get_free_size(MALLOC_CAP_8BIT);

    //获取剩余的dram大小与heap_caps_get_free_size函数一样
	uint32_t data = xPortGetFreeHeapSize();

    //获取最大的连续的堆区空间
	size_t heapmax = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
	int* p = (int*)malloc(heapmax);
	free(p);

    //获取栈区的最高水位线(也就是栈顶，意味着最大栈区空间)
	int stackmark=uxTaskGetStackHighWaterMark(NULL);
	char stack[6000];
	memset(stack, 1, 6000);
	
	printf("data=%ld\n", data);
	printf("dram=%d\n", dram);
	printf("iram=%d\n", iram);
	printf("max=%d\n", heapmax);
	printf("stackmark=%d\n", stackmark);

    /* 创建摄像头任务 */
    xTaskCreate(camera_task, "camera_task", 1024 * 50, NULL, 2, &cameraTaskHandle); 

    /* 创建udp接收数据任务 */
    xTaskCreate(udpReceive_task, "udpReceive_task", 1024 * 50, NULL, 3, NULL); 

    xTaskCreate(led_task, "led_task", 1024 * 10, NULL, 3, NULL); 
    
    workFlag = 1;

    //获取剩余的dram大小
	dram = heap_caps_get_free_size(MALLOC_CAP_8BIT);

    //获取剩余的iram大小
	iram = heap_caps_get_free_size(MALLOC_CAP_32BIT) - heap_caps_get_free_size(MALLOC_CAP_8BIT);

    //获取剩余的dram大小与heap_caps_get_free_size函数一样
	data = xPortGetFreeHeapSize();

    //获取最大的连续的堆区空间
	heapmax = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
	p = (int*)malloc(heapmax);
	free(p);

    //获取栈区的最高水位线(也就是栈顶，意味着最大栈区空间)
	stackmark=uxTaskGetStackHighWaterMark(NULL);
	
	printf("data=%ld\n", data);
	printf("dram=%d\n", dram);
	printf("iram=%d\n", iram);
	printf("max=%d\n", heapmax);
	printf("stackmark=%d\n", stackmark);
}