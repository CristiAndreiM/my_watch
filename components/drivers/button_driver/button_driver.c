#include "button_driver.h"
#include "event_bus.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_timer.h"
#include "esp_log.h"

static const char* TAG = "button_driver";
static QueueHandle_t s_edge_q = NULL;

static void IRAM_ATTR button_isr(void* arg)
{
    BaseType_t  woken = pdFALSE;
    uint32_t pin = (uint32_t)arg;
    xQueueSendFromISR(s_edge_q, &pin, &woken);
    portYIELD_FROM_ISR(woken);
}

//released = 1 | pressed = 0
static void button_task(void* arg)
{
    uint32_t pin;
    int64_t press_time_us = 0;
    bool is_pressed = false;

    for(;;)
    {
        if(xQueueReceive(s_edge_q, &pin, portMAX_DELAY) == pdTRUE)
        {
            vTaskDelay(pdMS_TO_TICKS(30));
            xQueueReset(s_edge_q);

            int level = gpio_get_level((gpio_num_t)pin);

            if(level == 0 && !is_pressed)
            {
                is_pressed = true;
                press_time_us = esp_timer_get_time();
            }
            else if(level == 1 && is_pressed)
            {
                is_pressed = false;
                int64_t duration_ms = (esp_timer_get_time() - press_time_us) / 1000;

                if(duration_ms >= 800)
                {
                    ESP_LOGI(TAG, "LONG press (%lld ms)", duration_ms);
                    event_bus_post(EVT_BUTTON_LONG, (uint32_t)duration_ms);
                }
                else
                {
                    ESP_LOGI(TAG, "SHORT press (%lld ms)", duration_ms);
                    event_bus_post(EVT_BUTTON_SHORT, (uint32_t)duration_ms);
                }
            }
        }
    }
}

esp_err_t button_driver_init(gpio_num_t pin)
{
    s_edge_q = xQueueCreate(10, sizeof(uint32_t));
    if(s_edge_q == NULL)
    {
        return ESP_ERR_NO_MEM;
    }

    BaseType_t ok = xTaskCreate(button_task, "button_task", 2048, NULL, 10, NULL);
    if(ok != pdPASS)
    {
        return ESP_FAIL;
    }

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << pin),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE,
    };

    ESP_ERROR_CHECK(gpio_config(&io_conf));

    esp_err_t err = gpio_install_isr_service(0);
    if(err != ESP_OK && err != ESP_ERR_INVALID_STATE)
    {
        return err;
    }

    return gpio_isr_handler_add(pin, button_isr, (void*)(uint32_t)pin);
}