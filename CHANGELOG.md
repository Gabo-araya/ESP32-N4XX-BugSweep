# Cambios

Formato basado en [Keep a Changelog](https://keepachangelog.com/es-ES/1.1.0/). Versionado [SemVer](https://semver.org/lang/es/).

## [0.9.0-beta] — 2026-11-07

Primera versión pública de prueba. **Beta**: las funciones están completas, pero los umbrales de detección no se han calibrado con cámaras ni rastreadores reales (ver [docs/PRUEBAS.md](docs/PRUEBAS.md)).

### Agregado
- **Revisión guiada** de ~1 minuto desde la pantalla de inicio (pulsación larga): redes → dispositivos → Bluetooth → resultado unificado.
- **Modo REDES**: escaneo WiFi pasivo (sin *probe requests*), con detección de nombres de red típicos de cámaras y redes ocultas.
- **Modo DISPOSITIVOS**: sniffer en modo promiscuo con saltos de canal; tráfico de subida y bajada por equipo; perfil de video (≥ 15 KB/s sostenidos ~15 s).
- **Modo BLUETOOTH**: escaneo BLE pasivo; detección de Apple Find My (separado / cerca del dueño), Tile, Samsung SmartTag y Google FMDN (posible); aviso "te acompaña" a los 10 minutos.
- **Rastreo**: localizador con tendencia (MAS CERCA / MAS LEJOS) y, en WiFi, gráfico de tráfico con detección de pico para la prueba de correlación (mover la mano frente a la cámara).
- **Puntaje de sospecha** de 0 a 100 con motivos explicados; tabla de 757 prefijos OUI generada desde el registro del IEEE.
- Interfaz accesible con un solo botón: alto contraste, símbolo + texto + color, marco de selección.
- Monitor serie con marca de tiempo (`[  123s]`) en cada evento.
- [Manual de usuario](docs/MANUAL.md), [ficha del dispositivo](docs/FICHA_DISPOSITIVO.md), PRD y plan de trabajo.
- Herramientas: `tools/generar_oui.py`, `tools/empaquetar.sh` (binario único) y `tools/anonimizar.py` (capturas de terreno).

### Seguridad
- Solo recepción, verificable en el binario: sin `esp_wifi_80211_tx`; NimBLE compilado solo como observador.
- La radio nunca se conecta a redes (sin persistencia ni reconexión automática).
- Nada se guarda: todo vive en la RAM.

### Limitaciones conocidas
- Solo 2.4 GHz. No detecta cámaras de 5 GHz, solo SD, analógicas ni celulares.
- El ESP32 (1x1) subestima el tráfico MIMO de celulares y notebooks modernos.
- Firma de Google FMDN sin verificar con un rastreador real.
- Textos sin tildes (las fuentes integradas solo tienen ASCII).
