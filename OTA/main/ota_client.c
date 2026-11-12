/**
 * @file ota_client.c
 * @brief Implementacion del cliente OTA por HTTPS.
 * @author Proyecto Enfasis II EI
 * @version 1.0.0
 * @date 2026-11-12
 */
#include <string.h>
#include "esp_log.h"
#include "esp_system.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_ota_ops.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ota_client.h"

static const char *TAG = "OTA_CLIENT";

/** Tamano de pila de la tarea OTA (bytes), segun la guia. */
#define OTA_TASK_STACK_SIZE 8192
/** Prioridad de la tarea OTA. */
#define OTA_TASK_PRIORITY   5
/** Intervalo (%) de avance entre mensajes de progreso. */
#define OTA_PROGRESS_STEP   10

/** Copia interna de la configuracion usada por la tarea. */
static ota_client_config_t s_task_cfg;
/** Indica si ya hay una tarea OTA en ejecucion. */
static volatile bool s_task_running;

void ota_client_log_running_info(void)
{
    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_app_desc_t desc;

    if (running != NULL && esp_ota_get_partition_description(running, &desc) == ESP_OK) {
        ESP_LOGI(TAG, "Firmware en ejecucion: version %s, particion '%s' (offset 0x%lx)",
                 desc.version, running->label, (unsigned long)running->address);
    } else {
        ESP_LOGW(TAG, "No se pudo leer la descripcion del firmware en ejecucion");
    }
}

esp_err_t ota_client_run(const ota_client_config_t *cfg)
{
    if (cfg == NULL || cfg->url == NULL || cfg->url[0] == '\0' ||
        cfg->cert_pem == NULL || cfg->timeout_ms <= 0) {
        ESP_LOGE(TAG, "Configuracion OTA invalida");
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "Iniciando descarga OTA desde %s", cfg->url);

    esp_http_client_config_t http_cfg = {
        .url = cfg->url,
        .cert_pem = cfg->cert_pem,
        .timeout_ms = cfg->timeout_ms,
        .keep_alive_enable = true,
    };
    esp_https_ota_config_t ota_cfg = { .http_config = &http_cfg };
    esp_https_ota_handle_t handle = NULL;

    esp_err_t err = esp_https_ota_begin(&ota_cfg, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_https_ota_begin fallo: %s", esp_err_to_name(err));
        return err;
    }

    /* Verificacion previa: descriptor de la imagen y version */
    esp_app_desc_t new_desc;
    err = esp_https_ota_get_img_desc(handle, &new_desc);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "No se pudo leer el descriptor de la imagen: %s", esp_err_to_name(err));
        goto abort_ota;
    }
    ESP_LOGI(TAG, "Firmware remoto: proyecto '%s', version %s", new_desc.project_name, new_desc.version);

    esp_app_desc_t run_desc;
    if (cfg->skip_same_version &&
        esp_ota_get_partition_description(esp_ota_get_running_partition(), &run_desc) == ESP_OK &&
        strncmp(new_desc.version, run_desc.version, sizeof(new_desc.version)) == 0) {
        ESP_LOGW(TAG, "La version %s ya esta instalada; no se actualiza", run_desc.version);
        err = ESP_ERR_INVALID_VERSION;
        goto abort_ota;
    }

    /* Descarga y escritura en la particion inactiva */
    int last_step = -1;
    while ((err = esp_https_ota_perform(handle)) == ESP_ERR_HTTPS_OTA_IN_PROGRESS) {
        int total = esp_https_ota_get_image_size(handle);
        int read  = esp_https_ota_get_image_len_read(handle);
        if (total > 0) {
            int pct = (int)(((int64_t)read * 100) / total);
            if (pct / OTA_PROGRESS_STEP != last_step) {
                last_step = pct / OTA_PROGRESS_STEP;
                ESP_LOGI(TAG, "Progreso: %d%% (%d/%d bytes)", pct, read, total);
            }
        }
    }
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Descarga fallida: %s", esp_err_to_name(err));
        goto abort_ota;
    }

    if (!esp_https_ota_is_complete_data_received(handle)) {
        ESP_LOGE(TAG, "Descarga incompleta");
        err = ESP_ERR_INVALID_SIZE;
        goto abort_ota;
    }

    /* finish valida la imagen (magic, checksum, hash) y fija la particion de arranque */
    err = esp_https_ota_finish(handle);
    if (err != ESP_OK) {
        if (err == ESP_ERR_OTA_VALIDATE_FAILED) {
            ESP_LOGE(TAG, "La imagen descargada no supero la validacion");
        } else {
            ESP_LOGE(TAG, "esp_https_ota_finish fallo: %s", esp_err_to_name(err));
        }
        return err; /* el handle ya fue liberado por finish */
    }

    ESP_LOGI(TAG, "Firmware verificado e instalado correctamente");
    return ESP_OK;

abort_ota:
    esp_https_ota_abort(handle);
    return err;
}

/**
 * @brief Tarea FreeRTOS: ejecuta la OTA y reinicia si fue exitosa.
 * @param[in] pvParameter No utilizado.
 */
static void ota_task(void *pvParameter)
{
    (void)pvParameter;

    esp_err_t err = ota_client_run(&s_task_cfg);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Actualizacion exitosa. Reiniciando...");
        vTaskDelay(pdMS_TO_TICKS(1000));
        esp_restart();
    }
    ESP_LOGE(TAG, "OTA no completada (0x%x: %s). Se mantiene el firmware actual.",
             (unsigned)err, esp_err_to_name(err));

    s_task_running = false;
    vTaskDelete(NULL);
}

esp_err_t ota_client_start_task(const ota_client_config_t *cfg)
{
    if (cfg == NULL || cfg->url == NULL || cfg->cert_pem == NULL || cfg->timeout_ms <= 0) {
        return ESP_ERR_INVALID_ARG;
    }
    if (s_task_running) {
        return ESP_ERR_INVALID_STATE;
    }

    s_task_cfg = *cfg;
    s_task_running = true;
    if (xTaskCreate(ota_task, "ota_task", OTA_TASK_STACK_SIZE, NULL, OTA_TASK_PRIORITY, NULL) != pdPASS) {
        s_task_running = false;
        return ESP_ERR_NO_MEM;
    }
    return ESP_OK;
}
