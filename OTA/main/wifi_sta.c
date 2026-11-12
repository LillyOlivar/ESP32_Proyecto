/**
 * @file wifi_sta.c
 * @brief Implementacion de la conexion Wi-Fi STA.
 * @author Proyecto Enfasis II EI
 * @version 1.0.0
 * @date 2026-11-12
 */
#include <string.h>
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "freertos/event_groups.h"
#include "wifi_sta.h"

static const char *TAG = "WIFI_STA";

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

/** Grupo de eventos que senala conexion o fallo. */
static EventGroupHandle_t s_wifi_events;
/** Reintentos realizados desde la ultima conexion. */
static int s_retry_num;
/** Reintentos maximos permitidos. */
static int s_max_retry;

/**
 * @brief Manejador de eventos Wi-Fi e IP.
 * @param[in] arg        No utilizado.
 * @param[in] event_base WIFI_EVENT o IP_EVENT.
 * @param[in] event_id   Identificador del evento.
 * @param[in] event_data Datos del evento (no utilizado).
 */
static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    (void)arg;
    (void)event_data;

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < s_max_retry) {
            s_retry_num++;
            ESP_LOGW(TAG, "Reintentando conexion (%d/%d)...", s_retry_num, s_max_retry);
            esp_wifi_connect();
        } else {
            xEventGroupSetBits(s_wifi_events, WIFI_FAIL_BIT);
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_events, WIFI_CONNECTED_BIT);
    }
}

esp_err_t wifi_sta_connect(const char *ssid, const char *password,
                           int max_retry, TickType_t timeout)
{
    wifi_config_t wifi_cfg = { 0 };

    if (ssid == NULL || ssid[0] == '\0' || strlen(ssid) >= sizeof(wifi_cfg.sta.ssid)) {
        ESP_LOGE(TAG, "SSID invalido. Configure OTA_WIFI_SSID en menuconfig.");
        return ESP_ERR_INVALID_ARG;
    }
    const char *pass = (password != NULL) ? password : "";
    if (strlen(pass) >= sizeof(wifi_cfg.sta.password)) {
        return ESP_ERR_INVALID_ARG;
    }

    s_max_retry = max_retry;
    s_retry_num = 0;
    s_wifi_events = xEventGroupCreate();
    if (s_wifi_events == NULL) {
        return ESP_ERR_NO_MEM;
    }

    esp_netif_create_default_wifi_sta();
    wifi_init_config_t init_cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&init_cfg));

    esp_event_handler_instance_t h_wifi, h_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler, NULL, &h_wifi));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler, NULL, &h_ip));

    strlcpy((char *)wifi_cfg.sta.ssid, ssid, sizeof(wifi_cfg.sta.ssid));
    strlcpy((char *)wifi_cfg.sta.password, pass, sizeof(wifi_cfg.sta.password));
    wifi_cfg.sta.threshold.authmode = (pass[0] == '\0') ? WIFI_AUTH_OPEN : WIFI_AUTH_WPA2_PSK;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg));
    ESP_ERROR_CHECK(esp_wifi_start());

    EventBits_t bits = xEventGroupWaitBits(s_wifi_events, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE, pdFALSE, timeout);

    esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, h_ip);
    esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, h_wifi);

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Conectado a '%s'", ssid);
        return ESP_OK;
    }
    if (bits & WIFI_FAIL_BIT) {
        ESP_LOGE(TAG, "No se pudo conectar a '%s'", ssid);
        return ESP_FAIL;
    }
    ESP_LOGE(TAG, "Timeout esperando conexion Wi-Fi");
    return ESP_ERR_TIMEOUT;
}
