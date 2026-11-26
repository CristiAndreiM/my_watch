#pragma once
#include <time.h>
#include "esp_err.h"

esp_err_t rtc_driver_init(void);
esp_err_t rtc_get_time(struct tm* out);
esp_err_t rtc_set_time(const struct tm* in); 