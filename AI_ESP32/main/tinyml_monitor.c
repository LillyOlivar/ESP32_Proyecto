/**
 * @file tinyml_monitor.c
 * @brief Implementacion del monitor TinyML (Z-Score bivariado + distancia euclidiana).
 * @author Proyecto Enfasis II EI
 * @version 1.0.0
 * @date 2026-11-12
 */
#include <math.h>
#include <stddef.h>
#include "tinyml_monitor.h"

/**
 * @brief Punto de rocio (C) con la formula de Magnus (a = 17.27, b = 237.7).
 * @param[in] temp Temperatura en C.
 * @param[in] rh   Humedad relativa en % (> 0).
 * @return Punto de rocio en C.
 */
static float calculate_dew_point(float temp, float rh)
{
    const float a = 17.27f;
    const float b = 237.7f;
    float alpha = ((a * temp) / (b + temp)) + logf(rh / 100.0f);
    return (b * alpha) / (a - alpha);
}

/**
 * @brief Deficit de presion de vapor (kPa) = SVP - AVP.
 * @param[in] temp Temperatura en C.
 * @param[in] rh   Humedad relativa en %.
 * @return VPD en kPa.
 */
static float calculate_vpd(float temp, float rh)
{
    float svp = 0.61078f * expf((17.27f * temp) / (temp + 237.3f)); /* presion de saturacion */
    float avp = svp * (rh / 100.0f);                                 /* presion actual */
    return svp - avp;
}

/**
 * @brief Score de anomalia: distancia euclidiana en el espacio Z-Score (T, RH).
 * @param[in] temp Temperatura en C.
 * @param[in] rh   Humedad relativa en %.
 * @return Score (>= 0).
 */
static float calculate_anomaly_score(float temp, float rh)
{
    float z_temp = (temp - TRAIN_TEMP_MEAN) / TRAIN_TEMP_STD;
    float z_hum  = (rh - TRAIN_HUM_MEAN) / TRAIN_HUM_STD;
    return sqrtf((z_temp * z_temp) + (z_hum * z_hum));
}

esp_err_t tinyml_monitor_process(float temp_c, float humidity_pct, sensor_insights_t *out)
{
    if (out == NULL || !isfinite(temp_c) || !isfinite(humidity_pct) ||
        temp_c < -40.0f || temp_c > 125.0f ||
        humidity_pct <= 0.0f || humidity_pct > 100.0f) {
        return ESP_ERR_INVALID_ARG;
    }

    out->temp_c = temp_c;
    out->humidity_pct = humidity_pct;

    /* 1. Fisica ambiental */
    out->dew_point_c = calculate_dew_point(temp_c, humidity_pct);
    out->vpd_kpa = calculate_vpd(temp_c, humidity_pct);

    /* 2. Inferencia de anomalia en el microcontrolador */
    out->anomaly_score = calculate_anomaly_score(temp_c, humidity_pct);
    out->is_anomaly = (out->anomaly_score > ANOMALY_THRESHOLD);

    return ESP_OK;
}
