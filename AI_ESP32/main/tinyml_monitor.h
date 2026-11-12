/**
 * @file tinyml_monitor.h
 * @brief Monitor TinyML: deteccion de anomalias con Z-Score bivariado y distancia euclidiana.
 * @author Proyecto Enfasis II EI
 * @version 1.0.0
 * @date 2026-11-12
 *
 * @note Este modulo NO implementa Random Forest ni Isolation Forest: usa
 *       estandarizacion Z-Score de (T, RH) y distancia euclidiana (L2) como
 *       score de anomalia. Incluye punto de rocio (Magnus) y VPD.
 */
#ifndef TINYML_MONITOR_H
#define TINYML_MONITOR_H

#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Resultado del procesamiento de una lectura. */
typedef struct {
    float temp_c;        /**< Temperatura de entrada (C). */
    float humidity_pct;  /**< Humedad relativa de entrada (%). */
    float dew_point_c;   /**< Punto de rocio, formula de Magnus (C). */
    float vpd_kpa;       /**< Deficit de presion de vapor (kPa). */
    float anomaly_score; /**< Distancia euclidiana en espacio Z-Score. */
    bool  is_anomaly;    /**< true si anomaly_score supera ANOMALY_THRESHOLD. */
} sensor_insights_t;

/** @name Parametros del baseline de entrenamiento @{ */
#define TRAIN_TEMP_MEAN 25.0f  /**< Media de temperatura (C). */
#define TRAIN_TEMP_STD  5.0f   /**< Desviacion estandar de temperatura (C). */
#define TRAIN_HUM_MEAN  60.0f  /**< Media de humedad (%). */
#define TRAIN_HUM_STD   15.0f  /**< Desviacion estandar de humedad (%). */
/** @} */

/** @brief Umbral del score para declarar una lectura atipica. */
#define ANOMALY_THRESHOLD 2.5f

/**
 * @brief Procesa una lectura: punto de rocio, VPD y deteccion de anomalia.
 *
 * @param[in]  temp_c        Temperatura en C (finita, entre -40 y 125, rango del SHT31).
 * @param[in]  humidity_pct  Humedad relativa en % (finita, 0 < RH <= 100).
 * @param[out] out           Resultado (no NULL).
 *
 * @return ESP_OK; ESP_ERR_INVALID_ARG si out es NULL o las entradas estan fuera de rango
 *         (RH = 0 haria indefinido el logaritmo del punto de rocio).
 *
 * @note Cuando is_anomaly es true, la aplicacion puede decidir marcar el evento
 *       o aumentar el muestreo; esa logica y la escritura en MicroSD NO estan
 *       implementadas en este modulo.
 */
esp_err_t tinyml_monitor_process(float temp_c, float humidity_pct, sensor_insights_t *out);

#ifdef __cplusplus
}
#endif
#endif /* TINYML_MONITOR_H */
