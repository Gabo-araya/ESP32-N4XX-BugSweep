// Revisión guiada: secuencia fija de pasos con duración e instrucciones.
// Solo lleva el tiempo; quien la usa (main.cpp) enciende la radio de cada paso.
#pragma once
#include <Arduino.h>

struct PasoGuia {
  const char *titulo;
  const char *linea1;
  const char *linea2;
  uint32_t ms;
};

class Revision {
public:
  static constexpr uint8_t PASOS = 3;
  static const PasoGuia &paso(uint8_t i);

  void iniciar() { indice_ = 0; tPaso_ = millis(); activa_ = true; }
  void cancelar() { activa_ = false; }
  bool activa() const { return activa_; }
  uint8_t indice() const { return indice_; }
  bool terminada() const { return indice_ >= PASOS; }
  float fraccion() const;       // avance del paso actual, 0..1

  // Llamar en cada loop(). true = el paso actual terminó y se pasó al siguiente
  // (o la revisión terminó: ver terminada()).
  bool avanzar();

private:
  bool activa_ = false;
  uint8_t indice_ = 0;
  uint32_t tPaso_ = 0;
};
