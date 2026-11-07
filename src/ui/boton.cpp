#include "boton.h"

Boton::Boton(uint8_t pin, uint16_t msLargo, uint8_t msRebote)
    : pin_(pin), msLargo_(msLargo), msRebote_(msRebote) {}

void Boton::begin() {
  pinMode(pin_, INPUT_PULLUP);
  crudo_ = estable_ = digitalRead(pin_);
  tCambio_ = millis();
}

Boton::Evento Boton::leer() {
  const uint32_t ahora = millis();
  const int lectura = digitalRead(pin_);

  // Antirrebote: aceptar un cambio solo si la lectura se mantiene msRebote_ ms.
  if (lectura != crudo_) {
    crudo_ = lectura;
    tCambio_ = ahora;
  }
  if (crudo_ != estable_ && ahora - tCambio_ >= msRebote_) {
    estable_ = crudo_;
    if (estable_ == LOW) {            // se apretó
      tApretado_ = ahora;
      largoEmitido_ = false;
    } else if (!largoEmitido_) {      // se soltó antes de msLargo_
      return Evento::Corto;
    }
  }

  if (estable_ == LOW && !largoEmitido_ && ahora - tApretado_ >= msLargo_) {
    largoEmitido_ = true;
    return Evento::Largo;
  }
  return Evento::Ninguno;
}
