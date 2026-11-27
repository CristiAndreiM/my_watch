#pragma once
#include <time.h>
#include "esp_err.h"

esp_err_t time_service_init(void);                      // RTC -> system clock, TZ, start the the tick
esp_err_t time_service_get_local(struct tm *out);       // current LOCAL time
esp_err_t time_service_set_local(const struct tm *in);  // set from LOCAL time -> system + RTC (as UTC)