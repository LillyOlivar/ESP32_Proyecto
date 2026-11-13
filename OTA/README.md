# Módulo OTA (ESP32 + ESP-IDF)

Actualización de firmware por Wi-Fi y HTTPS siguiendo la guía OTA del proyecto.

## Qué incluye
- `main/ota_client.c/.h`: cliente OTA (`esp_https_ota`): conexión TLS, lectura de versión, descarga a la partición inactiva, validación de la imagen, selección de arranque y reinicio.
- `main/wifi_sta.c/.h`: conexión Wi-Fi propia (la guía usa `example_connect()`, que depende de los ejemplos de ESP-IDF; se sustituyó para no depender de ellos).
- `main/main.c`: aplicación de ejemplo.
- `server/server_ota.py`, `server/gen_cert.sh`: servidor HTTPS (puerto 8070) y certificado.
- `partitions.csv`: `ota_0` y `ota_1` de 1500K (requiere flash de **4 MB**), `sdkconfig.defaults`, `version.txt`.

## Requisitos
ESP-IDF 6.1 (compilado para `esp32`), Python 3, OpenSSL, PC y ESP32 en la misma red.

## Pasos
1. **Certificado** (PC): `cd server && ./gen_cert.sh 192.168.1.50` (use la IP real de su PC; genera `server.crt/key` y copia el `.crt` a `main/certs/`).
2. **Configurar**: `idf.py set-target esp32` (o su modelo) y `idf.py menuconfig` → *Configuracion OTA*: SSID, contraseña y URL (`https://<IP>:8070/firmware_v2.bin`).
3. **Compilar y flashear v1.0.0**: `idf.py build flash monitor`.
4. **Compilar v2.0.0**: `echo 2.0.0 > version.txt`, luego `idf.py reconfigure build`, y copie `build/ota_esp32.bin` a `server/firmware_v2.bin`.
5. **Servidor** (PC): `cd server && python3 server_ota.py`.
6. **Probar**: reinicie el ESP32 (con la v1.0.0 flasheada; para repetir la prueba, reflashee v1.0.0).
   El monitor debe mostrar la descarga con progreso, "Firmware verificado e instalado", el reinicio y luego `version 2.0.0` en la otra partición (`ota_1`).

## Estado
| Funcionalidad | Estado |
|---|---|
| Descarga HTTPS + validación + reinicio | Implementado (no probado en hardware) |
| Rollback automático (`APP_ROLLBACK`) | Pendiente |
| Firma del firmware (Secure Boot) | Pendiente |
