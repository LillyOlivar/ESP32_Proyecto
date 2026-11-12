/**
 * @file ota_client.h
 * @brief Cliente OTA (Over-The-Air) por HTTPS para ESP32 con ESP-IDF.
 * @author Proyecto Enfasis II EI
 * @version 1.0.0
 * @date 2026-11-12
 *
 * Descarga un firmware desde un servidor HTTPS, lo valida y lo escribe en la
 * particion OTA inactiva (ota_0 / ota_1) mediante la API esp_https_ota.
 */
#ifndef OTA_CLIENT_H
#define OTA_CLIENT_H

#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Configuracion del cliente OTA. */
typedef struct {
    const char *url;          /**< URL HTTPS del binario (p. ej. https://IP:8070/firmware_v2.bin). */
    const char *cert_pem;     /**< Certificado del servidor en PEM, terminado en NUL. */
    int         timeout_ms;   /**< Timeout HTTP en milisegundos (> 0). */
    bool        skip_same_version; /**< true: no actualiza si la version del binario es igual a la instalada. */
} ota_client_config_t;

/**
 * @brief Registra en consola la version y la particion en ejecucion.
 * @note Util para comprobar que la actualizacion cambio de particion.
 */
void ota_client_log_running_info(void);

/**
 * @brief Descarga, valida e instala un firmware (bloqueante).
 *
 * Pasos: conexion TLS, lectura del descriptor de la imagen, comparacion de
 * version, descarga a la particion inactiva, verificacion de la imagen y
 * seleccion de la nueva particion de arranque. No reinicia el dispositivo.
 *
 * @param[in] cfg Configuracion (no NULL; url y cert_pem no NULL; timeout_ms > 0).
 *
 * @return ESP_OK si la imagen quedo instalada y lista para arrancar;
 *         ESP_ERR_INVALID_ARG si la configuracion es invalida;
 *         ESP_ERR_INVALID_VERSION si la version es igual y se pidio omitirla;
 *         ESP_ERR_INVALID_SIZE si la descarga quedo incompleta;
 *         otro codigo esp_err_t si falla la conexion, la descarga o la validacion.
 *
 * @warning Consume varios KB de pila; llamar desde una tarea con >= 8192 bytes.
 */
esp_err_t ota_client_run(const ota_client_config_t *cfg);

/**
 * @brief Crea una tarea que ejecuta ota_client_run() y reinicia si tiene exito.
 *
 * @param[in] cfg Configuracion; se copia internamente, pero los punteros url y
 *                cert_pem deben seguir validos mientras la tarea se ejecute.
 *
 * @return ESP_OK si la tarea fue creada; ESP_ERR_INVALID_ARG o ESP_ERR_INVALID_STATE
 *         (ya en ejecucion); ESP_ERR_NO_MEM si no se pudo crear la tarea.
 *
 * @note Si falla la actualizacion, el dispositivo sigue con el firmware actual.
 */
esp_err_t ota_client_start_task(const ota_client_config_t *cfg);

#ifdef __cplusplus
}
#endif
#endif /* OTA_CLIENT_H */
