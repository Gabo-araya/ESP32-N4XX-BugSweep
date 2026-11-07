# Ficha técnica del dispositivo

Placa de desarrollo **ESP32 con LCD de 1.9"**, usada en ESP32-N4XX-BugSweep.

Cada dato indica su fuente, con esta leyenda:

| Marca | Fuente |
|---|---|
| 🔬 | Medido en la placa (`esptool`, `lsusb`) |
| 💾 | Obtenido del respaldo de la flash de fábrica |
| 📷 | Leído en la serigrafía de la foto del producto |
| 🏷️ | Página del vendedor (poco confiable) |
| 📘 | Hoja de datos de Espressif para el ESP32 |
| ❓ | Por verificar |

---

## 1. Identificación

| Campo | Valor | Fuente |
|---|---|---|
| Nombre comercial | ESP32 WiFi Bluetooth Development Board 1.4/1.9 Inch LCD CH340 Type-C | 🏷️ |
| Vendedor | DIY Electronics Module 999 Store (AliExpress) | 🏷️ |
| Variante | Con pantalla de 1.9" (también se vende una de 1.4") | 🏷️ 📷 |
| Dimensiones | 63.7 × 31.5 mm | 🏷️ |
| Peso | ~20 g | 🏷️ |
| Fabricante de la placa / número de modelo | No informado | ❓ |

---

## 2. Microcontrolador

| Campo | Valor | Fuente |
|---|---|---|
| Chip | **ESP32-D0WD-V3** | 🔬 |
| Revisión del silicio | v3.1 | 🔬 |
| CPU | 2 núcleos Xtensa LX6, hasta 240 MHz | 🔬 📘 |
| Coprocesador de bajo consumo | ULP ("LP Core" en el reporte de esptool) | 🔬 📘 |
| Cristal | 40 MHz | 🔬 |
| ROM interna | 448 KB | 📘 |
| SRAM interna | 520 KB (de los cuales unos 320 KB quedan disponibles para datos en Arduino) | 📘 |
| SRAM del RTC | 16 KB (se conserva durante el *deep sleep*) | 📘 |
| PSRAM | **No tiene** (el chip es D0WD y no hay memoria externa) | 📘 |
| eFuse | Calibración de Vref grabada, *Coding Scheme* "None" | 🔬 |
| Dirección MAC | Única por placa. Se lee con `esptool flash-id`. **No publicarla** | 🔬 |

### 2.1 Mejoras de la revisión V3

Comparada con las revisiones v0 y v1, la V3:

- Corrige problemas de la caché con PSRAM. No afecta a esta placa, que no tiene PSRAM.
- Agrega **Secure Boot V2** (con firmas RSA-PSS).
- Endurece la protección contra ataques de *fault injection* sobre el arranque seguro.

Fuente: 📘 (*ESP32 ECO V3 User Guide*).

### 2.2 Seguridad por hardware

| Función | Disponible | Fuente |
|---|---|---|
| Secure Boot V2 | Sí | 📘 |
| Cifrado de la flash (AES-256) | Sí | 📘 |
| Aceleradores criptográficos AES, SHA-2, RSA y ECC | Sí | 📘 |
| Generador de números aleatorios por hardware | Sí (es de verdad aleatorio solo cuando la radio WiFi o BT está activa) | 📘 |
| Estado actual en esta placa | Secure Boot y cifrado de la flash **desactivados** (de fábrica) | 🔬 |

> ⚠️ Activar Secure Boot o el cifrado de la flash quema eFuses de forma **irreversible**. No lo hagas sin un plan.

---

## 3. Memoria flash

| Campo | Valor | Fuente |
|---|---|---|
| Capacidad | **4 MB** (32 Mbit) | 🔬 |
| ID JEDEC | Fabricante `0x0B`, dispositivo `0x4016` | 🔬 |
| Chip probable | XTX Technology XT25F32B (`0x0B` corresponde a XTX) | 🔬 ❓ |
| Voltaje | 3.3 V, fijado por un *strapping pin* | 🔬 |
| Modo y velocidad de fábrica | DIO a 80 MHz (cabecera del bootloader) | 💾 |

### 3.1 Tabla de particiones de fábrica

Es la tabla `default.csv` estándar de Arduino-ESP32:

| Nombre | Tipo | Offset | Tamaño | Contenido de fábrica |
|---|---|---|---|---|
| (bootloader) | — | `0x1000` | 28 KB | Bootloader de ESP-IDF |
| (tabla de particiones) | — | `0x8000` | 4 KB | — |
| `nvs` | data/nvs | `0x9000` | 20 KB | Configuración del WiFi en modo estación (ver la nota de privacidad) |
| `otadata` | data/ota | `0xE000` | 8 KB | Indica qué aplicación arranca |
| `app0` | app/ota_0 | `0x10000` | 1280 KB | Sketch de Arduino (ver la sección 3.2) |
| `app1` | app/ota_1 | `0x150000` | 1280 KB | Restos de otro firmware; no es una imagen válida |
| `spiffs` | data/spiffs | `0x290000` | 1408 KB | Vacía |
| `coredump` | data/coredump | `0x3F0000` | 64 KB | Vacía |

