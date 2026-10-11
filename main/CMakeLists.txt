idf_component_register(
    SRCS "app_main.c"
    INCLUDE_DIRS "."
    REQUIRES app_config wifi_manager net_time onenet app_telemetry
)
