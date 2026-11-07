#!/usr/bin/env bash
# Compila y genera UN solo .bin para instalar sin compilar (dirección 0x0).
# Uso: tools/empaquetar.sh        -> dist/bugsweep-v<versión>-esp32-lcd19.bin (+ .sha256)
set -euo pipefail
cd "$(dirname "$0")/.."

ENV=esp32-lcd19
VERSION=$(sed -n 's/.*BUGSWEEP_VERSION=\\"\([^\\]*\)\\".*/\1/p' platformio.ini)
[ -n "$VERSION" ] || { echo "No encontré BUGSWEEP_VERSION en platformio.ini" >&2; exit 1; }

PIO=${PIO:-pio}
"$PIO" run -e "$ENV"

B=.pio/build/$ENV
BOOT_APP0=$HOME/.platformio/packages/framework-arduinoespressif32/tools/partitions/boot_app0.bin
SALIDA=dist/bugsweep-v$VERSION-$ENV.bin
mkdir -p dist

# Mismas direcciones y parámetros que usa "pio run -t upload" para esp32dev (DIO, 40 MHz).
uvx esptool --chip esp32 merge-bin -o "$SALIDA" \
  --flash-mode dio --flash-freq 40m --flash-size 4MB \
  0x1000 "$B/bootloader.bin" \
  0x8000 "$B/partitions.bin" \
  0xe000 "$BOOT_APP0" \
  0x10000 "$B/firmware.bin"

(cd dist && sha256sum "$(basename "$SALIDA")" > "$(basename "$SALIDA").sha256")
echo
echo "Listo: $SALIDA"
cat "$SALIDA.sha256"
