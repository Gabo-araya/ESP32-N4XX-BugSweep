# ESP32-N4XX-BugSweep

**Detector pasivo de cámaras y micrófonos inalámbricos ocultos para ESP32.**

Este firmware convierte una placa ESP32 barata con pantalla LCD en un detector de bolsillo. **Solo escucha** el tráfico WiFi de 2.4 GHz y Bluetooth LE, y ayuda a identificar y ubicar cámaras y micrófonos ocultos que transmiten.

> **Estado:** en desarrollo temprano (Fase 1). Ver el [plan de trabajo](docs/PLAN.md).

## Qué hace y qué no hace

El dispositivo:

- ✅ Lista las redes WiFi, los dispositivos conectados a ellas (modo promiscuo) y los dispositivos Bluetooth LE.
- ✅ Marca como sospechosos los nombres de red y los fabricantes típicos de cámaras.
- ✅ Detecta dispositivos que suben video de forma continua (en modo promiscuo).
- ✅ Detecta rastreadores BLE: Apple Find My (AirTag), Tile, Samsung SmartTag y la red de Google.
- ✅ Ayuda a ubicar un dispositivo según la intensidad de su señal y comprueba si su tráfico reacciona al movimiento (correlación).
- ❌ **No** detecta WiFi de 5 GHz, cámaras que solo graban en una tarjeta SD, micrófonos analógicos de radiofrecuencia ni micrófonos que usan la red celular.

Un resultado limpio **no garantiza** que no haya dispositivos ocultos. Complementa la revisión con una linterna (para ver el reflejo de un lente) y con un detector de radiofrecuencia de banda ancha. Ver [los límites de la detección](docs/ANTECEDENTES.md#3-límites-físicos-de-la-detección).

## Hardware

- Placa "ESP32 WiFi Bluetooth Development Board 1.9 Inch LCD CH340 Type-C".
  - Chip ESP32-D0WD-V3 y 4 MB de flash.
  - Pantalla ST7789 de 170×320 píxeles.
- Cable USB-C de datos (no solo de carga).

Todas las especificaciones, el pinout y el firmware de fábrica están en la [ficha del dispositivo](docs/FICHA_DISPOSITIVO.md).

## Compilar e instalar

Requisitos: [VS Code](https://code.visualstudio.com/) con la extensión [PlatformIO IDE](https://platformio.org/install/ide?install=vscode).

En Linux, primero da permiso al puerto serie:

```bash
sudo usermod -aG dialout $USER   # después cierra sesión y vuelve a entrar
```

Después:

1. Abre esta carpeta en VS Code.
2. En la barra de PlatformIO, haz clic en **✓ Build**, luego en **→ Upload**, y después en **🔌 Monitor**.

O desde la terminal:

```bash
pio run -t upload && pio device monitor
```

Si la subida se queda en `Connecting...`, mantén apretado el botón **BOOT** hasta que empiece a escribir.

## Tabla de fabricantes (OUI)

`src/rules/oui_tabla.h` se genera a partir del registro público del IEEE:

```bash
python3 -I tools/generar_oui.py
```

Para agregar un fabricante, edita la lista `FABRICANTES` del script y vuelve a ejecutarlo.

## Verificar que es solo receptor

El firmware no incluye funciones para transmitir tramas WiFi arbitrarias, ni para anunciarse o conectarse por BLE: NimBLE se compila solo con el rol de observador. Se puede comprobar en el binario:

```bash
pio run
nm -C .pio/build/esp32-lcd19/firmware.elf | grep -ciE "esp_wifi_80211_tx|ble_gap_adv_start|ble_gap_connect"
# debe imprimir 0
```

## Documentación

| Documento | Contenido |
|---|---|
| [docs/FICHA_DISPOSITIVO.md](docs/FICHA_DISPOSITIVO.md) | Ficha técnica: chip, flash, radio, pantalla, GPIO y firmware de fábrica |
| [docs/ANTECEDENTES.md](docs/ANTECEDENTES.md) | Hardware, decisiones, ruta de aprendizaje y bitácora |
| [docs/PRD.md](docs/PRD.md) | Requisitos del producto |
| [docs/PLAN.md](docs/PLAN.md) | Plan de trabajo por fases |

## Uso responsable

Este proyecto es **solo receptor**: no transmite tramas, no se conecta a redes ajenas y no descifra contenido. Úsalo para proteger tu privacidad en espacios que ocupas legítimamente.

Las direcciones MAC de terceros se tratan como datos personales: solo se guardan en RAM y no se almacenan ni se envían a ningún lado. Revisa la legislación local antes de usarlo.

## Licencia

[MIT](LICENSE) © 2026 Gabo Araya

---



