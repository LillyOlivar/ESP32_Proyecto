/**
 * @file main.c
 * @brief Aplicacion de integracion de IA: lee T/RH y ejecuta K-Means y TinyML Monitor.
 * @author Proyecto Enfasis II EI
 * @version 1.0.0
 * @date 2026-11-12
 *
 * @note Registro en MicroSD y muestreo adaptativo NO estan implementados;
 *       los resultados solo se muestran por la consola de ESP-IDF.
 */
#include <stddef.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "kmeans.h"
#include "tinyml_monitor.h"
#include "sht31.h"

static const char *TAG = "AI_ESP32";

#ifdef CONFIG_AI_USE_SIMULATED_SAMPLES
/** @brief Muestra simulada [T, RH]. */
typedef struct { float t; float rh; } sample_t;

/** @brief Muestras de prueba de los codigos originales (kmeans.c y randomforest.c). */
static const sample_t k_samples[] = {
    {16.2f, 72.0f}, {23.8f, 52.0f}, {37.5f, 88.0f}, {25.0f, 48.0f},   /* kmeans.c */
    {24.5f, 58.0f}, {25.1f, 62.0f}, {38.5f, 95.0f}, {22.0f, 15.0f}    /* randomforest.c */
};
#endif

/**
 * @brief Ejecuta ambos algoritmos sobre una lectura y la muestra por consola.
 * @param[in] idx Numero de muestra (desde 1).
 * @param[in] t   Temperatura en C.
 * @param[in] rh  Humedad relativa en %.
 */
static void run_inference(int idx, float t, float rh)
{
    kmeans_result_t km;
    sensor_insights_t ins;

    ESP_LOGI(TAG, "---------------------------------------------");
    ESP_LOGI(TAG, "Muestra [%d] -> T: %.2f °C | RH: %.2f %%", idx, t, rh);

    if (kmeans_predict(t, rh, &km) == ESP_OK) {
        ESP_LOGI(TAG, "[K-Means] Cluster: ID %d [%s] | Distancia: %.4f",
                 km.cluster_id, km.cluster_label, km.min_distance);
    } else {
        ESP_LOGE(TAG, "[K-Means] Entrada invalida");
    }

    if (tinyml_monitor_process(t, rh, &ins) == ESP_OK) {
        ESP_LOGI(TAG, "[TinyML] Rocío: %.2f °C | VPD: %.2f kPa", ins.dew_point_c, ins.vpd_kpa);
        ESP_LOGI(TAG, "[TinyML] Score: %.2f | Estado: %s", ins.anomaly_score,
                 ins.is_anomaly ? "ANOMALÍA DETECTADA" : "NORMAL");
        if (ins.is_anomaly) {
            ESP_LOGW(TAG, "-> Evento atípico (hook para MicroSD / muestreo adaptativo: pendiente de implementar)");
        }
    } else {
        ESP_LOGE(TAG, "[TinyML] Lectura fuera de rango");
    }
}

/**
 * @brief Punto de entrada: modo simulado (por defecto) o lectura real del SHT31.
 */
void app_main(void)
{
    ESP_LOGI(TAG, "Iniciando inferencia K-Means y TinyML Monitor...");

#ifdef CONFIG_AI_USE_SIMULATED_SAMPLES
    ESP_LOGW(TAG, "Modo simulado: no se usa el sensor SHT31");
    for (size_t i = 0; i < sizeof(k_samples) / sizeof(k_samples[0]); i++) {
        run_inference((int)i + 1, k_samples[i].t, k_samples[i].rh);
        vTaskDelay(pdMS_TO_TICKS(CONFIG_AI_SAMPLE_PERIOD_MS));
    }
    ESP_LOGI(TAG, "Fin de las muestras simuladas");
#else
    esp_err_t err = sht31_init(CONFIG_SHT31_I2C_SDA_GPIO, CONFIG_SHT31_I2C_SCL_GPIO,
                               CONFIG_SHT31_I2C_ADDR, CONFIG_SHT31_I2C_FREQ_HZ,
#ifdef CONFIG_SHT31_INTERNAL_PULLUP
                               true);
#else
                               false);
#endif
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "No se pudo iniciar el SHT31 (%s)", esp_err_to_name(err));
        return;
    }

    for (int n = 1; ; n++) {
        float t, rh;
        err = sht31_read(&t, &rh);
        if (err == ESP_OK) {
            run_inference(n, t, rh);
        } else {
            ESP_LOGE(TAG, "Fallo de lectura SHT31 (%s)", esp_err_to_name(err));
        }
        vTaskDelay(pdMS_TO_TICKS(CONFIG_AI_SAMPLE_PERIOD_MS));
    }
#endif
}
