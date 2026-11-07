// Seguimiento de UN dispositivo (modo rastreo): señal suavizada, tendencia
// (acercándose / alejándose) e historial de bytes transmitidos por segundo.
#pragma once
#include <Arduino.h>

class Seguimiento {
public:
  static constexpr uint16_t MS_MUESTRA = 250;    // 4 muestras por segundo
  static constexpr size_t MUESTRAS = 90;         // 22.5 s de historia
  static constexpr size_t RECIENTES = 8;         // "ahora" = últimos 2 s

  void iniciar(const uint8_t mac[6]);
  void detener() { activo_ = false; }
  bool activo() const { return activo_; }
  bool esObjetivo(const uint8_t mac[6]) const { return activo_ && memcmp(mac, mac_, 6) == 0; }

  void registrar(uint16_t largo, int8_t rssi);   // trama TRANSMITIDA por el objetivo
  bool tick();                                   // true al cerrar una muestra

  bool conSenal() const { return activo_ && millis() - tUltima_ < 3000 && !primera_; }
  int8_t rssi() const { return (int8_t)lroundf(rapido_); }
  int8_t tendencia() const;          // +1 acercándose, -1 alejándose, 0 estable
  float bpsActual() const;           // promedio de los últimos 2 s
  float bpsBase() const;             // promedio del resto del historial
  bool pico() const;                 // tráfico actual muy sobre la base
  size_t cantidad() const { return n_; }
  uint32_t muestra(size_t i) const { return hist_[(inicio_ + i) % MUESTRAS]; }  // 0 = más antigua

private:
  float promedio(size_t desde, size_t hasta) const;

  uint8_t mac_[6] = {};
  bool activo_ = false;
  uint32_t bytes_ = 0;               // muestra en curso
  int32_t sumaRssi_ = 0;
  uint16_t cuentaRssi_ = 0;
  uint32_t tMuestra_ = 0, tUltima_ = 0;
  bool primera_ = true;
  float rapido_ = -100, lento_ = -100;   // promedios exponenciales del RSSI
  uint32_t hist_[MUESTRAS] = {};
  size_t inicio_ = 0, n_ = 0;
};
