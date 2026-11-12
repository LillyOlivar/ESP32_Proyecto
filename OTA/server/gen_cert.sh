#!/usr/bin/env bash
# Genera el certificado autofirmado para la IP del PC y lo copia al firmware.
# Uso: ./gen_cert.sh 192.168.1.50
set -euo pipefail

if [ $# -ne 1 ]; then
    echo "Uso: $0 <IP_DEL_PC>   (ejemplo: $0 192.168.1.50)" >&2
    exit 1
fi
IP="$1"
cd "$(dirname "$0")"

openssl req -x509 -newkey rsa:2048 -keyout server.key -out server.crt -days 365 -nodes \
    -subj "/CN=${IP}" -addext "subjectAltName=IP:${IP}"

mkdir -p ../main/certs
cp server.crt ../main/certs/server.crt
echo "OK: server.crt/server.key creados y server.crt copiado a main/certs/"
