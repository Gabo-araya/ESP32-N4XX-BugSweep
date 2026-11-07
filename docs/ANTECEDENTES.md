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
| 2026-11-07 | Fase 2 cerrada: las redes se ven en la placa. |
| 2026-11-07 | Fase 3: reglas de sospecha. **Decisiones:** (1) la tabla OUI se **genera** desde el registro oficial del IEEE (`tools/generar_oui.py`) y nunca se escribe a mano; (2) puntaje = mejor patrón de SSID + categoría del fabricante (cámara 60, chip de cámara 35, IoT 15) + red oculta (15), con tope de 100; (3) las MAC localmente administradas se marcan como "aleatorias" porque no tienen fabricante; (4) el texto en pantalla va sin tildes, porque las fuentes integradas de TFT_eSPI solo tienen ASCII. |
| 2026-11-07 | Fase 3 cerrada y commit hecho. |
| 2026-11-07 | Fase 4: sniffer promiscuo. **Decisiones:** (1) el *callback* solo copia metadatos (direcciones, tamaño, RSSI, canal y SSID de los beacons) a una cola de FreeRTOS, y **nunca guarda el payload**; (2) la tasa de cada estación se escala por el tiempo realmente escuchado en su canal; (3) el perfil de video exige una subida sostenida de 15 KB/s o más durante unos 15 s; (4) `radio::iniciar()` desactiva la persistencia y la reconexión automática, porque la NVS de fábrica tiene una red guardada; (5) el proyecto pasa a C++17. **Límite conocido:** el ESP32 es una radio 1x1 y no decodifica tramas MIMO de 2 flujos (802.11n MCS8 o superior), así que subestima el tráfico de celulares y notebooks modernos. Las cámaras baratas suelen ser 1x1, así que se ven bien. |
| 2026-11-07 | Fase 4 cerrada: el modo dispositivos funciona en la placa (fotos en `docs/img/paso4_*.jpg`). Las fotos revelan poco contraste en la fila seleccionada (fondo gris claro en el IPS), así que se cambia por un marco amarillo. |
| 2026-11-07 | Fase 5: modo RASTREO. **Decisiones:** (1) se fija el canal del objetivo: escucha el 100 % del tiempo en vez de ~1/13; (2) se miden los bytes que el objetivo **transmite** (a2), lo que cubre tanto a las estaciones como a las cámaras en modo AP; (3) la tendencia se calcula con dos promedios exponenciales del RSSI; (4) el gráfico se dibuja en un `TFT_eSprite` (~34 KB de RAM) para que no parpadee. **Error corregido:** al volver del canal fijo, los bytes acumulados inflaban la primera ventana y daban falsos "VIDEO"; ahora `Sniffer::iniciar()` reinicia los acumuladores. |
| 2026-11-07 | Fase 5 subida a la placa. Falta validar sus parámetros con pruebas reales. |
| 2026-11-07 | Fase 6: BLE. **Decisiones:** (1) tabla de particiones `huge_app.csv` (3 MB de app, sin OTA); (2) NimBLE-Arduino 1.4.3 con los roles central, periférico y broadcaster **desactivados al compilar**. Se verificó con `nm` que el binario no contiene `ble_gap_adv_start`, `ble_gap_connect` ni `esp_wifi_80211_tx`; (3) escaneo pasivo (`setActiveScan(false)`); (4) los modos de radio son excluyentes; (5) en BLE, la dirección aleatoria es lo normal, así que no se anota como motivo de sospecha. |
| 2026-11-07 | Fase 7 (parcial): se descarta GitHub Actions por decisión del autor y se reescribe el README con instrucciones de compilación para los tres sistemas operativos, problemas frecuentes y una guía de uso. Las pruebas de las fases 5 y 6 quedan pendientes: falta el hardware (rastreadores BLE, cámara de referencia). |
| 2026-11-07 | Fase 7: revisión guiada. **Decisiones:** (1) el aparato arranca en una pantalla de inicio que ofrece la revisión con pulsación larga (la opción recomendada); (2) cada revisión **parte de cero** (`limpiar()` en el sniffer y en BLE); (3) el resultado junta lo sospechoso de los tres modos y permite saltar al detalle y al rastreo; (4) incluso con un resultado limpio, la pantalla recuerda los límites (5 GHz, cámaras solo SD, radio analógica). Se documenta cómo alimentar el aparato desde el teléfono (OTG) y cómo usarlo como monitor serie. |
| 2026-11-07 | Se crea `docs/MANUAL.md`, el manual de usuario para personas sin conocimientos técnicos. La sección "Uso" del README queda como un resumen que enlaza al manual, para no duplicar contenido. La tabla de motivos del manual se armó a partir de los textos reales de `src/rules/sospecha.cpp`. |
| 2026-11-07 | Se prepara la **v0.9.0-beta**. **Decisiones:** (1) la versión se define una sola vez, en `platformio.ini` (`BUGSWEEP_VERSION`); (2) el binario único se arma con `esptool merge-bin` (DIO, 40 MHz, 4 MB) y se verificó byte a byte contra lo compilado; (3) las capturas de terreno van en `capturas/` (ignorada por git) y se comparten solo seudonimizadas: OUI conservado, MAC y nombres no sospechosos reemplazados por un HMAC-SHA256 con una sal local. |
| 2026-11-07 | Se crea `docs/instrucciones_test.md`: 13 tests sin costo y 9 con dispositivos, resultados esperados derivados de las reglas reales, y una lista de compras priorizada por valor y costo. `PRUEBAS.md` queda dedicado a capturar y compartir. **Advertencia agregada:** el celular de prueba debe estar en 2.4 GHz, o el aparato no verá su tráfico. |
