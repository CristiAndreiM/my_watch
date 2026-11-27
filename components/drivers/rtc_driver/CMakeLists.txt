idf_component_register(
                        SRCS "rtc_ds1307.c"
                        INCLUDE_DIRS "include"
                        PRIV_REQUIRES i2c_bus esp_driver_i2c log)