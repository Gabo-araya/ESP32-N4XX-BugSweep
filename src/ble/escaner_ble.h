// Escáner Bluetooth LE PASIVO (sin scan requests) con detección de rastreadores.
#pragma once
#include <Arduino.h>
#include <vector>
#include "rules/sospecha.h"
#include "sniffer/seguimiento.h"

struct DispositivoBle {
  uint8_t mac[6];
  bool publica = false;          // dirección pública (tiene OUI) o aleatoria
  int8_t rssi = -100;            // suavizado
  char nombre[21] = "";
  uint16_t empresa = 0xFFFF;     // Company ID de los datos de fabricante (0xFFFF = no hay)
  Rastreador rastreador = Rastreador::NINGUNO;
  uint32_t primerMs = 0, ultimoMs = 0;
  uint32_t anuncios = 0;

  uint32_t minutosVisto() const { return (ultimoMs - primerMs) / 60000; }
};

class EscanerBle {
public:
  static constexpr size_t MAX_DISPOSITIVOS = 64;
  static constexpr uint32_t OLVIDAR_MS = 120000;
  static constexpr uint32_t MS_REFRESCO = 2000;   // cada cuánto avisar para redibujar la lista

  void iniciar();              // la primera vez inicializa NimBLE
  void detener();
  void limpiar() { dispositivos_.clear(); }   // olvidar todo lo visto
  bool activo() const { return activo_; }

  // Llamar en cada loop(). true = hay algo nuevo que dibujar (lista cada 2 s,
  // o una muestra nueva si hay un seguimiento activo).
  bool actualizar();

  const std::vector<DispositivoBle> &dispositivos() const { return dispositivos_; }
  Seguimiento &seguimiento() { return seguimiento_; }
  const Seguimiento &seguimiento() const { return seguimiento_; }

private:
  void procesar(const struct AnuncioBle &a);

  std::vector<DispositivoBle> dispositivos_;
  Seguimiento seguimiento_;
  bool iniciado_ = false, activo_ = false;
  uint32_t tRefresco_ = 0;
};

const char *nombreEmpresaBle(uint16_t companyId);   // "Apple", "Samsung"... o nullptr
