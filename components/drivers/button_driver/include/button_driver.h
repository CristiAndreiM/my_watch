#pragma once
#include "esp_err.h"
#include "driver/gpio.h"

esp_err_t button_driver_init(gpio_num_t pin);