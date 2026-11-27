idf_component_register( SRCS "main.c"
                        INCLUDE_DIRS "."
                        PRIV_REQUIRES board_config sys_info event_bus button_driver display_driver
                                 i2c_bus esp_timer rtc_driver time_service log)
