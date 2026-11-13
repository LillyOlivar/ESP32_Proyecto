# ESP32: OTA e Inteligencia Artificial

Proyecto de Énfasis II (Entornos Inteligentes) con dos requisitos:
1. **OTA** por Wi-Fi/HTTPS → [`OTA/`](OTA/README.md)
2. **IA en el ESP32** (K-Means y TinyML Monitor con SHT31) → [`AI_ESP32/`](AI_ESP32/README.md)

Son dos proyectos ESP-IDF independientes (cada uno con su `app_main`). Los módulos exponen funciones reutilizables, de modo que se pueden combinar en una sola aplicación copiando los `.c/.h` y llamando a `ota_client_start_task()` y a las funciones de inferencia.

```text
ESP32_Proyecto/
├── OTA/        cliente OTA, servidor Python, particiones
├── AI_ESP32/   kmeans, tinyml_monitor, driver SHT31
├── docs/       Doxyfile (salida en docs/html)
├── README.md
└── .gitignore
```

## Documentación Doxygen
```bash
cd docs
doxygen Doxyfile      # abre docs/html/index.html
```

## Publicar en GitHub
```bash
cd ESP32_Proyecto
git init
git add .
git commit -m "ESP32: OTA por HTTPS e IA (K-Means y TinyML Monitor)"
git branch -M main
git remote add origin https://github.com/TU_USUARIO/TU_REPOSITORIO.git
git push -u origin main
```
(Cree antes el repositorio vacío en GitHub y reemplace la URL.) `.gitignore` excluye claves y certificados: nunca suba `server.key`.

## Verificación realizada y pendiente
- Verificado: ambos proyectos compilan con ESP-IDF v6.1 para `esp32` (ESP-WROOM-32); algoritmos ejecutados en PC con gcc; servidor HTTPS probado localmente con `curl`; documentacion Doxygen generada (Doxygen 1.18.0).
- **No verificado** (sin cable USB para flashear): OTA real en el dispositivo y lectura del SHT31.
