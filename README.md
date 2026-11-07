# ESP32-N4XX-BugSweep

**Detector pasivo de cámaras y micrófonos inalámbricos ocultos para ESP32.**

Este firmware convierte una placa ESP32 barata con pantalla LCD en un detector de bolsillo. **Solo escucha** el tráfico WiFi de 2.4 GHz y Bluetooth LE, y ayuda a identificar y ubicar cámaras, micrófonos y rastreadores ocultos que transmiten.

> **Estado:** v0.9.0-beta. Funciones completas; umbrales en calibración. Ver el [plan de trabajo](docs/PLAN.md) y el [CHANGELOG](CHANGELOG.md).

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
- Cable USB-C **de datos**. Muchos cables baratos solo sirven para cargar y no transmiten datos.
- Para usarlo en la calle: una batería externa (*power bank*) o **el teléfono**:
  - **Android**: cable USB-C a USB-C. Si no enciende, usa un adaptador **OTG** (USB-C macho a USB-A hembra) con un cable A→C; en algunos modelos hay que activar "OTG" en Ajustes.
  - **iPhone 15 o posterior**: cable USB-C a USB-C.
  - La app gratuita **Serial USB Terminal** (Android, a 115200 baudios) convierte el teléfono en monitor serie.

Todas las especificaciones, el pinout y el firmware de fábrica están en la [ficha del dispositivo](docs/FICHA_DISPOSITIVO.md).

---

## Uso

📖 **[Manual de usuario completo](docs/MANUAL.md)**: revisión guiada, cómo interpretar los resultados, rastreo, qué hacer si encuentras algo y revisión manual complementaria.

Resumen rápido: todo se maneja con el botón **BOOT**, el que está junto al USB-C. Un **toque corto** avanza y una **pulsación larga** elige. Al encender, mantén apretado el botón para hacer una **revisión guiada** de ~1 minuto.

---

## Instalar sin compilar

