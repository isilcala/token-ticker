idf_component_register(SRCS "config_store.c" "config_serial_protocol.c"
					INCLUDE_DIRS "."
					REQUIRES cjson domain fatfs nvs_flash sdmmc waveshare-s3-rlcd-4.2)