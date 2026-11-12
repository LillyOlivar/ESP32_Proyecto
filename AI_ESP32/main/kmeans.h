/**
 * @file kmeans.h
 * @brief Clasificacion ambiental con K-Means (K = 3) sobre temperatura y humedad.
 * @author Proyecto Enfasis II EI
 * @version 1.0.0
 * @date 2026-11-12
 *
 * Inferencia con centroides pre-entrenados (espacio normalizado Min-Max [0,1]).
 * Logica tomada de kmeans.c original, sin cambios en centroides ni etiquetas.
 */
#ifndef KMEANS_H
#define KMEANS_H

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Numero de grupos/estados a identificar. */
#define K_CLUSTERS   3
/** @brief Numero de variables: [0] temperatura (C), [1] humedad (%). */
#define NUM_FEATURES 2

/** @brief Resultado de la inferencia K-Means. */
typedef struct {
    int         cluster_id;    /**< ID del cluster (0, 1 o 2); -1 si no se asigno. */
    const char *cluster_label; /**< Etiqueta del estado ambiental. */
    float       min_distance;  /**< Distancia euclidiana al centroide asignado (espacio normalizado). */
} kmeans_result_t;

/**
 * @brief Clasifica una muestra [Temp, RH] en el cluster mas cercano.
 *
 * Normaliza con Min-Max (T: 0-50 C, RH: 0-100 %), busca el centroide con
 * menor distancia euclidiana cuadratica y devuelve la raiz como distancia final.
 *
 * @param[in]  temp     Temperatura en grados Celsius (no NaN).
 * @param[in]  humidity Humedad relativa en % (no NaN).
 * @param[out] result   Resultado de la clasificacion (no NULL).
 *
 * @return ESP_OK; ESP_ERR_INVALID_ARG si result es NULL o una entrada es NaN.
 *
 * @note Valores fuera del rango de normalizacion se saturan a 0 o 1.
 */
esp_err_t kmeans_predict(float temp, float humidity, kmeans_result_t *result);

#ifdef __cplusplus
}
#endif
#endif /* KMEANS_H */
