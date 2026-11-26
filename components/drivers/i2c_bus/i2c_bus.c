#include "i2c_bus.h"
#include "esp_log.h"
#include "esp_timer.h"

#define I2C_SCAN_START_ADDR  0x01
#define I2C_SCAN_END_ADDR    0x7E
#define I2C_PROBE_TIMEOUT_MS 50

static const char *TAG = "i2c_bus";
static i2c_master_bus_handle_t s_bus = NULL;

i2c_master_bus_handle_t i2c_bus_handle(void)
{
    return s_bus;
}

esp_err_t i2c_bus_init(gpio_num_t sda, gpio_num_t scl)
{
        if (s_bus != NULL) return ESP_ERR_INVALID_STATE;
        
       i2c_master_bus_config_t cfg = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = sda,
        .scl_io_num = scl,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
       };
       esp_err_t err = i2c_new_master_bus(&cfg, &s_bus);

       if(err != ESP_OK)
            return err;
        
       return ESP_OK;
}

void i2c_bus_scan(void)
{
    if(s_bus == NULL)
    {
        ESP_LOGW(TAG, "Bus not initialized");
        return;
    }

    int count = 0;
    int64_t start_us = esp_timer_get_time();

    for(uint16_t addr = I2C_SCAN_START_ADDR; addr <= I2C_SCAN_END_ADDR; addr++)
    {
        esp_err_t err = i2c_master_probe(s_bus, addr, I2C_PROBE_TIMEOUT_MS);

        if(err == ESP_OK)
        {
            ESP_LOGI(TAG, "found device at 0x%02X", addr);
            count++;
        }
    }

    int64_t elapsed_ms = (esp_timer_get_time() - start_us) / 1000;
    ESP_LOGI(TAG, "scan done: %d device(s) in %lld ms", count, elapsed_ms);
}