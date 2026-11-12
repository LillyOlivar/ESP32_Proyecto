/**
 * @file main.c
 * @brief Aplicacion de ejemplo del modulo OTA: Wi-Fi + actualizacion por HTTPS.
 * @author Proyecto Enfasis II EI
 * @version 1.0.0
 * @date 2026-11-12
 */
#include "esp_log.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "sdkconfig.h"
#include "ota_client.h"
#include "wifi_sta.h"

static const char *TAG = "OTA_MAIN";

/** Certificado del servidor incrustado desde main/certs/server.crt (ver CMakeLists.txt). */
extern const uint8_t server_cert_pem_start[] asm("_binary_server_crt_start");

/** Tiempo maximo de espera de la conexion Wi-Fi (ms). */
#define WIFI_CONNECT_TIMEOUT_MS 30000

/**
 * @brief Punto de entrada: inicializa NVS y red, conecta Wi-Fi y lanza la OTA.
 */
void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ota_client_log_running_info();

    ret = wifi_sta_connect(CONFIG_OTA_WIFI_SSID, CONFIG_OTA_WIFI_PASSWORD,
                           CONFIG_OTA_WIFI_MAX_RETRY, pdMS_TO_TICKS(WIFI_CONNECT_TIMEOUT_MS));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Sin Wi-Fi no es posible la OTA (%s)", esp_err_to_name(ret));
        return;
    }

    const ota_client_config_t cfg = {
        .url = CONFIG_OTA_SERVER_URL,
        .cert_pem = (const char *)server_cert_pem_start,
        .timeout_ms = CONFIG_OTA_TIMEOUT_MS,
        .skip_same_version = CONFIG_OTA_SKIP_SAME_VERSION,
    };
    ret = ota_client_start_task(&cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "No se pudo iniciar la tarea OTA (%s)", esp_err_to_name(ret));
    }
}
