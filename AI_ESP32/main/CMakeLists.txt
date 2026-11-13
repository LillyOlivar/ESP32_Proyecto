idf_component_register(
    SRCS "main.c" "kmeans.c" "tinyml_monitor.c" "sht31.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES esp_driver_i2c esp_timer
)


