#include "display_driver.h"
#include "board_config.h"
#include "driver/spi_master.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_ili9341.h"
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#define STRIP_LINES 20

static esp_lcd_panel_io_handle_t s_io = NULL;
static esp_lcd_panel_handle_t s_panel = NULL;
static SemaphoreHandle_t s_dma_done = NULL;
static uint16_t *s_buf = NULL;

static bool color_done_cb(esp_lcd_panel_io_handle_t io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    BaseType_t woken = pdFALSE;
    xSemaphoreGiveFromISR(s_dma_done, &woken);
    return (woken == pdTRUE);
}

esp_err_t display_init(void)
{
    s_dma_done = xSemaphoreCreateBinary();
    if(s_dma_done == NULL)
        return ESP_ERR_NO_MEM;
    xSemaphoreGive(s_dma_done);

    size_t buf_size = LCD_H_RES * STRIP_LINES * sizeof(uint16_t);
    s_buf = heap_caps_malloc(buf_size, MALLOC_CAP_DMA);
    if(s_buf == NULL)
        return ESP_ERR_NO_MEM;
    
    spi_bus_config_t bus = {
        .sclk_io_num = PIN_LCD_SCK,
        .mosi_io_num = PIN_LCD_MOSI,
        .miso_io_num = -1,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = buf_size,
    };
    esp_err_t err = spi_bus_initialize(SPI2_HOST, &bus, SPI_DMA_CH_AUTO);
    if(err != ESP_OK) return err;

    esp_lcd_panel_io_spi_config_t io_cfg = {
        .dc_gpio_num = PIN_LCD_D_C,
        .cs_gpio_num = PIN_LCD_CS,
        .pclk_hz = 40 * 1000 * 1000,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .spi_mode = 0,
        .trans_queue_depth = 10,
        .on_color_trans_done = color_done_cb,
    };
    err = esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI2_HOST, &io_cfg, &s_io);
    if(err != ESP_OK) return err;

    esp_lcd_panel_dev_config_t panel_cfg = {
        .reset_gpio_num = PIN_LCD_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR,
        .bits_per_pixel = 16,
    };
    err = esp_lcd_new_panel_ili9341(s_io, &panel_cfg, &s_panel);
    if(err != ESP_OK) return err;

    err = esp_lcd_panel_reset(s_panel);
    if(err != ESP_OK) return err;

    err = esp_lcd_panel_init(s_panel);
    if(err != ESP_OK) return err;
    return esp_lcd_panel_disp_on_off(s_panel, true);
}

esp_err_t display_fill_rect(int x, int y, int w, int h, uint16_t color)
{
    if(s_panel == NULL || s_dma_done == NULL || s_buf == NULL)
        return ESP_ERR_INVALID_STATE;
    
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > LCD_H_RES) { w = LCD_H_RES - x; }
    if (y + h > LCD_V_RES) { h = LCD_V_RES - y; }

    if (w <= 0 || h <= 0)
        return ESP_OK;
        
    uint16_t swapped = __builtin_bswap16(color);

    xSemaphoreTake(s_dma_done, portMAX_DELAY);

    int max_lines = (h < STRIP_LINES) ? h : STRIP_LINES;
    int pixels_per_strip = w * max_lines;
    for(int i = 0; i < pixels_per_strip; i++)
    {
        s_buf[i] = swapped;
    }

    xSemaphoreGive(s_dma_done);

    for(int y0 = y; y0 < y + h; y0 += STRIP_LINES)
    {
        int lines = (y + h - y0 < STRIP_LINES) ? (y + h - y0) : STRIP_LINES;

        xSemaphoreTake(s_dma_done, portMAX_DELAY);
        esp_err_t err = esp_lcd_panel_draw_bitmap(s_panel, x, y0, x + w, y0 + lines, s_buf);
        if(err != ESP_OK)
        {
            xSemaphoreGive(s_dma_done);
            return err;
        }
    }
    return ESP_OK;
}

int display_width(void) { return LCD_H_RES; }
int display_height(void) { return LCD_V_RES; }