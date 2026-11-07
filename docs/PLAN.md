# Plan de trabajo — ESP32-N4XX-BugSweep

Cada paso es pequeño y se puede comprobar: tiene un resultado visible en la pantalla o en el monitor serie. No se avanza al siguiente paso hasta que el anterior funciona en el hardware.

Leyenda: ✅ hecho · 🟡 en curso · ⬜ pendiente

## Fase 0 — Preparación
- ✅ Dar permisos al puerto serie e instalar esptool.
- ✅ Identificar el chip y la memoria flash.
- ✅ Respaldar el firmware de fábrica.
- ✅ Identificar la pantalla y su pinout.
- ✅ Instalar VS Code y PlatformIO.
- ✅ Escribir la documentación inicial (README, ANTECEDENTES, PRD, PLAN y ficha del dispositivo).
- ✅ Elegir el nombre del proyecto y la licencia (MIT).

## Fase 1 — Hola mundo con pantalla
- ✅ **Paso 1:** pantalla, colores y contador de segundos (`src/main.cpp`).
  - Comprobar: las barras se ven roja, verde, azul y blanca, y el monitor serie muestra `[paso1] vivo N s`.
  - Ajustar si hace falta: la polaridad de la retroiluminación, la inversión de colores, el orden RGB/BGR y el offset.
- ✅ **Paso 1b:** leer el botón BOOT (GPIO0), distinguiendo pulsación corta y larga, y mostrar el resultado en pantalla.
- **Aprendes:** `setup()` y `loop()`, el bus SPI, `platformio.ini` y la lectura de entradas digitales.

## Fase 2 — Escáner WiFi
- ✅ Listar las redes con SSID, canal y RSSI, ordenadas por intensidad de señal, usando **escaneo pasivo** (`src/wifi/escaner_wifi.*`).
- ✅ Desplazarse por la lista con el botón: corto para bajar, largo para escanear de nuevo.
- ✅ Marcar las redes ocultas y las abiertas.
- ⬜ **Antes de la fase 6:** cambiar a la tabla de particiones `huge_app.csv`. Con el WiFi, el programa ya ocupa el 59 % de los 1.25 MB disponibles, y el stack de BLE suma unos 400 a 500 KB.
- **Aprendes:** `WiFi.scanNetworks()`, qué es el RSSI (en dBm) y qué son los canales de 2.4 GHz.

## Fase 3 — Reglas de sospecha
- 🟡 Lista de patrones de SSID de cámaras, con puntaje y motivo (`src/rules/sospecha.cpp`).
- 🟡 Tabla OUI generada desde el registro oficial del IEEE con `tools/generar_oui.py`: 757 prefijos de 24 fabricantes, en flash.
- 🟡 Puntaje de 0 a 100 con sus motivos. Niveles `!!` ALTA, `!` MEDIA y `?` BAJA (símbolo + texto + color).
- 🟡 Escaneo continuo, cursor y vista de detalle que se actualiza sola.
- ⬜ Calibrar los pesos con dispositivos reales (cámara de prueba y redes del entorno).
- **Aprendes:** qué es un OUI, la memoria flash frente a la RAM, y cómo separar los datos del código.

## Fase 4 — Modo promiscuo
- ⬜ Capturar tramas de gestión y de datos con `esp_wifi_set_promiscuous_rx_cb`.
- ⬜ Saltar entre los canales 1 a 13.
- ⬜ Tabla de estaciones: MAC, punto de acceso (BSSID), bytes por segundo y RSSI.
- ⬜ Detectar el perfil de streaming: tasa de subida alta y sostenida.
- **Aprendes:** el formato de las tramas 802.11, las funciones de *callback* que corren en una interrupción (ISR), las colas de FreeRTOS y la concurrencia.

## Fase 5 — Correlación y localizador
- ⬜ Fijar un dispositivo y mostrar un gráfico de su tráfico en tiempo real.
- ⬜ Fijar un dispositivo y mostrar una barra grande de RSSI con su tendencia (suavizado por media móvil).
- **Aprendes:** filtrado de señales, cómo dibujar sin parpadeo (*sprites*) y cómo diseñar la interfaz para un solo botón.

## Fase 6 — BLE
- ⬜ Escanear dispositivos BLE con NimBLE.
- ⬜ Detectar rastreadores (AirTag, Tile, SmartTag) por sus datos de fabricante.
- ⬜ Coordinar el uso compartido de la radio entre WiFi y BLE.
- **Aprendes:** los anuncios BLE, las direcciones aleatorias y la coexistencia de radios.

## Fase 7 — Pulido
- ⬜ Puntuación de sospecha (0–100) con sus motivos.
- ⬜ Salida serie en JSON por línea.
- ⬜ Revisión guiada para personas sin experiencia, paso a paso en la pantalla.
- ⬜ Pruebas con una cámara de referencia (un ESP32-CAM propio) y medición de las métricas del PRD.
- ⬜ Integración continua: compilar en GitHub Actions con PlatformIO.
- ⬜ Publicar la versión v1.0.

## Organización del código (a medida que crezca)

```
src/
  main.cpp          # arranque y máquina de estados de la interfaz
  ui/               # pantallas y botón
  wifi_scan/        # escaneo de puntos de acceso
  sniffer/          # modo promiscuo y estadísticas por estación
  ble_scan/         # BLE y rastreadores
  rules/            # patrones de SSID, tabla OUI y puntuación
```
