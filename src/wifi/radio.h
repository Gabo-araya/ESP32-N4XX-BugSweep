// Inicialización común de la radio WiFi para escáner y sniffer.
#pragma once
#include <WiFi.h>

namespace radio {
// Modo estación SIN conectarse jamás: no se guardan credenciales ni hay reconexión
// automática (la NVS de fábrica podría tener una red guardada).
inline void iniciar() {
  WiFi.persistent(false);
  WiFi.setAutoReconnect(false);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(false, false);
}
}  // namespace radio
