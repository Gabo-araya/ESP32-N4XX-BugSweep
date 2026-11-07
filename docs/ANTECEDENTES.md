# Antecedentes e historial del proyecto

Este documento reúne lo que se sabe del hardware, las decisiones tomadas y la bitácora del proyecto. Es la "memoria" del proyecto: si algo se descubre o se decide, se anota aquí.

---

## 1. Origen

El objetivo es construir un dispositivo portátil que ayude a **detectar cámaras y micrófonos ocultos** que transmiten de forma inalámbrica. Se usa una placa de desarrollo ESP32 barata que trae una pantalla incorporada.

El proyecto también sirve para aprender desarrollo embebido. El autor partió sin experiencia previa en ESP32.

---

## 2. Hardware

Resumen:
- Chip **ESP32-D0WD-V3** con 4 MB de flash y sin PSRAM.
- Pantalla LCD **ST7789 de 1.9"** (170×320) conectada por SPI.
- Puente USB-serie **CH340** con conector USB-C.
- Radio **solo de 2.4 GHz**, con WiFi y Bluetooth 4.2.

Todos los detalles están en la **[ficha del dispositivo](FICHA_DISPOSITIVO.md)**: especificaciones, pinout de la pantalla, mapa de GPIO, tabla de particiones, firmware de fábrica y cómo restaurarlo.

---

## 3. Límites físicos de la detección

| ✅ Se puede detectar | ❌ No se puede detectar con este hardware |
|---|---|
| Dispositivos que transmiten por WiFi de 2.4 GHz (la mayoría de las cámaras espía baratas) | WiFi de 5 GHz |
| Dispositivos Bluetooth LE (rastreadores tipo AirTag o Tile, algunos micrófonos) | Cámaras que solo graban en una tarjeta SD sin transmitir |
| Puntos de acceso con nombres típicos de cámaras (`HDWiFiCam`, `IPC-…`, `MV…`) y redes ocultas | Micrófonos analógicos de radiofrecuencia (433 MHz, 900 MHz, 1.2 GHz) |
| El fabricante del chip, a partir del OUI de la dirección MAC | Micrófonos que transmiten por la red celular (GSM/4G) |
| Cambios de tráfico correlacionados con movimiento | Dispositivos con cable |

Técnicas clave:

1. **Correlación de tráfico.** Una cámara que transmite video envía datos todo el tiempo, y cuando la escena cambia (mover la mano frente a ella, apagar la luz), el volumen de datos cambia también. El ESP32 puede escuchar ese tráfico en **modo promiscuo** aunque la cámara esté conectada a una red ajena, sin conectarse a ella.
2. **Localización por RSSI.** La intensidad de la señal permite acercarse al dispositivo jugando a "frío o caliente".
3. **Identificación de fabricante por OUI.** Algunos fabricantes sugieren que el dispositivo es una cámara (Hikvision, Dahua, Tuya, Beken, Ingenic…). Otros son ambiguos (Espressif, Realtek), porque sus chips se usan en muchos tipos de aparatos.

Métodos complementarios, fuera del dispositivo:
- La linterna del celular, para ver el reflejo de un lente.
- Un detector de radiofrecuencia de banda ancha, para los micrófonos analógicos.
- `nmap` en tu propia red.
- Una cámara térmica.

---

## 4. Alternativas evaluadas

| Opción | Pros | Contras | Decisión |
|---|---|---|---|
| **ESP32 Marauder** / **Bruce** | Ya están hechos, con escáner WiFi y BLE y detección de AirTags | Incluyen funciones de ataque (deauth, spam de beacons) que son ilegales fuera de tu propia red. No están pensados para cámaras ni para esta pantalla | Solo como referencia |
| **Firmware propio** | Hecho a la medida, solo pasivo, sirve para aprender, permite controlar la accesibilidad | Hay que escribirlo | ✅ **Elegido** |

| Opción | Pros | Contras | Decisión |
|---|---|---|---|
| **PlatformIO + Arduino** | Fácil de aprender, muchas librerías | Menos control fino que ESP-IDF | ✅ **Elegido** |
| **ESP-IDF** | Control total del WiFi | Curva de aprendizaje alta | Para después, si hace falta |

