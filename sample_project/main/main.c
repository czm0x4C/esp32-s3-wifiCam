#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"

#include "esp_system.h"
#include "esp_event.h"

#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_log.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"

#include "esp_camera.h"

#include "ringbuffer.h" 
/* 变量 */
/*tcp连接*/
// #define HOST_IP_ADDR "47.108.81.162"
// #define HOST_IP_ADDR "47.109.108.212"
#define HOST_IP_ADDR "192.168.3.10"
#define PORT 8000

/*UDP连接*/
static const char *UDP = "UDP";
#define UDP_SEND_MAX_LEN ( 60 * 1024 ) /* 设置UDP一包发送的最大数量 */
_RingBuffer CameraBuffer;/* 缓冲区 */
unsigned char CameraStartFlag = 0;/* 判断摄像头状态 */
/* 函数声明 */
void wifi_init_sta(char *WIFI_Name,char *WIFI_PassWord); /* 初始化WIFI */
static esp_err_t init_camera();     /* 初始化摄像头 */
/* 任务函数 */

void camera_task(void *pvParameters);       /* 摄像头数据采集任务 */

void app_main(void)
{
    if(init_camera() != ESP_OK)
    {
        CameraStartFlag = 1;
    }

    // RingBuffer_Init(&CameraBuffer); /* 初始化缓冲区 */

    // wifi_init_sta("Xiaomi_6A71","1234567890");/*初始化WIFI*/
    // wifi_init_sta("test","12345678");/*初始化WIFI*/
    wifi_init_sta("HUAWEI-ESLAB458","12345678"); /*初始化WIFI*/
}

/*wifi连接*/

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

#define EXAMPLE_ESP_MAXIMUM_RETRY  10

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


void wifi_init_sta(char *WIFI_Name,char *WIFI_PassWord)
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
    ESP_ERROR_CHECK(esp_wifi_start() );

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
       
        //xTaskCreate(tcp_client_task, "tcp_client", 1024 * 10, NULL, 6, NULL);/*TCP_client 连接TCP*/
        xTaskCreate(camera_task, "camera_task", 1024 * 10, NULL, 5, NULL); 
    } 
    else if (bits & WIFI_FAIL_BIT) 
    {
        ESP_LOGI(wifi_sta, "Failed to connect to SSID:%s, password:%s",
                 WIFI_Name, WIFI_PassWord);
        //NvsWriteDataToFlash("","","");/*超出最大重连次数后，退出连接，清楚保存的连接信息，重启*/
        esp_restart();
    } 
    else 
    {
        ESP_LOGE(wifi_sta, "UNEXPECTED EVENT");
    }
}




/*摄像头*/

#define CAM_PIN_PWDN 38
#define CAM_PIN_RESET 39 //software reset will be performed
#define CAM_PIN_XCLK 40
#define CAM_PIN_SIOD 3
#define CAM_PIN_SIOC 46

#define CAM_PIN_D7 47
#define CAM_PIN_D6 21
#define CAM_PIN_D5 14
#define CAM_PIN_D4 13
#define CAM_PIN_D3 12
#define CAM_PIN_D2 11
#define CAM_PIN_D1 10
#define CAM_PIN_D0 9
#define CAM_PIN_VSYNC 48
#define CAM_PIN_HREF 41
#define CAM_PIN_PCLK 45
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
    .xclk_freq_hz = 24000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_JPEG, //YUV422,GRAYSCALE,RGB565,JPEG
    .frame_size = FRAMESIZE_FHD,    //QQVGA-UXGA, For ESP32, do not use sizes above QVGA when not JPEG. The performance of the ESP32-S series has improved a lot, but JPEG mode always gives better frame rates.
    //FRAMESIZE_QVGA FRAMESIZE_UXGA
    .jpeg_quality = 10, //0-63, for OV series camera sensors, lower number means higher quality
    .fb_count = 2,       //When jpeg mode is used, if fb_count more than one, the driver will work in continuous mode.
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
};

static esp_err_t init_camera()
{
    //initialize the camera
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK)
    {
        ESP_LOGE(camera, "Camera Init Failed");
        return err;
    }

    return ESP_OK;
}


void camera_task(void *pvParameters)
{
    int addr_family = 0;
    int ip_protocol = 0;
    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = inet_addr(HOST_IP_ADDR);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(PORT);
    addr_family = AF_INET;
    ip_protocol = IPPROTO_IP;

    int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
    if (sock < 0) {
        ESP_LOGE(UDP, "Unable to create socket: errno %d", errno);
    }

    // Set timeout
    struct timeval timeout;
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);

    ESP_LOGI(UDP, "Socket created, sending to %s:%d", HOST_IP_ADDR, PORT);
    CameraStartFlag = 2;

    while(1)
    {
        /*非阻塞接收UDP数据*/
        static unsigned char rx_buffer[100];
        struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
        socklen_t socklen = sizeof(source_addr);
        int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1,MSG_DONTWAIT, (struct sockaddr *)&source_addr, &socklen);
        // Error occurred during receiving
        if (len > 0) 
        {
            rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
            ESP_LOGI(UDP, "Received %d bytes:", len);
            ESP_LOGI(UDP, "%s", rx_buffer);
        }

        if(CameraStartFlag == 2)
        {
            ESP_LOGI(camera, "Taking picture...");
            camera_fb_t *pic = esp_camera_fb_get();
            if( pic != NULL)
            {
                /* 使用 UDP 发送时，一个数据包大小不超过64K*/
                int preSendPicLen = pic->len;
                int sendCnt = 0;
                int err = 0;
                unsigned char * startPalce =  pic->buf;
                if(preSendPicLen > UDP_SEND_MAX_LEN)
                {
                    ESP_LOGI(camera,"preSendPicLen = %d \r\n",preSendPicLen);
                    while(UDP_SEND_MAX_LEN * sendCnt < preSendPicLen)
                    {
                        err = sendto(sock,startPalce + (UDP_SEND_MAX_LEN * sendCnt),UDP_SEND_MAX_LEN, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
                        if (err < 0) 
                        {
                            ESP_LOGE(UDP, "Error occurred during sending: errno %d", errno);
                            break;
                            
                        }
                        sendCnt++;
                        vTaskDelay(1);
                    }
                    sendCnt -= 1;
                    ESP_LOGI(camera,"sendCnt = %d \r\n",sendCnt);
                    ESP_LOGI(camera,"leave = %d \r\n",preSendPicLen % UDP_SEND_MAX_LEN);
                    err = sendto(sock,startPalce + (UDP_SEND_MAX_LEN * sendCnt),preSendPicLen % UDP_SEND_MAX_LEN, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
                    if (err < 0) 
                    {
                        ESP_LOGE(UDP, "Error occurred during sending: errno %d", errno);
                        break;
                    }
                    vTaskDelay(1);
                    sendCnt = 0;
                }
                else
                {
                    err = sendto(sock,startPalce,preSendPicLen, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
                    if (err < 0) 
                    {
                        ESP_LOGE(UDP, "Error occurred during sending: errno %d", errno);
                        break;
                        
                    }
                }
                // use pic->buf to access the image
                ESP_LOGI(camera, "Picture taken! Its size was: %zu bytes,width: %zu,height: %zu", pic->len,pic->width,pic->height);
                esp_camera_fb_return(pic);
            }


        }
        else
        {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }
}