#include "seguimiento.h"

void Seguimiento::iniciar(const uint8_t mac[6]) {
  memcpy(mac_, mac, 6);
  bytes_ = 0;
  sumaRssi_ = 0;
  cuentaRssi_ = 0;
  tMuestra_ = millis();
  tUltima_ = 0;
  primera_ = true;
  inicio_ = n_ = 0;
  activo_ = true;
}

void Seguimiento::registrar(uint16_t largo, int8_t rssi) {
  bytes_ += largo;
  sumaRssi_ += rssi;
  cuentaRssi_++;
  tUltima_ = millis();
}

bool Seguimiento::tick() {
  const uint32_t ahora = millis();
  const uint32_t dt = ahora - tMuestra_;
  if (!activo_ || dt < MS_MUESTRA) return false;

  // Historial circular de bytes/s.
  const uint32_t bps = (uint64_t)bytes_ * 1000 / dt;
  if (n_ < MUESTRAS) hist_[(inicio_ + n_++) % MUESTRAS] = bps;
  else { hist_[inicio_] = bps; inicio_ = (inicio_ + 1) % MUESTRAS; }

  // Dos promedios del RSSI: uno rápido (~0.5 s) y uno lento (~3 s).
  // Si el rápido supera al lento, la señal viene subiendo: nos acercamos.
  if (cuentaRssi_) {
    const float m = (float)sumaRssi_ / cuentaRssi_;
    if (primera_) { rapido_ = lento_ = m; primera_ = false; }
    rapido_ += 0.5f * (m - rapido_);
    lento_ += 0.08f * (m - lento_);
  }
  bytes_ = 0;
  sumaRssi_ = 0;
  cuentaRssi_ = 0;
  tMuestra_ = ahora;
  return true;
}

int8_t Seguimiento::tendencia() const {
  if (!conSenal()) return 0;
  const float d = rapido_ - lento_;
  return d > 2.5f ? 1 : d < -2.5f ? -1 : 0;
}

float Seguimiento::promedio(size_t desde, size_t hasta) const {
  if (hasta <= desde) return 0;
  uint64_t s = 0;
  for (size_t i = desde; i < hasta; i++) s += muestra(i);
  return (float)s / (hasta - desde);
}

float Seguimiento::bpsActual() const {
  return promedio(n_ > RECIENTES ? n_ - RECIENTES : 0, n_);
}

float Seguimiento::bpsBase() const {
  return n_ > RECIENTES ? promedio(0, n_ - RECIENTES) : 0;
}

bool Seguimiento::pico() const {
  if (n_ < 20) return false;                       // necesita ~5 s de base
  const float base = bpsBase(), ahora = bpsActual();
  return ahora > 2 * base && ahora > base + 5000;  // el doble, y al menos +5 KB/s
}
