/**
 * @file sht31.c
 * @brief Implementacion del driver SHT31 con i2c_master.
 * @author Proyecto Enfasis II EI
 * @version 1.0.0
 * @date 2026-11-12
 */
#include <stdbool.h>
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "sht31.h"

static const char *TAG = "SHT31";

/** Comando de medicion unica: alta repeticion, sin clock stretching (0x2400). */
static const uint8_t SHT31_CMD_MEASURE_HIGH[2] = { 0x24, 0x00 };
/** Tiempo de conversion (ms), mayor al maximo de 15 ms de la hoja de datos. */
#define SHT31_MEASURE_DELAY_MS 20
/** Timeout de las transacciones I2C (ms). */
#define SHT31_I2C_TIMEOUT_MS   100

static i2c_master_bus_handle_t s_bus;
static i2c_master_dev_handle_t s_dev;

/**
 * @brief CRC-8 del SHT31 (polinomio 0x31, valor inicial 0xFF).
 * @param[in] data Datos (2 bytes).
 * @param[in] len  Cantidad de bytes.
 * @return CRC calculado.
 */
static uint8_t sht31_crc8(const uint8_t *data, int len)
{
    uint8_t crc = 0xFF;
    for (int i = 0; i < len; i++) {
        crc ^= data[i];
        for (int b = 0; b < 8; b++) {
            crc = (crc & 0x80) ? (uint8_t)((crc << 1) ^ 0x31) : (uint8_t)(crc << 1);
        }
    }
    return crc;
}

esp_err_t sht31_init(int sda_gpio, int scl_gpio, uint8_t addr, uint32_t freq_hz, bool pullup)
{
    if (sda_gpio < 0 || scl_gpio < 0 || sda_gpio == scl_gpio || freq_hz == 0 ||
        (addr != 0x44 && addr != 0x45)) {
        ESP_LOGE(TAG, "Parametros invalidos: configure SDA/SCL y direccion en menuconfig");
        return ESP_ERR_INVALID_ARG;
    }
    if (s_dev != NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    i2c_master_bus_config_t bus_cfg = {
        .i2c_port = -1, /* asignacion automatica */
        .sda_io_num = sda_gpio,
        .scl_io_num = scl_gpio,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = pullup,
    };
    esp_err_t err = i2c_new_master_bus(&bus_cfg, &s_bus);
    if (err != ESP_OK) {
        return err;
    }

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = addr,
        .scl_speed_hz = freq_hz,
    };
    err = i2c_master_bus_add_device(s_bus, &dev_cfg, &s_dev);
    if (err != ESP_OK) {
        i2c_del_master_bus(s_bus);
        s_bus = NULL;
        s_dev = NULL;
    }
    return err;
}

esp_err_t sht31_read(float *temp_c, float *humidity_pct)
{
    if (temp_c == NULL || humidity_pct == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    if (s_dev == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t err = i2c_master_transmit(s_dev, SHT31_CMD_MEASURE_HIGH, sizeof(SHT31_CMD_MEASURE_HIGH),
                                        SHT31_I2C_TIMEOUT_MS);
    if (err != ESP_OK) {
        return err;
    }
    vTaskDelay(pdMS_TO_TICKS(SHT31_MEASURE_DELAY_MS));

    uint8_t buf[6]; /* T_MSB, T_LSB, T_CRC, RH_MSB, RH_LSB, RH_CRC */
    err = i2c_master_receive(s_dev, buf, sizeof(buf), SHT31_I2C_TIMEOUT_MS);
    if (err != ESP_OK) {
        return err;
    }
    if (sht31_crc8(&buf[0], 2) != buf[2] || sht31_crc8(&buf[3], 2) != buf[5]) {
        ESP_LOGW(TAG, "CRC invalido en la lectura");
        return ESP_ERR_INVALID_CRC;
    }

    uint16_t raw_t  = ((uint16_t)buf[0] << 8) | buf[1];
    uint16_t raw_rh = ((uint16_t)buf[3] << 8) | buf[4];
    *temp_c = -45.0f + 175.0f * ((float)raw_t / 65535.0f);   /* formula de la hoja de datos */
    *humidity_pct = 100.0f * ((float)raw_rh / 65535.0f);
    return ESP_OK;
}

esp_err_t sht31_deinit(void)
{
    if (s_dev == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    esp_err_t err = i2c_master_bus_rm_device(s_dev);
    s_dev = NULL;
    esp_err_t err2 = i2c_del_master_bus(s_bus);
    s_bus = NULL;
    return (err != ESP_OK) ? err : err2;
}
