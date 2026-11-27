#pragma once
#include <stdint.h>
#include "esp_err.h"

#define RGB565(r, g, b)  ((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3))

esp_err_t display_init(void);
esp_err_t display_fill_rect(int x, int y, int w, int h, uint16_t color);
int       display_width(void);
int       display_height(void);