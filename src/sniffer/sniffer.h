// Sniffer WiFi en modo promiscuo: SOLO RECIBE. Salta entre canales 1..13 y
// mantiene una tabla de dispositivos (estaciones) con su tráfico de subida.
#pragma once
#include <Arduino.h>
#include <vector>

struct Estacion {
  uint8_t mac[6];
  uint8_t bssid[6];         // red (AP) a la que habla; todo cero si no se sabe
  bool conBssid = false;
  int8_t rssi = -100;       // promedio suavizado, solo de tramas que ELLA transmite
  uint8_t canal = 0;
  float subidaBps = 0;      // bytes/s que sube (suavizado)
  float bajadaBps = 0;      // bytes/s que recibe (suavizado)
  uint8_t ventanasAltas = 0;  // ventanas seguidas sobre el umbral de video
  uint32_t tramas = 0;
  uint32_t vistoMs = 0;
  // acumuladores de la ventana en curso
  uint32_t bytesSubidaV = 0, bytesBajadaV = 0;

  bool perfilVideo() const;
};

struct PuntoAcceso {
  uint8_t bssid[6];
  char ssid[33];            // "" = oculta
  uint8_t canal;
};

class Sniffer {
public:
  static constexpr uint16_t MS_POR_CANAL = 200;
  static constexpr uint8_t CANALES = 13;
  static constexpr uint8_t VUELTAS_POR_VENTANA = 2;   // ventana ≈ 2 × 13 × 200 ms ≈ 5.2 s
  static constexpr size_t MAX_ESTACIONES = 64;
  static constexpr size_t MAX_APS = 64;
  static constexpr uint32_t OLVIDAR_MS = 120000;     // quitar si no se ve en 2 min
  static constexpr float UMBRAL_VIDEO_BPS = 15000;   // 15 KB/s ≈ 120 kbit/s
  static constexpr uint8_t VENTANAS_VIDEO = 3;       // ≈ 15 s sostenidos

  void begin();             // crea la cola; la radio debe estar iniciada (radio::iniciar)
  void iniciar();
  void detener();
  bool activo() const { return activo_; }

  // Llamar en cada loop(): procesa tramas y salta de canal.
  // Devuelve true cuando se cierra una ventana (momento de redibujar).
  bool actualizar();

  const std::vector<Estacion> &estaciones() const { return estaciones_; }
  const PuntoAcceso *buscarAp(const uint8_t bssid[6]) const;
  uint8_t canalActual() const { return canal_; }
  uint32_t descartadas() const;
  uint32_t ventanas() const { return ventanas_; }

private:
  void procesar(const struct Trama &t);
  Estacion *estacion(const uint8_t mac[6], bool crear);
  bool esAp(const uint8_t mac[6]) const;
  void registrarAp(const struct Trama &t);
  void saltarCanal();
  void cerrarVentana();

  std::vector<Estacion> estaciones_;
  std::vector<PuntoAcceso> aps_;
  bool activo_ = false;
  uint8_t canal_ = 1;
  uint32_t tCanal_ = 0;
  uint16_t saltos_ = 0;
  uint32_t msEscuchado_[CANALES + 1] = {};   // por canal, en la ventana actual
  uint32_t ventanas_ = 0;
};
