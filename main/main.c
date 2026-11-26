#include "board_config.h"
#include "sys_info.h"
#include "event_bus.h"
#include "button_driver.h"
#include "i2c_bus.h"
#include "rtc_driver.h"
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
        struct tm now;
        char time_str[32];
        if(rtc_get_time(&now) == ESP_OK)
            {
                strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", &now);
                ESP_LOGI(TAG, "Time: %s", time_str);
            }
        
        event_bus_post(EVT_TIME_TICK, tick_count++);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void ui_task(void* arg)
{
    QueueHandle_t q = (QueueHandle_t)arg;
    app_event_t evt;

    for(;;)
    {
        if(xQueueReceive(q, &evt, portMAX_DELAY) == pdTRUE)
        {
            ESP_LOGI(TAG, "[ui] got %s (val=%ld, t=%lu ms)",
                     event_name(evt.type), (long)evt.value, (unsigned long)evt.timestamp_ms);
            
            if(evt.type == EVT_BUTTON_LONG)
            {
                struct tm t = {
                                .tm_year = 126,
                                .tm_mon = 10,
                                .tm_mday = 26,
                                .tm_hour = 12,
                                .tm_min = 0,
                                .tm_sec = 0,
                                .tm_wday = 4
                            };
                if(rtc_set_time(&t) == ESP_OK)
                {
                    ESP_LOGI(TAG, "[ui] RTC time set to 12:00:00");
                }
            }
        }
      
    }
}

static void power_task(void* arg){
    QueueHandle_t q = (QueueHandle_t)arg;
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

    ESP_ERROR_CHECK(button_driver_init(PIN_BUTTON));
    ESP_ERROR_CHECK(i2c_bus_init(PIN_I2C_SDA, PIN_I2C_SCL));
    i2c_bus_scan();
    ESP_ERROR_CHECK(rtc_driver_init());

    QueueHandle_t ui_q = event_bus_subscribe("ui", 8);
    QueueHandle_t power_q = event_bus_subscribe("power", 8);

    xTaskCreate(ui_task, "ui_task", 4096, ui_q, 4, NULL);
    xTaskCreate(power_task, "power_task", 4096, power_q, 5, NULL);

    event_bus_post(EVT_BOOT_DONE, 0);
    xTaskCreate(worker_task, "worker_task", 4096, NULL, 5, NULL);

   
    i2c_bus_scan();
}
