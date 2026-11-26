idf_component_register( SRCS "main.c"
                        INCLUDE_DIRS "."
                        REQUIRES sys_info event_bus button_driver i2c_bus esp_timer rtc_driver log)