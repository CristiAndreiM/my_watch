idf_component_register( SRCS "main.c"
                        INCLUDE_DIRS "."
                        REQUIRES sys_info event_bus button_driver esp_timer log)