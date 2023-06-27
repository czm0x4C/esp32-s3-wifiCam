#include "wifiConnect.h"

static const char *TAG = "wifiConnect";

#define EXAMPLE_ESP_WIFI_SSID      "ESP32S3CAM"
#define EXAMPLE_ESP_WIFI_PASS      "12345678"
#define EXAMPLE_ESP_WIFI_CHANNEL   1
#define EXAMPLE_MAX_STA_CONN       4

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station %02x:%02x:%02x:%02x:%02x:%02x join, AID=%d",
                 event->mac[0],event->mac[1],event->mac[2],event->mac[3],event->mac[4],event->mac[5], event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station %02x:%02x:%02x:%02x:%02x:%02x leave, AID=%d",
                 event->mac[0],event->mac[1],event->mac[2],event->mac[3],event->mac[4],event->mac[5], event->aid);
    }
}

void wifi_init_softap(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
            .channel = EXAMPLE_ESP_WIFI_CHANNEL,
            .password = EXAMPLE_ESP_WIFI_PASS,
            .max_connection = EXAMPLE_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };
    if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS, EXAMPLE_ESP_WIFI_CHANNEL);
}

void readWifiInfo(  unsigned char * wifiName,unsigned char * wifiPassword,unsigned char *wifiConnectState)
{
/*******************************************************************************************************/
    nvs_handle_t nvsHandle = {0}; 
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }  
    ESP_ERROR_CHECK( err );

    err = nvs_open("wifiInfo", NVS_READWRITE, &nvsHandle);
    if (err != ESP_OK) 
    {
        ESP_LOGE("nvs","Error (%s) opening NVS handle!", esp_err_to_name(err));
    }
    /* 先读NVS指定键值对中是否有数据 */
    unsigned int readNvsLen = 0;
    
    err = nvs_get_str(nvsHandle, "wifiName",NULL, &readNvsLen);
    if(readNvsLen > 0 )/* 有数据，那么就读取 */
    {
        readNvsLen = 50;
        err = nvs_get_str(nvsHandle, "wifiName",(char *)wifiName, &readNvsLen); /* 读取保存在nvs中的设备ID */
        ESP_LOGI("nvs","wifiName is %s,len = %d",wifiName,readNvsLen - 1);
    }
    else
    {
        strcpy((char *)wifiName,"");
    }
    
    readNvsLen = 0;
    err = nvs_get_str(nvsHandle, "wifiPassword",NULL, &readNvsLen);
    if(readNvsLen > 0 )/* 有数据，那么就读取 */
    {
        readNvsLen = 50;
        err = nvs_get_str(nvsHandle, "wifiPassword",(char *)wifiPassword, &readNvsLen); /* 读取保存在nvs中的设备ID */
        ESP_LOGI("nvs","wifiPassword is %s,len = %d",wifiPassword,readNvsLen - 1);
    }
    else
    {
        strcpy((char *)wifiPassword,"");    
    }

    readNvsLen = 0;
    err = nvs_get_str(nvsHandle, "connectState",NULL, &readNvsLen);
    if(readNvsLen > 0 )/* 有数据，那么就读取 */
    {
        readNvsLen = 10;
        err = nvs_get_str(nvsHandle, "connectState",(char *)wifiConnectState, &readNvsLen); /* 读取保存在nvs中的设备ID */
        ESP_LOGI("nvs","wifiConnectState is %s,len = %d",wifiConnectState,readNvsLen - 1);
    }
    else
    {
        strcpy((char *)wifiConnectState,"fail"); 
    }
    nvs_close(nvsHandle);
    /*******************************************************************************************************/
}

void writeWifiInfo(unsigned char * wifiName,unsigned char * wifiPassword,
                    unsigned char * udpIP,unsigned int udpPORT,
                    unsigned char *wifiConnectState)
{
/*******************************************************************************************************/
    nvs_handle_t nvsHandle = {0}; 
    // esp_err_t err;
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }  
    ESP_ERROR_CHECK( err );


    err = nvs_open("wifiInfo", NVS_READWRITE, &nvsHandle);
    if (err != ESP_OK) 
    {
        printf("Error (%s) opening NVS handle!", esp_err_to_name(err));
    }

    ESP_ERROR_CHECK(nvs_set_str(nvsHandle,"wifiName",(char *)wifiName));            /* 写入wifiName */
    ESP_ERROR_CHECK(nvs_set_str(nvsHandle,"wifiPassword",(char *)wifiPassword));    /* 写入wifiPassword */
    ESP_ERROR_CHECK(nvs_set_str(nvsHandle,"udpIP",(char *)udpIP));                  /* 写入dupIP   */
    ESP_ERROR_CHECK(nvs_set_u32(nvsHandle,"udpPORT",udpPORT));                      /* 写入udpPORT  */ 
    ESP_ERROR_CHECK(nvs_set_str(nvsHandle,"connectState",(char *)wifiConnectState));/* 写入wifiConnectState  */

    err = nvs_commit(nvsHandle); /* 确认数据写入 */
    ESP_ERROR_CHECK( err );
    nvs_close(nvsHandle);
    /*******************************************************************************************************/
}
/*wifi连接*/

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

#define EXAMPLE_ESP_MAXIMUM_RETRY  2

#define CONFIG_ESP_WIFI_AUTH_OPEN 1

#if CONFIG_ESP_WIFI_AUTH_OPEN
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_OPEN
#elif CONFIG_ESP_WIFI_AUTH_WEP
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WEP
#elif CONFIG_ESP_WIFI_AUTH_WPA_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WAPI_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WAPI_PSK
#endif

static const char *wifi_sta = "wifi station";

static int s_retry_num = 0;


static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(wifi_sta, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(wifi_sta,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(wifi_sta, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(char *WIFI_Name,char *WIFI_PassWord,char *isConnected)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    ESP_ERROR_CHECK( nvs_flash_init() );

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            /* Setting a password implies station will connect to all security modes including WEP/WPA.
             * However these modes are deprecated and not advisable to be used. Incase your Access point
             * doesn't support WPA2, these mode can be enabled by commenting below line */
	     .threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,
        },
    };

    strcpy((char *)&wifi_config.sta.ssid,WIFI_Name);
    strcpy((char *)&wifi_config.sta.password,WIFI_PassWord);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(wifi_sta, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) 
    {
        ESP_LOGI(wifi_sta, "connected to ap SSID:%s password:%s",
                 WIFI_Name, WIFI_PassWord);
        *isConnected = 1;
        // appInit();
    
    } 
    else if (bits & WIFI_FAIL_BIT) 
    {
        ESP_LOGI(wifi_sta, "Failed to connect to SSID:%s, password:%s",
                 WIFI_Name, WIFI_PassWord);
        *isConnected = 0;
        //NvsWriteDataToFlash("","","");/*超出最大重连次数后，退出连接，清楚保存的连接信息，重启*/
        // esp_restart();
    } 
    else 
    {
        ESP_LOGE(wifi_sta, "UNEXPECTED EVENT");
    }
}