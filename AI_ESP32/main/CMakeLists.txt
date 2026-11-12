idf_component_register(
    SRCS "main.c" "kmeans.c" "tinyml_monitor.c" "sht31.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES driver esp_timer
)
