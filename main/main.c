#include "sys_info.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_log.h"

static const char* TAG = "main";

static void heartbeat_task(void* arg)
{
    for(;;){
        ESP_LOGI(TAG, "heartbeat, uptime = %lld ms | free stack = %u bytes", 
            (long long)esp_timer_get_time()/1000,
            (unsigned int)uxTaskGetStackHighWaterMark(NULL));
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void worker_task(void* arg)
{
    for(;;){
        ESP_LOGI(TAG, "working on core %d", xPortGetCoreID());
        vTaskDelay(pdMS_TO_TICKS(300));
    }
}

void app_main(void)
{
    sys_info_print();
    xTaskCreate(heartbeat_task, "heartbeat_task", 4096, NULL, 1, NULL);
    xTaskCreate(worker_task, "worker_task", 4096, NULL, 5, NULL);

    ESP_LOGI(TAG, "app_main finished");
}
