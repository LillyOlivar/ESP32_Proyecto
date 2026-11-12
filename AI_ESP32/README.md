# Módulo de IA en el ESP32

| Módulo | Algoritmo | Archivos |
|---|---|---|
| K-Means | K=3, Min-Max (T 0–50 °C, RH 0–100 %), centroides y etiquetas originales, distancia² en el bucle y raíz final | `kmeans.c/.h` |
| TinyML Monitor | Z-Score bivariado + distancia euclidiana (umbral 2.5), punto de rocío (Magnus), VPD. **No es Random Forest.** | `tinyml_monitor.c/.h` |
| Sensor | Driver SHT31 I2C (medición única, CRC) | `sht31.c/.h` |
| Integración | `app_main` que llama a ambos algoritmos | `main.c` |

## Uso
Requiere ESP-IDF ≥ 5.2 (API `i2c_master`).
- **Sin hardware** (por defecto): `idf.py build flash monitor` ejecuta las 8 muestras de los códigos originales.
- **Con SHT31**: `idf.py menuconfig` → *Configuracion IA / SHT31*: desactive las muestras simuladas y configure SDA, SCL, dirección (0x44/0x45). Los pines no tienen valor por defecto: dependen de su cableado.

Cada algoritmo se reutiliza con `kmeans_predict(t, rh, &res)` y `tinyml_monitor_process(t, rh, &res)`.

## Comprobación en PC (compilación con gcc, no en el ESP32)
Salida esperada para las muestras originales: 16.2/72 → cluster 0; 23.8/52 y 25/48 → cluster 1; 37.5/88 → cluster 2; el monitor marca ANOMALÍA en 37.5/88, 38.5/95 y 22/15.

## Estado
| Funcionalidad | Estado |
|---|---|
| K-Means y TinyML Monitor | Implementados |
| Lectura real del SHT31 | Implementada, no probada en hardware |
| Registro en MicroSD | **Pendiente** (solo se expone `is_anomaly` / `cluster_id`) |
| Muestreo adaptativo (10 min / 5 s) | **Pendiente** |
