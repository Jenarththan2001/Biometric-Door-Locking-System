idf_component_register(SRCS "oled.c"
                            "ssd1306.c"
                            "ssd1306_i2c_legacy.c"
                       INCLUDE_DIRS "."
                       REQUIRES driver)
