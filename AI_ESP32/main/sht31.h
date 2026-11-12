/**
 * @file sht31.h
 * @brief Driver minimo I2C del sensor SHT31 (medicion unica) para ESP-IDF.
 * @author Proyecto Enfasis II EI
 * @version 1.0.0
 * @date 2026-11-12
 *
 * Usa la API driver/i2c_master.h (ESP-IDF >= 5.2).
 */
#ifndef SHT31_H
#define SHT31_H

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Inicializa el bus I2C y registra el SHT31.
 *
 * @param[in] sda_gpio GPIO de SDA (>= 0).
 * @param[in] scl_gpio GPIO de SCL (>= 0).
 * @param[in] addr     Direccion I2C de 7 bits (0x44 o 0x45).
 * @param[in] freq_hz  Frecuencia del bus en Hz (> 0).
 * @param[in] pullup   true para activar los pull-ups internos.
 *
 * @return ESP_OK; ESP_ERR_INVALID_ARG si los pines/parametros son invalidos;
 *         ESP_ERR_INVALID_STATE si ya estaba inicializado; otro error de driver.
 *
 * @note Los pines dependen del cableado del usuario; no se asumen valores.
 */
esp_err_t sht31_init(int sda_gpio, int scl_gpio, uint8_t addr, uint32_t freq_hz, bool pullup);

/**
 * @brief Realiza una medicion unica (alta repeticion, sin clock stretching).
 *
 * @param[out] temp_c       Temperatura en C (no NULL).
 * @param[out] humidity_pct Humedad relativa en % (no NULL).
 *
 * @return ESP_OK; ESP_ERR_INVALID_STATE si no esta inicializado;
 *         ESP_ERR_INVALID_ARG si un puntero es NULL;
 *         ESP_ERR_INVALID_CRC si falla el CRC; otro error de I2C.
 *
 * @note Bloquea ~20 ms durante la conversion.
 */
esp_err_t sht31_read(float *temp_c, float *humidity_pct);

/**
 * @brief Libera el dispositivo y el bus I2C.
 * @return ESP_OK o el error del driver.
 */
esp_err_t sht31_deinit(void);

#ifdef __cplusplus
}
#endif
#endif /* SHT31_H */
