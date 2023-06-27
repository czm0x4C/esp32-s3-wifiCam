#include "httpServer.h"

static const char *TAG = "httpServer";

extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[]   asm("_binary_index_html_end");

unsigned char wifiName[50];
unsigned char wifiPassword[50];

/* An HTTP POST handler */
static esp_err_t connectWIFI_post_handler(httpd_req_t *req)
{
    char buf[100];
    int ret, remaining = req->content_len;

    while (remaining > 0) {
        /* Read the data for the request */
        if ((ret = httpd_req_recv(req, buf,
                        MIN(remaining, sizeof(buf)))) <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                /* Retry receiving if timeout occurred */
                continue;
            }
            return ESP_FAIL;
        }

        /* Send back the same data */
        httpd_resp_send_chunk(req, buf, ret);
        remaining -= ret;

        /* Log data received */
        ESP_LOGI(TAG, "=========== RECEIVED DATA ==========");
        ESP_LOGI(TAG, "%.*s", ret, buf);
        ESP_LOGI(TAG, "====================================");

        char *tempString = strtok((char *)buf,",");
        unsigned char srtPlace = 0;/* 字段位置计数 */
        while(tempString)
        {
            /* 获取字段的字符串 */
            if(srtPlace == 0)                /* 获取wifi名称 */
            {
                strcpy((char *)wifiName,tempString);
            }
            else if(srtPlace == 1)                /* 获取wifi密码  */
            {
                strcpy((char *)wifiPassword,tempString);
            }
            else if(srtPlace == 2)                /* 获取udp服务器IP */
            {
                ESP_LOGI("httpServer", "IP = %s,len = %d", tempString,strlen((char *)tempString));
                memset(deviceAttributeInfo.UDP_serverIP,0,sizeof(deviceAttributeInfo.UDP_serverIP));
                strcpy((char *)deviceAttributeInfo.UDP_serverIP,tempString);
            }
            else if(srtPlace == 3)                /* 获取udp服务器端名 */
            {
                ESP_LOGI("httpServer", "PORT = %s,len = %d", tempString,strlen((char *)tempString));
                unsigned short tempPort = atoi(tempString);
                deviceAttributeInfo.UDP_serverPort = tempPort;
            }
            tempString = strtok(NULL,",");
            srtPlace++;
        }

        ESP_LOGI("httpServer", "wifiName = %s,len = %d", wifiName,strlen((char *)wifiName));
        ESP_LOGI("httpServer", "wifiPassword = %s,len = %d", wifiPassword,strlen((char *)wifiPassword));
        ESP_LOGI("httpServer", "IP = %s,len = %d",deviceAttributeInfo.UDP_serverIP,strlen((char *)deviceAttributeInfo.UDP_serverIP));
        ESP_LOGI("httpServer", "PORT = %d", deviceAttributeInfo.UDP_serverPort);
        writeWifiInfo(wifiName,wifiPassword,(unsigned char *)deviceAttributeInfo.UDP_serverIP,deviceAttributeInfo.UDP_serverPort,(unsigned char *)"ok");
        
    }
    // End response
    httpd_resp_send_chunk(req, NULL, 0);
    esp_restart();/* 重启设备 */
    return ESP_OK;
}

static const httpd_uri_t connectWIFI = {
    .uri       = "/connectWIFI",
    .method    = HTTP_POST,
    .handler   = connectWIFI_post_handler,
    .user_ctx  = NULL
};
/* This handler allows the custom error handling functionality to be
 * tested from client side. For that, when a PUT request 0 is sent to
 * URI /ctrl, the /hello and /echo URIs are unregistered and following
 * custom error handler http_404_error_handler() is registered.
 * Afterwards, when /hello or /echo is requested, this custom error
 * handler is invoked which, after sending an error message to client,
 * either closes the underlying socket (when requested URI is /echo)
 * or keeps it open (when requested URI is /hello). This allows the
 * client to infer if the custom error handler is functioning as expected
 * by observing the socket state.
 */
esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err)
{
    /* For any other URI send 404 and close socket */
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Some 404 error message");
    return ESP_FAIL;
}

static esp_err_t sendPage_get_handler(httpd_req_t *req)
{
    const size_t upload_script_size = (index_html_end - index_html_start);

    httpd_resp_send_chunk(req, (const char *)index_html_start, upload_script_size);

    return ESP_OK;
}

static const httpd_uri_t sendPage = {
    .uri       = "/index",
    .method    = HTTP_GET,
    .handler   = sendPage_get_handler,
    .user_ctx  = "sendPage"
};

httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &sendPage);/* 发送网页 */
        httpd_register_uri_handler(server, &connectWIFI);
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

static esp_err_t stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    return httpd_stop(server);
}

void disconnect_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server) {
        ESP_LOGI(TAG, "Stopping webserver");
        if (stop_webserver(*server) == ESP_OK) {
            *server = NULL;
        } else {
            ESP_LOGE(TAG, "Failed to stop http server");
        }
    }
}

void connect_handler(void* arg, esp_event_base_t event_base,
                            int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server == NULL) {
        ESP_LOGI(TAG, "Starting webserver");
        *server = start_webserver();
    }
}