Fuente: 💾

### 3.2 Firmware de fábrica

- `app0` contiene un sketch compilado con el **IDE de Arduino en Windows**, con el núcleo **arduino-esp32 2.0.11** (ESP-IDF v4.4.5), el **12 de junio de 2023**.
- La partición también contiene textos de **ESP-AT**, el firmware de comandos AT de Espressif (`+BLEADVDATA`, `AT_BLE`…). Lo más probable es que la placa trajera ESP-AT de origen y que después le grabaran el sketch de demostración encima, sin borrar la flash.
- Por el puerto serie a 115200 baudios solo imprime puntos (`.....`). Parece que intenta conectarse a una red WiFi.
- Respaldo completo: `docs/respaldo_fabrica.bin` (4 MB). **Este archivo está excluido de git.**

> 🔒 **Privacidad:** la partición NVS del respaldo tiene las claves `sta.ssid` y `sta.pswd`. Puede contener el nombre y la contraseña de una red WiFi grabados en la fábrica. Esta es otra razón para no publicar el `.bin`.

Para restaurar el firmware de fábrica:

```bash
uvx esptool --port /dev/ttyUSB0 write-flash 0 docs/respaldo_fabrica.bin
```

---

## 4. Radio

| Campo | Valor | Fuente |
|---|---|---|
| WiFi | IEEE 802.11 b/g/n, **solo 2.4 GHz** (2412–2484 MHz, canales 1–14) | 📘 🏷️ |
| Velocidad máxima de WiFi | 150 Mbps (802.11n, HT40) | 📘 |
| Modos de WiFi | Estación, punto de acceso, ambos a la vez y **modo promiscuo** (*sniffer*) | 📘 |
| Bluetooth | v4.2: Classic (BR/EDR) y **BLE** | 📘 |
| Potencia de transmisión | Hasta unos +20 dBm (802.11b) | 📘 |
| Sensibilidad de recepción | Unos −97 dBm (802.11b a 1 Mbps) | 📘 |
| Antena | En la PCB (antena impresa), ganancia de 2 dBi | 🏷️ |
| Conector para antena externa | No se ve | 📷 ❓ |
| Radio compartida | WiFi y BT usan **la misma radio**, que se reparte por turnos (coexistencia) | 📘 |

> ⚠️ Las páginas de marketing afirman que soporta 5 GHz. **Es falso.**

**Notas para BugSweep:**
- El modo promiscuo captura tramas de gestión, de control y de datos de **un canal a la vez**. Para cubrir toda la banda hay que saltar entre canales.
- No se puede capturar WiFi y escanear BLE al mismo tiempo con rendimiento completo. Hay que alternarlos.
- La antena impresa es omnidireccional. Para ubicar un dispositivo, sirve la tendencia del RSSI, no una dirección.

---

## 5. Pantalla

| Campo | Valor | Fuente |
|---|---|---|
| Tamaño | 1.9" | 📷 🏷️ |
| Tecnología | LCD IPS a color | 🏷️ |
| Controlador | **ST7789** (confirmado: funciona con el driver ST7789 de TFT_eSPI) | 🔬 |
| Resolución | **170 × 320** píxeles (confirmado) | 🔬 |
| Colores | 65 mil (RGB565, 16 bits) | 📘 (ST7789) |
| Interfaz | SPI, solo escritura (sin MISO) | 📷 |
| Táctil | No | 📷 |
| Retroiluminación | LED controlado por GPIO32 a través de LEDK (cátodo) | 📷 |
| Polaridad de la retroiluminación | **`HIGH` = encendida** (confirmado) | 🔬 |
| Inversión de color / orden RGB-BGR / offset | Valores por defecto de TFT_eSPI para ST7789 de 170×320: colores RGB correctos, sin inversión adicional y sin desplazamiento visible (confirmado) | 🔬 |

### 5.1 Pinout de la pantalla (serigrafía)

| N.º | Serigrafía | Función | GPIO |
|---|---|---|---|
| 1 | GND | Tierra | — |
| 2 | LEDK D32 | Cátodo de la retroiluminación | **32** |
| 3 | LEDA 3V3 | Ánodo de la retroiluminación | 3.3 V |
| 4 | VDD 3V3 | Alimentación del panel | 3.3 V |
| 5 | GND | Tierra | — |
| 6 | GND | Tierra | — |
| 7 | RS D2 | Data/Command (DC) | **2** |
| 8 | CS D15 | Chip Select | **15** |
| 9 | SCL D18 | Reloj SPI (SCLK) | **18** |
| 10 | SDA D23 | Datos SPI (MOSI) | **23** |
| 11 | RST D4 | Reset | **4** |
| 12 | GND | Tierra | — |

