#include "sys_info.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_timer.h"
#include "esp_log.h"

static const char* TAG = "main";
static QueueHandle_t data_queue;

static void worker_task(void* arg)
{ 
    int count = 0;
    for(;;){
        ESP_LOGI(TAG, "worker running...");

        if(xQueueSend(data_queue, &count, 0) != pdTRUE)
            ESP_LOGW(TAG, "Queue full! Dropped %d", count);

        if(count >= 10)
            vTaskDelete(NULL);
            
        count++;
        vTaskDelay(pdMS_TO_TICKS(300));
    }
}

static void ui_task(void* arg)
{
    int rx_val = 0;
    for(;;){
        if(xQueueReceive(data_queue, &rx_val, pdMS_TO_TICKS(2000)) == pdTRUE){
            ESP_LOGI(TAG, "value: %d", rx_val);
        }
        else{
            ESP_LOGI(TAG, "timeout");
        }
    }
}

void app_main(void)
{
    data_queue = xQueueCreate(5, sizeof(int));

    sys_info_print();

    xTaskCreate(worker_task, "worker_task", 4096, NULL, 5, NULL);
    xTaskCreate(ui_task, "ui_task", 4096, NULL, 4, NULL);

    ESP_LOGI(TAG, "app_main finished");
}
