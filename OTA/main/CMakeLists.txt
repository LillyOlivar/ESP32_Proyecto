# El certificado debe existir antes de compilar (ver server/gen_cert.sh)
if(NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/certs/server.crt")
    message(FATAL_ERROR "Falta main/certs/server.crt. Ejecute server/gen_cert.sh <IP_DEL_PC> antes de compilar.")
endif()

# EMBED_TXTFILES equivale al target_add_binary_data(... TEXT) de la guia:
# genera el simbolo _binary_server_crt_start (terminado en NUL).
idf_component_register(
    SRCS "main.c" "ota_client.c" "wifi_sta.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES esp_https_ota esp_http_client app_update esp_wifi esp_event esp_netif nvs_flash esp_system
    EMBED_TXTFILES "certs/server.crt"
)

