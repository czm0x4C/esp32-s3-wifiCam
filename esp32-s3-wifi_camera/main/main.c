#include "wifiConnect.h"
#include "httpServer.h"
#include "camera.h"

void app_main(void)
{
    // 关闭USB jtag 功能
    gpio_reset_pin(19);
    gpio_reset_pin(20);   

    unsigned char connectState[10];/*  */
    char isConnectedFlag = 0;
    ESP_LOGI("main","readWifiInfo");
    readWifiInfo(wifiName,wifiPassword,connectState); /* 读取要连接的wifi信息 */
    if(strcmp((char *)connectState,"fail") == 0)      /* 保存的wifi信息无效 */
    {
        ESP_LOGI("main","start wifi softap");
        wifi_init_softap();                           /* wifi AP模式 用户连接ESP32产生的wifi */
        static httpd_handle_t server = NULL;
        ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server));
        ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server));
        /* Start the server for the first time */
        server = start_webserver();
    }
    else if(strcmp((char *)connectState,"ok") == 0)   /* 保存的信息有效 */
    {
        ESP_LOGI("main","start wifi sta");
        wifi_init_sta((char *)wifiName,(char *)wifiPassword,&isConnectedFlag);  /* 连接指定的wifi */
        ESP_LOGI("wifiConnect","state = %d",isConnectedFlag);                   /* 判断连接是否成功 */
        if(isConnectedFlag == 0)                                                
        {
            writeWifiInfo((unsigned char *)"111",(unsigned char *)"111",(unsigned char *)"111",1111,(unsigned char *)"fail");   /* 连接失败处理 */
            esp_restart();/* 重启设备 */
        }
        else if(isConnectedFlag == 1)/* 连接成功处理 */
        {
            cameraSetConfig();                                                           /* 读取保存的设备信息 */
            appInit();
        }   
    }
}
