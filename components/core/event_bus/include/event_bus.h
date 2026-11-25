#pragma once
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

typedef enum
{
    EVT_BOOT_DONE, 
    EVT_BUTTON_SHORT,
    EVT_BUTTON_LONG,
    EVT_TIME_TICK,
    EVT_COUNT
} app_event_type_t;

typedef struct {
    app_event_type_t type;
    uint32_t timestamp_ms;  //when it happened
    int32_t value;          //small optional data
} app_event_t;

void            event_bus_init(void);
QueueHandle_t   event_bus_subscribe(const char* name, UBaseType_t depth);
void            event_bus_post(app_event_type_t type, int32_t value);
const char*     event_name(app_event_type_t type);