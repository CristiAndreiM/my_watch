#include "sys_info.h"
#include "event_bus.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_timer.h"
#include "esp_log.h"

static const char* TAG = "main";

static void worker_task(void* arg)
{ 
    int32_t tick_count = 0;

    for(;;)
    {
        event_bus_post(EVT_TIME_TICK, tick_count++);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void ui_task(void* arg)
{
    QueueHandle_t q = event_bus_subscribe("ui", 8);
    configASSERT(q);                                //halts the execusion with an explicit message if q == NULL
    app_event_t evt;

    for(;;)
    {
        if(xQueueReceive(q, &evt, portMAX_DELAY) == pdTRUE){
                ESP_LOGI(TAG, "[ui] got %s (val=%ld, t=%lu ms)", event_name(evt.type), (long)evt.value, (unsigned long)evt.timestamp_ms);
        }
    }
}

static void power_task(void* arg){
    QueueHandle_t q = event_bus_subscribe("power", 8);
    configASSERT(q);
    app_event_t evt;

    for(;;)
    {
        if(xQueueReceive(q, &evt, portMAX_DELAY) == pdTRUE)
        {
            ESP_LOGI(TAG, "[power] got %s (val=%ld, t=%lu ms)",
                event_name(evt.type), (long)evt.value, (unsigned long)evt.timestamp_ms);
        }
    }
}

void app_main(void)
{
    event_bus_init();
    sys_info_print();

    xTaskCreate(ui_task, "ui_task", 4096, NULL, 4, NULL);
    xTaskCreate(power_task, "power_task", 4096, NULL, 5, NULL);
    xTaskCreate(worker_task, "worker_task", 4096, NULL, 5, NULL);

    event_bus_post(EVT_BOOT_DONE, 0);
}
