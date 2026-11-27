#include "time_service.h"
#include "rtc_driver.h"
#include "event_bus.h"
#include "esp_timer.h"
#include "esp_log.h"
#include <sys/time.h>
#include <stdlib.h>

static const char* TAG = "time_service.c";
static int32_t s_tick_count = 0;

static void tick_timer_cb(void *arg)
{
    event_bus_post(EVT_TIME_TICK, s_tick_count++);
}

esp_err_t time_service_init(void)
{
    struct tm rtc_time = {0};
    esp_err_t err = rtc_get_time(&rtc_time);
    if(err == ESP_OK)
    {
        setenv("TZ", "UTC0", 1);
        tzset();
        rtc_time.tm_isdst = 0;
        time_t epoch = mktime(&rtc_time);
        
        if(epoch != (time_t)-1)
        {
            struct timeval tv = { .tv_sec = epoch };
            settimeofday(&tv, NULL); 
        }
    }
    else
    {
        ESP_LOGW(TAG, "Failed to read RTC (%s), continuing without sync", esp_err_to_name(err));
    }
    setenv("TZ", "EET-2EEST,M3.5.0/3,M10.5.0/4", 1);
    tzset();

    const esp_timer_create_args_t timer_args = {
        .callback = tick_timer_cb,
        .name = "time_tick",
    };
    esp_timer_handle_t timer = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &timer));
    return esp_timer_start_periodic(timer, 1000000);
}

esp_err_t time_service_get_local(struct tm *out)
{
    if(out == NULL)
        return ESP_ERR_INVALID_ARG;
    
    time_t now = time(NULL);
    if(localtime_r(&now, out) == NULL)
        return ESP_FAIL;

    return ESP_OK;
}

esp_err_t time_service_set_local(const struct tm *in)
{
    if(in == NULL)
        return ESP_ERR_INVALID_ARG;
    
    struct tm local_copy = *in;
    local_copy.tm_isdst = -1;

    time_t epoch = mktime(&local_copy);
    if(epoch == (time_t)-1)
        return ESP_ERR_INVALID_ARG;

    struct tm utc_tm = {0};
    if(gmtime_r(&epoch, &utc_tm) == NULL)
        return ESP_FAIL;
    
    esp_err_t err = rtc_set_time(&utc_tm);
    if(err != ESP_OK)
        return err;
    
    struct timeval tv = { .tv_sec = epoch };
    settimeofday(&tv, NULL);
    return ESP_OK;
}