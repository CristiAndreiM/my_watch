idf_component_register(
                        SRCS "time_service.c"
                        INCLUDE_DIRS "include"
                        PRIV_REQUIRES rtc_driver event_bus esp_timer log
                    )