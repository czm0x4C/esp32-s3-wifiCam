#include <stdio.h>
#include <nvs_flash.h>
void app_main(void)
{
    nvs_handle_t nvsHandle = {0}; 
    esp_err_t err = nvs_flash_init();
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
    ESP_ERROR_CHECK( err );
}
