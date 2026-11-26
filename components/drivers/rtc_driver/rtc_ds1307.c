#include "rtc_driver.h"
#include "i2c_bus.h"
#include "esp_log.h"
#include "esp_timer.h"

#define DEVICE_ADDR  0x68

static const char* TAG = "rtc_ds1307";
static i2c_master_dev_handle_t s_dev = NULL;

static uint8_t bcd_to_dec(uint8_t v) { return (v >> 4) * 10 + (v & 0x0F); }
static uint8_t dec_to_bcd(uint8_t d) { return ((d / 10) << 4) | (d % 10); }

esp_err_t rtc_driver_init(void)
{
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address =   DEVICE_ADDR,
        .scl_speed_hz = 100000
    };
    i2c_master_bus_add_device(i2c_bus_handle(), &dev_cfg, &s_dev);

    uint8_t sec = 0;
    uint8_t reg = 0x00;
    i2c_master_transmit_receive(s_dev, &reg, 1, &sec, 1, 100);

    if(sec & 0x80)
    {
        uint8_t data[2] = { 0x00, sec & 0x7F};
        esp_err_t err = i2c_master_transmit(s_dev, data, 2, 100);
        if(err != ESP_OK)
            return err;
    }
    
    ESP_LOGI(TAG, "DS1307 initialized");
    return ESP_OK;
}

esp_err_t rtc_get_time(struct tm* out)
{
    if(s_dev == NULL || out == NULL)
        return ESP_ERR_INVALID_STATE;

    uint8_t reg = 0x00, buf[7];   
    esp_err_t err = i2c_master_transmit_receive(s_dev, &reg, 1, buf, 7, 100);
    if(err != ESP_OK)
        return err;
    
    out->tm_sec = bcd_to_dec(buf[0] & 0x7F);
    out->tm_min = bcd_to_dec(buf[1]);
    out->tm_hour = bcd_to_dec(buf[2] & 0x3F);
    out->tm_wday = bcd_to_dec(buf[3]) - 1;
    out->tm_mday = bcd_to_dec(buf[4]);
    out->tm_mon = bcd_to_dec(buf[5]) - 1;
    out->tm_year = bcd_to_dec(buf[6]) + 100;
    return ESP_OK;
}

esp_err_t rtc_set_time(const struct tm* in)
{
    uint8_t buf[8];

    buf[0] = 0x00;
    buf[1] = dec_to_bcd(in->tm_sec);
    buf[2] = dec_to_bcd(in->tm_min);
    buf[3] = dec_to_bcd(in->tm_hour);
    buf[4] = dec_to_bcd(in->tm_wday + 1);
    buf[5] = dec_to_bcd(in->tm_mday);
    buf[6] = dec_to_bcd(in->tm_mon + 1);
    buf[7] = dec_to_bcd(in->tm_year - 100);

    return i2c_master_transmit(s_dev, buf, 8, 100);
}