#include "escaner_wifi.h"
#include <WiFi.h>
#include <algorithm>

void EscanerWifi::begin() {
  WiFi.mode(WIFI_STA);   // modo estación, pero nunca nos conectamos a ninguna red
  WiFi.disconnect();
  redes_.reserve(MAX_REDES);
}

void EscanerWifi::iniciar() {
  if (escaneando_) return;
  // async=true, mostrar ocultas=true, PASIVO=true, tiempo por canal
  WiFi.scanNetworks(true, true, true, MS_POR_CANAL);
  escaneando_ = true;
}

bool EscanerWifi::actualizar() {
  if (!escaneando_) return false;
  const int16_t n = WiFi.scanComplete();
  if (n == WIFI_SCAN_RUNNING) return false;

  escaneando_ = false;
  if (n < 0) {               // WIFI_SCAN_FAILED
    WiFi.scanDelete();
    return false;
  }

  redes_.clear();
  for (int i = 0; i < n && redes_.size() < MAX_REDES; i++) {
    RedWifi r;
    r.ssid = WiFi.SSID(i);
    memcpy(r.bssid, WiFi.BSSID(i), 6);
    r.rssi = WiFi.RSSI(i);
    r.canal = WiFi.channel(i);
    r.abierta = WiFi.encryptionType(i) == WIFI_AUTH_OPEN;
    redes_.push_back(r);
  }
  WiFi.scanDelete();         // libera la memoria interna del escaneo

  std::sort(redes_.begin(), redes_.end(),
            [](const RedWifi &a, const RedWifi &b) { return a.rssi > b.rssi; });
  barridos_++;
  return true;
}