Cada versión publicada incluye un **binario único** en [Releases](https://github.com/Gabo-araya/ESP32-N4XX-BugSweep/releases), por ejemplo `bugsweep-v0.9.0-beta-esp32-lcd19.bin`, que se escribe en la dirección `0x0`.

**Desde el navegador** (Chrome o Edge, sin instalar nada):
1. Abre [ESP Tool web de Espressif](https://espressif.github.io/esptool-js/) y conecta la placa.
2. **Connect**, elige el puerto (CH340 / USB Serial).
3. En **Flash Address** escribe `0x0`, elige el `.bin` y presiona **Program**.

**Desde la terminal**:

```bash
uvx esptool --port /dev/ttyUSB0 write-flash 0x0 bugsweep-v0.9.0-beta-esp32-lcd19.bin
```

Comprueba la descarga con el archivo `.sha256` publicado junto al binario: `sha256sum -c bugsweep-v0.9.0-beta-esp32-lcd19.bin.sha256`.

> Igual necesitas el driver CH340 en Windows y el grupo `dialout` en Linux (ver el paso 1 de abajo).

## Compilar e instalar

### 1. Instalar las herramientas

1. **[VS Code](https://code.visualstudio.com/)**.
2. La extensión **[PlatformIO IDE](https://platformio.org/install/ide?install=vscode)**. En VS Code: pestaña Extensiones (Ctrl+Shift+X), busca "PlatformIO IDE" e instálala. La primera vez descarga sus herramientas; espera a que termine.
3. **El driver del chip USB-serie CH340**:

   | Sistema | Qué hacer |
   |---|---|
   | Linux | No necesita driver. Sí necesita permisos (ver más abajo) |
   | Windows | Instala el driver CH341SER desde el [sitio de WCH](https://www.wch-ic.com/downloads/CH341SER_EXE.html). La placa aparece como `COMx` en el Administrador de dispositivos |
   | macOS | Las versiones recientes lo traen incluido. Si no aparece, instala el [driver de WCH para macOS](https://www.wch-ic.com/downloads/CH341SER_MAC_ZIP.html) |

4. **Solo en Linux**: da permiso al puerto serie a tu usuario.

   ```bash
   sudo usermod -aG dialout $USER
   ```

   **Después cierra la sesión del escritorio y vuelve a entrar**, o reinicia. No basta con cerrar la terminal. Para comprobarlo: `id -nG | grep dialout`.

### 2. Descargar el proyecto

```bash
git clone https://github.com/Gabo-araya/ESP32-N4XX-BugSweep.git
```

O en GitHub: **Code → Download ZIP**, y descomprímelo.

### 3a. Compilar y subir desde VS Code

1. **File → Open Folder…** y elige la carpeta `ESP32-N4XX-BugSweep`, la que contiene `platformio.ini`.
2. Espera a que PlatformIO configure el proyecto. **La primera vez descarga unos 500 MB**: el compilador del ESP32, el framework Arduino y las librerías.
3. Conecta la placa por USB.
4. En la barra azul inferior:
   - **✓ (Build)** compila. Debe terminar con `SUCCESS`.
   - **→ (Upload)** compila y sube el programa a la placa.
   - **🔌 (Serial Monitor)** muestra los mensajes de la placa. Para salir: Ctrl+C.

### 3b. Compilar y subir desde la terminal

Si prefieres no usar VS Code, instala PlatformIO Core:

```bash
pipx install platformio          # o bien: uv tool install platformio
```

Y dentro de la carpeta del proyecto:

```bash
pio run                          # compilar
pio run -t upload                # compilar y subir
pio device monitor               # ver los mensajes (salir: Ctrl+C)
```

PlatformIO detecta el puerto solo. Si tienes varias placas conectadas, indícalo con `--upload-port /dev/ttyUSB0` (Linux), `COM3` (Windows) o `/dev/cu.wchusbserial*` (macOS).

### 4. Comprobar que funciona

Al arrancar, la pantalla muestra **REDES** y, a los pocos segundos, la lista de redes WiFi cercanas. El monitor serie debe mostrar:

```
[bugsweep] tabla OUI: 757 prefijos
[bugsweep] modo REDES (solo recepcion)
[redes] barrido #1: 12 redes, 0 sospechosas
```

### Problemas frecuentes

| Síntoma | Causa | Solución |
|---|---|---|
| `Permission denied: '/dev/ttyUSB0'` | Tu usuario no tiene permiso sobre el puerto (Linux) | Agrégalo al grupo `dialout` (paso 1.4) y **vuelve a iniciar sesión** |
| `/dev/ttyUSB0` aparece y desaparece al conectar | El servicio `brltty` (lectores Braille) se apropia de los chips CH340 | `sudo apt remove brltty`. Si usas una línea Braille, mejor desactiva solo la regla del CH340 |
| `Could not open port ... busy` | El monitor serie u otro programa está usando el puerto | Cierra el monitor (Ctrl+C) antes de subir el programa |
| La subida se queda en `Connecting.....` | La placa no entró en modo de grabación | Mantén apretado **BOOT** hasta que empiece a escribir. Si sigue igual, prueba otro cable USB |
| No aparece ningún puerto | El cable solo carga, o falta el driver CH340 | Usa un cable de datos e instala el driver (paso 1.3) |
| `ModuleNotFoundError: No module named 'platformio'` al usar `pio` en la terminal | Se actualizó el Python del sistema y el entorno de PlatformIO quedó roto | Cierra VS Code, borra `~/.platformio/penv` y vuelve a abrir VS Code: lo reinstala. Alternativa: usa la opción 3b |
| Pantalla negra, pero el monitor serie avanza | Polaridad de la retroiluminación de otra versión de la placa | En `platformio.ini`, cambia `TFT_BACKLIGHT_ON=HIGH` por `LOW` |
| Colores cambiados o imagen corrida | Otro panel u otra variante (por ejemplo, la de 1.4") | Ajusta `TFT_WIDTH`, `TFT_HEIGHT` y los pines en `platformio.ini` (ver la [ficha](docs/FICHA_DISPOSITIVO.md#5-pantalla)) |

### Volver al firmware de fábrica o borrar la placa

```bash
# Respaldar lo que tiene la placa (hazlo antes de la primera subida)
uvx esptool --port /dev/ttyUSB0 read-flash 0 ALL respaldo.bin

# Restaurar un respaldo
uvx esptool --port /dev/ttyUSB0 write-flash 0 respaldo.bin

# Borrar la flash completa, incluida la NVS con redes WiFi guardadas
uvx esptool --port /dev/ttyUSB0 erase-flash
```

`uvx` viene con [uv](https://docs.astral.sh/uv/). Sin uv, usa `pipx run esptool` o el esptool que trae PlatformIO.

> ⚠️ **No publiques nunca un respaldo `.bin`**: puede contener nombres y contraseñas de redes WiFi. El `.gitignore` del proyecto los excluye.

---

## Estructura del código

```
src/
  main.cpp              modos, vistas y botón (máquina de estados)
  ui/boton.*            botón con antirrebote: pulsación corta y larga
  ui/vistas.*           pantallas: lista, detalle y rastreo
  wifi/escaner_wifi.*   escaneo WiFi pasivo (beacons)
  wifi/radio.h          inicio de la radio sin conectarse nunca
  sniffer/sniffer.*     modo promiscuo: estaciones y tráfico por canal
  sniffer/seguimiento.* rastreo: tendencia de la señal e historial de tráfico
  guia/revision.*       revisión guiada: pasos, duraciones e instrucciones
  ble/escaner_ble.*     BLE pasivo y detección de rastreadores
  rules/sospecha.*      reglas y puntaje de sospecha
  rules/oui_tabla.h     tabla de fabricantes (GENERADA, no editar)
tools/generar_oui.py    genera oui_tabla.h desde el registro del IEEE
tools/empaquetar.sh     compila y genera el binario único de una versión
tools/anonimizar.py     seudonimiza capturas de terreno antes de compartirlas
```

### Tabla de fabricantes (OUI)

`src/rules/oui_tabla.h` se genera a partir del registro público del IEEE:

```bash
python3 -I tools/generar_oui.py
```

Para agregar un fabricante, edita la lista `FABRICANTES` del script y vuelve a ejecutarlo.

### Verificar que es solo receptor

El firmware no incluye funciones para transmitir tramas WiFi arbitrarias, ni para anunciarse o conectarse por BLE: NimBLE se compila solo con el rol de observador. Se puede comprobar en el binario:

```bash
pio run
nm -C .pio/build/esp32-lcd19/firmware.elf | grep -ciE "esp_wifi_80211_tx|ble_gap_adv_start|ble_gap_connect"
# debe imprimir 0
```

Si tu `nm` no reconoce el archivo (por ejemplo, en Windows), usa el que trae el compilador del ESP32: `~/.platformio/packages/toolchain-xtensa-esp32/bin/xtensa-esp32-elf-nm`.

### Publicar una versión

1. Cambia `BUGSWEEP_VERSION` en `platformio.ini` y agrega la entrada en `CHANGELOG.md`.
2. Genera el binario: `tools/empaquetar.sh` → `dist/bugsweep-v<versión>-esp32-lcd19.bin` y su `.sha256`.
3. Haz commit, crea la etiqueta y súbela: `git tag -a v<versión> -m "..." && git push origin v<versión>`.
4. En GitHub: **Releases → Draft a new release**, elige la etiqueta, pega la entrada del CHANGELOG y adjunta el `.bin` y el `.sha256`. Si es beta, marca **Set as a pre-release**.

## Documentación

| Documento | Contenido |
|---|---|
| [docs/MANUAL.md](docs/MANUAL.md) | **Manual de usuario**: uso del aparato e interpretación de resultados |
| [docs/FICHA_DISPOSITIVO.md](docs/FICHA_DISPOSITIVO.md) | Ficha técnica: chip, flash, radio, pantalla, GPIO y firmware de fábrica |
| [docs/ANTECEDENTES.md](docs/ANTECEDENTES.md) | Hardware, decisiones, ruta de aprendizaje y bitácora |
| [docs/PRD.md](docs/PRD.md) | Requisitos del producto |
| [docs/PLAN.md](docs/PLAN.md) | Plan de trabajo por fases |
| [docs/instrucciones_test.md](docs/instrucciones_test.md) | Casos de prueba (T01 a T28) y dispositivos para probar |
| [docs/PRUEBAS.md](docs/PRUEBAS.md) | Cómo capturar, seudonimizar y compartir pruebas de terreno |
| [CHANGELOG.md](CHANGELOG.md) | Historial de versiones |

## Uso responsable

Este proyecto es **solo receptor**: no transmite tramas, no se conecta a redes ajenas y no descifra contenido. Úsalo para proteger tu privacidad en espacios que ocupas legítimamente.

Las direcciones MAC de terceros se tratan como datos personales: solo se guardan en RAM y no se almacenan ni se envían a ningún lado. Revisa la legislación local antes de usarlo.

## Licencia

[MIT](LICENSE) © 2026 Gabo Araya
