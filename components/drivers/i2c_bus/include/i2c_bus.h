#pragma once
#include "esp_err.h"
#include "driver/i2c_master.h"

esp_err_t               i2c_bus_init(gpio_num_t sda, gpio_num_t scl);
i2c_master_bus_handle_t i2c_bus_handle(void);   //for the device drivers
void                    i2c_bus_scan(void);     //log every address that answer