#include "board_config.h"
#include "sys_info.h"
#include "event_bus.h"
#include "button_driver.h"
#include "i2c_bus.h"
#include "time_service.h"
#include "rtc_driver.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_timer.h"
#include "esp_log.h"

static const char* TAG = "main";

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
                    .tm_mday = 27,
                    .tm_hour = 12,
                    .tm_min = 0,
                    .tm_sec = 0
                };
                esp_err_t err = time_service_set_local(&t);

                if(err != ESP_OK)
                    ESP_LOGE(TAG, "[ui] Failed to set time: %s", esp_err_to_name(err));
                else
                    ESP_LOGI(TAG, "[ui] Time set to 12:00:00  27.11.2026");
            }
            else if(evt.type == EVT_TIME_TICK)
            {
                struct tm t = {0};

                if(time_service_get_local(&t) == ESP_OK)
                {
                    char buf[32];
                    strftime(buf, sizeof(buf), "%H:%M:%S  %d.%m.%Y", &t);
                    ESP_LOGI(TAG, "[ui] %s", buf);
                }
                else
                {
                    ESP_LOGW(TAG, "[ui] Failed to get local time");
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

    QueueHandle_t ui_q = event_bus_subscribe("ui", 8);
    QueueHandle_t power_q = event_bus_subscribe("power", 8);

    xTaskCreate(ui_task, "ui_task", 4096, ui_q, 4, NULL);
    xTaskCreate(power_task, "power_task", 4096, power_q, 5, NULL);

    ESP_ERROR_CHECK(i2c_bus_init(PIN_I2C_SDA, PIN_I2C_SCL));
    i2c_bus_scan();
    ESP_ERROR_CHECK(rtc_driver_init());
    ESP_ERROR_CHECK(time_service_init());
    ESP_ERROR_CHECK(button_driver_init(PIN_BUTTON));

    event_bus_post(EVT_BOOT_DONE, 0);
}