| Opción | Decisión |
|---|---|
| Librería **TFT_eSPI** configurada con `build_flags` en `platformio.ini` | ✅ La configuración vive en el proyecto y no hay que editar la librería |
| Plataforma `espressif32@^6` (núcleo Arduino 2.x) | ✅ TFT_eSPI tiene problemas conocidos con el núcleo Arduino 3.x |

---

## 5. Preparación del entorno (Linux)

```bash
# Permiso para usar el puerto serie (después cierra sesión y vuelve a entrar)
sudo usermod -aG dialout $USER

# Si /dev/ttyUSB0 desaparece al conectar la placa, el culpable es brltty
# (un programa para lectores Braille que se apropia de los chips CH340)
sudo apt remove brltty

# Identificar el chip
uvx esptool --port /dev/ttyUSB0 flash-id

# Respaldar la flash completa
uvx esptool --port /dev/ttyUSB0 read-flash 0 ALL respaldo_fabrica.bin

# Monitor serie (sales con Ctrl+])
uvx --from pyserial pyserial-miniterm /dev/ttyUSB0 115200
```

Editor: VS Code con la extensión **PlatformIO IDE**.

---

## 6. Ruta de aprendizaje

1. C/C++ básico: variables, funciones, `setup()` y `loop()`.
2. PlatformIO: compilar, subir el programa y leer el monitor serie.
3. Pantalla: la librería TFT_eSPI y el bus SPI.
4. Escaneo WiFi (`WiFi.scanNetworks`) y BLE (NimBLE).
5. Tramas 802.11 y modo promiscuo.
6. Direcciones MAC, OUI y direcciones MAC aleatorias.
7. FreeRTOS (tareas y colas), para hacer varias cosas a la vez.

---

## 7. Bitácora

| Fecha | Evento |
|---|---|
| 2026-11-07 | Inicio del proyecto. Se evalúa qué puede detectar un ESP32 y qué no. |
| 2026-11-07 | Se identifica el chip (ESP32-D0WD-V3, 4 MB) y se respalda el firmware de fábrica. |
| 2026-11-07 | Se identifica la pantalla (ST7789 de 1.9", 170×320) y su pinout a partir de la serigrafía de la foto del producto. |
| 2026-11-07 | Se crea el proyecto PlatformIO. Paso 1: prueba de pantalla y puerto serie (`src/main.cpp`). **Pendiente: probar en el hardware.** |
| 2026-11-07 | Se crea la documentación: README, ANTECEDENTES, PRD y PLAN. |
| 2026-11-07 | El proyecto pasa a llamarse **ESP32-N4XX-BugSweep**. Se adopta la licencia MIT (© Gabo Araya). |
| 2026-11-07 | Se analiza el respaldo de fábrica: tabla de particiones `default.csv` de Arduino, un sketch compilado con arduino-esp32 2.0.11 (12 de junio de 2023) sobre restos de ESP-AT, y una NVS con claves de WiFi. Se crea `FICHA_DISPOSITIVO.md`. |
| 2026-11-07 | **Paso 1 funcionando**: la compilación y la subida funcionan, la pantalla muestra el título y los colores, y el monitor serie avanza. Retroiluminación confirmada con `HIGH`. Problema resuelto: `Permission denied` en `/dev/ttyUSB0`, porque el grupo `dialout` exige volver a iniciar sesión. |
| 2026-11-07 | Una foto confirma la pantalla: 170×320, colores RGB correctos, sin inversión y sin desplazamiento. **Paso 1 cerrado.** |
| 2026-11-07 | Paso 1b cerrado: módulo `Boton` reutilizable (antirrebote y pulsación corta/larga). El repositorio se publica en GitHub. |
| 2026-11-07 | Fase 2: escáner WiFi **pasivo** (`scanNetworks(async, ocultas, pasivo=true, 300 ms/canal)`). **Decisión:** no se usa escaneo activo, porque emite *probe requests* (contradice el diseño solo receptor del PRD y delata al dispositivo). |
