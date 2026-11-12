/**
 * @file wifi_sta.h
 * @brief Conexion Wi-Fi en modo estacion (STA) para el cliente OTA.
 * @author Proyecto Enfasis II EI
 * @version 1.0.0
 * @date 2026-11-12
 */
#ifndef WIFI_STA_H
#define WIFI_STA_H

#include "esp_err.h"
#include "freertos/FreeRTOS.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Conecta el ESP32 a una red Wi-Fi y espera a obtener direccion IP.
 *
 * @param[in] ssid        SSID de la red (no NULL ni vacio).
 * @param[in] password    Contrasena (NULL o "" para red abierta).
 * @param[in] max_retry   Reintentos maximos tras una desconexion.
 * @param[in] timeout     Tiempo maximo de espera en ticks de FreeRTOS.
 *
 * @return ESP_OK si se obtuvo IP; ESP_ERR_INVALID_ARG si el SSID es invalido;
 *         ESP_FAIL si se agotaron los reintentos; ESP_ERR_TIMEOUT si vencio el tiempo.
 *
 * @note Requiere haber llamado antes a nvs_flash_init(), esp_netif_init()
 *       y esp_event_loop_create_default().
 * @warning Debe llamarse una sola vez.
 */
esp_err_t wifi_sta_connect(const char *ssid, const char *password,
                           int max_retry, TickType_t timeout);

#ifdef __cplusplus
}
#endif
#endif /* WIFI_STA_H */
