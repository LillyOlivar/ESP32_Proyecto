#!/usr/bin/env python3
"""
@file server_ota.py
@brief Servidor HTTPS local para distribuir firmware OTA al ESP32.
@author Proyecto Enfasis II EI
@version 1.0.0
@date 2026-11-12

Sirve los archivos del directorio indicado (por defecto, el del script) por
HTTPS en el puerto 8070, como describe la guia OTA. Requiere server.crt y
server.key (ver gen_cert.sh) y el binario firmware_v2.bin.
"""
import argparse
import functools
import http.server
import os
import ssl
import sys

PORT = 8070
CERT_FILE = "server.crt"
KEY_FILE = "server.key"


def build_parser():
    """
    @brief Construye el parser de argumentos de linea de comandos.
    @return argparse.ArgumentParser configurado.
    """
    p = argparse.ArgumentParser(description="Servidor HTTPS para OTA del ESP32")
    here = os.path.dirname(os.path.abspath(__file__))
    p.add_argument("--port", type=int, default=PORT, help="puerto HTTPS (por defecto 8070)")
    p.add_argument("--dir", default=here, help="directorio con firmware_v2.bin y certificados")
    p.add_argument("--cert", default=None, help="ruta de server.crt (por defecto en --dir)")
    p.add_argument("--key", default=None, help="ruta de server.key (por defecto en --dir)")
    return p


def main():
    """
    @brief Inicia el servidor HTTPS y atiende peticiones hasta Ctrl+C.
    @return Codigo de salida del proceso (0 correcto, 1 error de configuracion).
    @warning server.key es privada: no debe subirse al repositorio.
    """
    args = build_parser().parse_args()
    cert = args.cert or os.path.join(args.dir, CERT_FILE)
    key = args.key or os.path.join(args.dir, KEY_FILE)

    for path in (cert, key):
        if not os.path.isfile(path):
            print(f"ERROR: no existe {path}. Ejecute gen_cert.sh primero.", file=sys.stderr)
            return 1

    handler = functools.partial(http.server.SimpleHTTPRequestHandler, directory=args.dir)
    httpd = http.server.ThreadingHTTPServer(("", args.port), handler)

    # Contexto SSL: ESP-IDF exige conexion TLS
    context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
    context.load_cert_chain(certfile=cert, keyfile=key)
    httpd.socket = context.wrap_socket(httpd.socket, server_side=True)

    print(f"Servidor OTA HTTPS activo en https://0.0.0.0:{args.port} (directorio: {args.dir})")
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        print("\nServidor detenido.")
    finally:
        httpd.server_close()
    return 0


if __name__ == "__main__":
    sys.exit(main())
