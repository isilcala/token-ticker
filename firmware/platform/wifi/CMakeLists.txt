idf_component_register(SRCS "wifi_platform.c"
					INCLUDE_DIRS "."
					REQUIRES domain esp_event esp_netif esp_wifi freertos)