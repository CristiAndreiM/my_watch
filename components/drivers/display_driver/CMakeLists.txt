idf_component_register(
    SRCS "display_ili9341.c"
    INCLUDE_DIRS "include"
    PRIV_REQUIRES board_config esp_driver_spi esp_driver_gpio esp_lcd esp_lcd_ili9341 freertos log
)