#include "revision.h"

namespace {
// Duraciones pensadas para cada técnica:
//  - redes: ~3 barridos pasivos de ~4 s
//  - dispositivos: 5 ventanas de ~5.2 s (el perfil de video necesita 3 seguidas)
//  - bluetooth: los anuncios BLE se repiten cada 0.1-2 s; 15 s basta para verlos
const PasoGuia PASOS_GUIA[Revision::PASOS] = {
    {"Redes WiFi",          "Busco redes creadas por camaras.", "Quedate en el centro del lugar.", 12000},
    {"Dispositivos WiFi",   "Escucho cuanto transmite cada",    "equipo. Quedate quieto ~30 s.",   27000},
    {"Bluetooth",           "Busco rastreadores (AirTag,",      "Tile, SmartTag...).",             15000},
};
}  // namespace

const PasoGuia &Revision::paso(uint8_t i) { return PASOS_GUIA[i < PASOS ? i : PASOS - 1]; }

float Revision::fraccion() const {
  if (!activa_ || terminada()) return 1.0f;
  const float f = (float)(millis() - tPaso_) / paso(indice_).ms;
  return f > 1.0f ? 1.0f : f;
}

bool Revision::avanzar() {
  if (!activa_ || terminada()) return false;
  if (millis() - tPaso_ < paso(indice_).ms) return false;
  indice_++;
  tPaso_ = millis();
  return true;
}