Los GPIO 18 y 23 son los pines nativos del bus **VSPI**. Por eso la pantalla puede funcionar a 40 MHz o más sin pasar por la matriz de GPIO.

---

## 6. USB, alimentación y botones

| Campo | Valor | Fuente |
|---|---|---|
| Conector | USB-C | 📷 |
| Puente USB-serie | **CH340** (WCH), VID:PID `1a86:7523`. En Linux aparece como `/dev/ttyUSB0` | 🔬 |
| Velocidad serie | 115200 baudios para el monitor; hasta 921600 o más para grabar | 🏷️ 🔬 |
| Reset automático | Sí: esptool reinicia la placa y la pone en modo de grabación usando las señales DTR y RTS | 🔬 |
| Alimentación | 5 V por USB | 🏷️ |
| Regulador de 3.3 V | No identificado (probablemente un LDO del tipo AMS1117 o ME6211) | ❓ |
| Conector o cargador de batería | No se ve | 📷 ❓ |
| Botón **BOOT** | GPIO0. Si se mantiene apretado al arrancar, entra en modo de grabación. **Mientras el programa corre, se puede usar como botón del usuario** | 📷 📘 |
| Botón **EN** | Reinicia el chip | 📷 |
| LED de usuario | No identificado | ❓ |

---

## 7. GPIO

### 7.1 Pines ocupados o restringidos

| GPIO | Uso o restricción | Fuente |
|---|---|---|
| 0 | Botón BOOT. *Strapping pin* | 📷 📘 |
| 1, 3 | UART0 TX/RX, conectados al CH340 (el puerto serie) | 📘 |
| 2 | Pantalla DC. *Strapping pin* | 📷 📘 |
| 4 | Pantalla RST | 📷 |
| 5 | *Strapping pin* (temporización de SDIO) | 📘 |
| 6–11 | **Reservados para la flash. No usar** | 📘 |
| 12 | *Strapping pin* (voltaje de la flash). Ponerlo en alto al arrancar puede impedir el arranque | 📘 |
| 15 | Pantalla CS. *Strapping pin* | 📷 📘 |
| 18, 23 | Pantalla SCLK y MOSI | 📷 |
| 32 | Retroiluminación de la pantalla | 📷 |
| 34–39 | **Solo entrada**: sin salida y sin resistencias internas de pull-up o pull-down | 📘 |

### 7.2 Pines probablemente libres

13, 14, 16, 17, 19, 21, 22, 25, 26, 27 y 33 (todos de entrada y salida), más 34, 35, 36 y 39 (solo entrada).

❓ Hay que confirmar cuáles de estos llegan a los pines de la placa: la serigrafía de los dos costados largos no se lee en la foto.

**Otras restricciones:**
- **Con el WiFi activo, el ADC2 no funciona.** Afecta a los GPIO 0, 2, 4, 12–15 y 25–27. Para lecturas analógicas hay que usar el ADC1: GPIO 32–39.
- Pines útiles para extensiones de BugSweep:
  - Un zumbador o un motor de vibración en el GPIO 25, 26 o 27.
  - Un segundo botón en el GPIO 13 o 14.
  - Una medición de batería en el GPIO 34 o 35 (ADC1).

---

## 8. Periféricos internos

Datos de la hoja de Espressif (📘):

| Periférico | Cantidad o detalle |
|---|---|
| UART | 3 |
| SPI | 4 (2 libres para el usuario: HSPI y VSPI) |
| I²C | 2 |
| I²S | 2 |
| ADC | 2 de 12 bits (18 canales) |
| DAC | 2 canales de 8 bits (GPIO 25 y 26) |
| Táctil capacitivo | 10 canales |
| PWM | LEDC con 16 canales y MCPWM con 2 unidades |
| RMT | 8 canales (útil para infrarrojo) |
| TWAI (CAN 2.0) | 1 |
| SD/SDIO | Maestro y esclavo |
| MAC de Ethernet | Sí (requiere un chip PHY externo) |
| Temporizadores | 4 de 64 bits y *watchdogs* |

---

## 9. Toolchain verificada

| Herramienta | Versión | Uso |
|---|---|---|
| esptool | 5.6.0 (vía `uvx`) | Identificar el chip, respaldar y grabar |
| PlatformIO | Plataforma espressif32 6.13.0, framework-arduinoespressif32 2.0.17 | Compilar y subir (verificado) |
| TFT_eSPI | `^2.5.43` | Manejar la pantalla |

---

## 10. Pendientes de verificación

- [x] Resolución, offset, inversión y orden de color de la pantalla (paso 1, con foto).
- [x] Polaridad de la retroiluminación en el GPIO32 (`HIGH`).
- [ ] Serigrafía completa de los pines laterales y qué GPIO quedan accesibles.
- [ ] Modelo del regulador de 3.3 V y consumo real (en reposo, con WiFi en modo promiscuo y con la pantalla encendida).
- [ ] Si hay un LED de usuario o un circuito de batería.
- [ ] Una foto de la parte trasera de la placa.
