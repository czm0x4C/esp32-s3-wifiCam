#ifndef __WIFI_CONNECT_H__
#define __WIFI_CONNECT_H__

#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "nvs_flash.h"
#include "esp_netif.h"
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"

extern unsigned char wifiName[50];
extern unsigned char wifiPassword[50];

void wifi_init_softap(void);
void readWifiInfo(  unsigned char * wifiName,unsigned char * wifiPassword,unsigned char *wifiConnectState);
void writeWifiInfo( unsigned char * wifiName,unsigned char * wifiPassword,
                    unsigned char * udpIP,unsigned int udpPORT,
                    unsigned char *wifiConnectState);
void wifi_init_sta(char *WIFI_Name,char *WIFI_PassWord,char *isConnected);
#endif /* __WIFI_CONNECT_H__ */
