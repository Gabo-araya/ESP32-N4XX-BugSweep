# PRD — ESP32-N4XX-BugSweep

Detector pasivo de dispositivos inalámbricos ocultos.

| | |
|---|---|
| **Estado** | Borrador v0.1 |
| **Fecha** | 2026-11-07 |
| **Hardware** | ESP32-D0WD-V3 + LCD ST7789 de 1.9" (ver la [ficha del dispositivo](FICHA_DISPOSITIVO.md)) |

## 1. Problema

En alojamientos de arriendo temporal, hoteles, oficinas o vehículos pueden esconderse cámaras y micrófonos que transmiten por WiFi o Bluetooth. Las personas no tienen una forma barata y portátil de revisar si los hay. Las aplicaciones de celular están limitadas por el sistema operativo: no pueden escuchar el tráfico WiFi en modo promiscuo.

## 2. Objetivo

Construir un dispositivo de bolsillo, de menos de USD 20, que **escuche de forma pasiva** las señales de 2.4 GHz y BLE, **señale los dispositivos sospechosos** y **ayude a ubicarlos físicamente**, mostrando todo en su propia pantalla.

## 3. Usuarios

- **Principal:** una persona sin conocimientos técnicos que llega a un lugar y quiere revisarlo en 5 a 10 minutos.
- **Secundario:** una persona técnica que quiere ver los datos en bruto (MAC, canal, RSSI, tráfico) por el puerto serie.

## 4. Alcance

### 4.1 Requisitos funcionales

| ID | Requisito | Prioridad |
|---|---|---|
| F1 | Escanear redes WiFi de 2.4 GHz: SSID, BSSID, canal, RSSI y si la red está oculta | Must |
| F2 | Escanear dispositivos BLE: dirección, nombre, RSSI y datos del fabricante | Must |
| F3 | Marcar como sospechosas las redes con SSID típicos de cámara, según una lista de patrones | Must |
| F4 | Identificar el fabricante por OUI con una tabla local, sin conexión a internet | Must |
| F5 | Modo promiscuo: listar estaciones (clientes WiFi) con su volumen de tráfico por segundo, saltando entre los canales 1 a 13 | Must |
| F6 | Señalar las estaciones con tráfico de subida alto y sostenido (perfil de streaming de video) | Must |
| F7 | **Modo correlación**: fijar un dispositivo y mostrar su tráfico en tiempo real, para que la persona pruebe moviendo la mano o apagando la luz | Should |
| F8 | **Modo localizador**: fijar un dispositivo y mostrar su RSSI como barra grande, con números y tendencia ("más cerca" / "más lejos") | Should |
| F9 | Detectar rastreadores BLE conocidos (AirTag, Tile, SmartTag) | Should |
| F10 | Manejar todo con el botón BOOT: pulsación corta para avanzar y pulsación larga para seleccionar | Must |
| F11 | Enviar todos los eventos por el puerto serie en formato legible (JSON por línea) | Should |
| F12 | Asignar a cada dispositivo una puntuación de sospecha (0–100) con sus motivos explicados | Could |

### 4.2 Fuera de alcance (no-objetivos)

- ❌ **Cualquier transmisión activa**: deauth, spam de beacons, sondeo agresivo o conexión a redes ajenas. El dispositivo es **solo receptor**.
- ❌ Descifrar o capturar el contenido de las tramas. Solo se usan metadatos: dirección MAC, tamaño y tiempo de las tramas.
- ❌ Detectar WiFi de 5 GHz, radiofrecuencia analógica o redes celulares (son límites físicos del hardware).
- ❌ Guardar datos de forma permanente o enviarlos a la nube.

## 5. Requisitos no funcionales

### 5.1 Seguridad y privacidad
- **Pasivo por diseño.** El código no incluye funciones para transmitir tramas arbitrarias (`esp_wifi_80211_tx` no se usa).
- **Sin persistencia.** Las direcciones MAC de terceros son datos personales: se guardan solo en RAM y se borran al apagar.
- Ninguna dirección MAC real ni credencial se sube al repositorio. Las pruebas usan datos ficticios.

### 5.2 Accesibilidad
- Alto contraste: texto claro sobre fondo negro, con una relación de contraste mayor o igual a 7:1.
- **El color nunca es el único indicador.** Cada alerta lleva además texto o un símbolo (`!`, `?`, `OK`).
- Tamaño de texto mínimo legible a 40 cm: fuente 2 de TFT_eSPI (16 px) o mayor. Los datos clave usan la fuente 4 (26 px).
- Se maneja con un solo botón, con pulsaciones largas tolerantes (más de 600 ms).
- Una salida serie paralela permite usar el dispositivo con un lector de pantalla en el computador.
- Paleta segura para el daltonismo: amarillo y azul o cian como colores principales. No se depende del contraste rojo/verde.

### 5.3 Rendimiento
- Refrescar la pantalla en menos de 200 ms en el modo localizador.
- Recorrer todos los canales en menos de 3 s.
- Funcionar con 320 KB de RAM, sin PSRAM. Las tablas (OUI, patrones) se guardan en la flash (`PROGMEM`).

## 6. Métricas de éxito

1. Detecta una cámara WiFi de prueba (por ejemplo, una cámara IP barata o un ESP32-CAM) a 5 m en menos de 60 s.
2. El modo correlación distingue la cámara de prueba frente a otros 10 dispositivos domésticos.
3. El modo localizador permite ubicar la cámara de prueba en una habitación en menos de 3 minutos.
4. Una persona sin experiencia completa una revisión guiada sin leer un manual.

## 7. Riesgos

| Riesgo | Mitigación |
|---|---|
| Falsos positivos (un televisor o un celular que hace streaming) | Puntuación con motivos explicados y modo correlación para confirmar |
| Falsos negativos (dispositivos en 5 GHz, solo SD o analógicos) | Avisarlo en pantalla y en el README. Sugerir métodos complementarios |
| Direcciones MAC aleatorias (BLE, celulares) | Agrupar por otros datos (fabricante, patrón de anuncios). Documentar la limitación |
| Uso indebido (vigilar a terceros) | Funciona solo en modo pasivo. Aviso legal en el README |
| WiFi y BLE comparten la radio | Alternar los escaneos por turnos |
