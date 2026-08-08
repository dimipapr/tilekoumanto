idf_component_register(
    SRCS 
        "app_main.c"
        "tk_comms.c"
    INCLUDE_DIRS "."
    REQUIRES
        esp_event
        esp_netif
        esp_wifi
        nvs_flash
)