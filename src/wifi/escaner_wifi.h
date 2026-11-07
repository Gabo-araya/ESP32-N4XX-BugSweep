// Escáner WiFi PASIVO: solo escucha beacons, no transmite probe requests.
// No bloquea: se inicia con iniciar() y se consulta con actualizar() en cada loop().
#pragma once
#include <Arduino.h>
#include <vector>

struct RedWifi {
  String ssid;          // vacío si la red es oculta
  uint8_t bssid[6];     // MAC del punto de acceso
  int8_t rssi;          // dBm: -30 muy cerca, -90 muy lejos
  uint8_t canal;        // 1..13
  bool abierta;         // sin cifrado
  bool oculta() const { return ssid.isEmpty(); }
};

class EscanerWifi {
public:
  static constexpr size_t MAX_REDES = 40;     // tope para acotar el uso de RAM
  static constexpr uint32_t MS_POR_CANAL = 300; // ~3 beacons por red (cada ~102 ms)

  void begin();
  void iniciar();          // lanza un barrido de los canales 1..13
  bool actualizar();       // true justo cuando termina un barrido con resultados nuevos
  bool escaneando() const { return escaneando_; }

  const std::vector<RedWifi> &redes() const { return redes_; }  // ordenadas por RSSI
  uint32_t barridos() const { return barridos_; }

private:
  std::vector<RedWifi> redes_;
  bool escaneando_ = false;
  uint32_t barridos_ = 0;
};
