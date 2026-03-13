idf_component_register(
    SRCS
        "blink_example_main.c"
        "led_control.c"
        "config_manager.c"
        "wifi_manager.c"
        "web_server.c"
        "app_runtime.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES nvs_flash esp_wifi esp_event esp_netif esp_http_server
)