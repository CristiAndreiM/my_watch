idf_component_register( SRCS "button_driver.c"
                        INCLUDE_DIRS "include"
                        REQUIRES esp_driver_gpio
                        PRIV_REQUIRES esp_timer log event_bus)