idf_component_register(SRCS "display_port.c" "rlcd_driver.c"
					INCLUDE_DIRS "."
					REQUIRES esp_driver_gpio esp_driver_spi esp_lcd esp_timer heap lvgl waveshare-s3-rlcd-4.2)