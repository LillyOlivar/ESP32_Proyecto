/**
 * @file kmeans.c
 * @brief Implementacion de la inferencia K-Means (K = 3).
 * @author Proyecto Enfasis II EI
 * @version 1.0.0
 * @date 2026-11-12
 */
#include <math.h>
#include <stddef.h>
#include "kmeans.h"

/** @brief Definicion de un cluster: id, etiqueta y centroide normalizado [Temp, RH]. */
typedef struct {
    int         id;                    /**< Identificador del cluster. */
    const char *label;                 /**< Etiqueta del estado. */
    float       centroid[NUM_FEATURES]; /**< Centroide en espacio normalizado [0, 1]. */
} cluster_t;

/** @name Parametros de normalizacion Min-Max
 *  Rangos esperados para el escalado a [0, 1]. @{ */
#define TEMP_MIN 0.0f
#define TEMP_MAX 50.0f
#define HUM_MIN  0.0f
#define HUM_MAX  100.0f
/** @} */

/**
 * @brief Centroides pre-entrenados (offline/Python o Edge Impulse), en espacio normalizado.
 * @note Se conservan los valores originales; residen en flash (const).
 */
static const cluster_t clusters[K_CLUSTERS] = {
    { .id = 0, .label = "Ambiente Fresco / Nocturno",         .centroid = {0.30f, 0.70f} }, /* ~15 C, 70 %RH */
    { .id = 1, .label = "Operación Confort / Normal",         .centroid = {0.48f, 0.50f} }, /* ~24 C, 50 %RH */
    { .id = 2, .label = "Alta Temp / Alerta Condensación",    .centroid = {0.76f, 0.85f} }  /* ~38 C, 85 %RH */
};

/**
 * @brief Escala un valor al rango [0, 1] (Min-Max), saturando fuera de rango.
 * @param[in] val Valor a normalizar.
 * @param[in] min Minimo esperado.
 * @param[in] max Maximo esperado (> min).
 * @return Valor normalizado en [0, 1].
 */
static inline float normalize(float val, float min, float max)
{
    if (val < min) return 0.0f;
    if (val > max) return 1.0f;
    return (val - min) / (max - min);
}

esp_err_t kmeans_predict(float temp, float humidity, kmeans_result_t *result)
{
    if (result == NULL || isnan(temp) || isnan(humidity)) {
        return ESP_ERR_INVALID_ARG;
    }

    result->cluster_id = -1;
    result->cluster_label = "Desconocido";
    result->min_distance = 1e9f;

    /* 1. Normalizar para dar igual peso a Temp y Humedad */
    float norm_feat[NUM_FEATURES];
    norm_feat[0] = normalize(temp, TEMP_MIN, TEMP_MAX);
    norm_feat[1] = normalize(humidity, HUM_MIN, HUM_MAX);

    /* 2. Centroide mas cercano (distancia euclidiana al cuadrado, sin sqrtf en el bucle) */
    for (int i = 0; i < K_CLUSTERS; i++) {
        float d_temp = norm_feat[0] - clusters[i].centroid[0];
        float d_hum  = norm_feat[1] - clusters[i].centroid[1];
        float dist_sq = (d_temp * d_temp) + (d_hum * d_hum);

        if (dist_sq < result->min_distance) {
            result->min_distance = dist_sq;
            result->cluster_id = clusters[i].id;
            result->cluster_label = clusters[i].label;
        }
    }

    /* 3. Distancia euclidiana real como metrica final */
    result->min_distance = sqrtf(result->min_distance);
    return ESP_OK;
}
