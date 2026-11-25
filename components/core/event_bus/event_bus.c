#include "event_bus.h"
#include "freertos/semphr.h"
#include "esp_timer.h"
#include "esp_log.h"
#include <string.h>

#define MAX_SUBSCRIBERS 8

static const char* TAG = "event_bus";
static SemaphoreHandle_t s_lock = NULL;

typedef struct
{
    const char* name;
    QueueHandle_t queue;
}subscriber_t;

static subscriber_t subscribers[MAX_SUBSCRIBERS];
static int subscriber_count = 0;

void event_bus_init(void)
{
    s_lock = xSemaphoreCreateMutex();               //create the key
    subscriber_count = 0;
    memset(subscribers, 0, sizeof(subscribers));
}

QueueHandle_t event_bus_subscribe(const char* name, UBaseType_t depth)
{
    xSemaphoreTake(s_lock, portMAX_DELAY);          //It took the key

    if(subscriber_count >= MAX_SUBSCRIBERS)
    {
        ESP_LOGE(TAG, "Failed -> Queue full!");
        xSemaphoreGive(s_lock);                     //Gave the key
        return NULL;
    }

    QueueHandle_t q = xQueueCreate(depth, sizeof(app_event_t));

    if(q == NULL)
        {
            xSemaphoreGive(s_lock);
            return NULL;
        }
    
    subscribers[subscriber_count].name = name;
    subscribers[subscriber_count].queue = q;
    subscriber_count++;

    xSemaphoreGive(s_lock);

    return q;
}

void event_bus_post(app_event_type_t type, int32_t value)
{
    xSemaphoreTake(s_lock, portMAX_DELAY);
    app_event_t evt;
    evt.type = type;
    evt.value = value;
    evt.timestamp_ms = (uint32_t)(esp_timer_get_time() / 1000);

    for(int i = 0; i < subscriber_count; i++)
    {
        if(xQueueSend(subscribers[i].queue, &evt, 0) != pdTRUE)
            ESP_LOGW(TAG, "Queue full for %s", subscribers[i].name);
    }

    xSemaphoreGive(s_lock);
}

const char* event_name(app_event_type_t type)
{
    switch (type)
    {
    case EVT_BOOT_DONE:
        return "BOOT_DONE";
        break;
    case EVT_BUTTON_SHORT:
        return "EVT_BUTTON_SHORT";
        break;
    case EVT_BUTTON_LONG:
        return "EVT_BUTTON_LONG";
        break;
    case EVT_TIME_TICK:
        return "EVT_TIME_TICK";
        break;
    case EVT_COUNT:
        return "EVT_COUNT";
        break;
    default:
        return "UNKNOWN";
        break;
    }
}