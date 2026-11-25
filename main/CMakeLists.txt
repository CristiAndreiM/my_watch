idf_component_register( SRCS "main.c"
                        INCLUDE_DIRS "."
                        PRIV_REQUIRES sys_info event_bus esp_timer log)