idf_component_register(
    SRCS "app_telemetry.c"
    INCLUDE_DIRS "."
    REQUIRES app_config sht30 onenet
)
