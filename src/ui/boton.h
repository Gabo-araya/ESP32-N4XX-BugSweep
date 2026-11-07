// Botón con antirrebote que distingue pulsación corta y larga.
// Pensado para manejar toda la interfaz con un solo botón (BOOT = GPIO0).
#pragma once
#include <Arduino.h>

class Boton {
public:
  enum class Evento { Ninguno, Corto, Largo };

  // pin: GPIO del botón (activo en LOW, con pull-up interno).
  // msLargo: tiempo que hay que mantenerlo apretado para que cuente como pulsación larga.
  explicit Boton(uint8_t pin, uint16_t msLargo = 600, uint8_t msRebote = 30);

  void begin();

  // Llamar en cada vuelta de loop(). Devuelve a lo más un evento por pulsación:
  //  - Largo: apenas se cumple msLargo, sin esperar a que se suelte (respuesta inmediata).
  //  - Corto: al soltar, si no se alcanzó msLargo.
  Evento leer();

  bool apretado() const { return estable_ == LOW; }

private:
  const uint8_t pin_;
  const uint16_t msLargo_;
  const uint8_t msRebote_;
  int crudo_ = HIGH;          // última lectura sin filtrar
  int estable_ = HIGH;        // estado ya filtrado del rebote
  uint32_t tCambio_ = 0;      // cuándo cambió por última vez la lectura cruda
  uint32_t tApretado_ = 0;    // cuándo empezó la pulsación actual
  bool largoEmitido_ = false; // evita emitir Corto al soltar después de un Largo
};
