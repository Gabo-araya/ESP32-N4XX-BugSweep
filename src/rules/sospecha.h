// Reglas de sospecha: combinan nombre de red, fabricante (OUI) y otras pistas
// en un puntaje 0..100 con los motivos explicados en lenguaje simple.
#pragma once
#include <Arduino.h>
#include "categoria.h"

enum class Nivel : uint8_t { NINGUNO, BAJO, MEDIO, ALTO };

// Rastreadores BLE reconocibles por su anuncio.
enum class Rastreador : uint8_t {
  NINGUNO,
  FIND_MY_SEPARADO,   // Apple Find My (AirTag u otro) LEJOS de su dueño
  FIND_MY_CERCA,      // Apple Find My con su dueño cerca
  TILE,
  SMARTTAG,           // Samsung
  GOOGLE_FMDN,        // red Find My Device de Google (posible)
};

struct InfoFabricante {
  const char *nombre;     // nullptr si no está en la tabla
  Categoria categoria;
};

struct Evaluacion {
  static constexpr uint8_t MAX_MOTIVOS = 4;
  uint8_t puntaje = 0;
  Nivel nivel = Nivel::NINGUNO;
  InfoFabricante fabricante{nullptr, Categoria::IOT};
  bool macAleatoria = false;
  const char *motivos[MAX_MOTIVOS] = {};
  uint8_t nMotivos = 0;
};

// MAC "localmente administrada" (bit 0x02 del primer byte): es aleatoria
// o inventada, así que no tiene fabricante registrado.
inline bool esMacAleatoria(const uint8_t mac[6]) { return mac[0] & 0x02; }

InfoFabricante buscarFabricante(const uint8_t mac[6]);
unsigned totalPrefijosOui();   // tamaño de la tabla generada
Evaluacion evaluarRed(const String &ssid, const uint8_t bssid[6]);
// Dispositivo (estación) visto en modo promiscuo.
Evaluacion evaluarEstacion(const uint8_t mac[6], bool perfilVideo);
// Dispositivo Bluetooth LE. Solo las direcciones públicas tienen fabricante (OUI).
Evaluacion evaluarBle(const String &nombre, const uint8_t mac[6], bool publica, Rastreador r,
                      uint32_t minutosVisto);
const char *textoRastreador(Rastreador r);   // "AirTag/FindMy", "Tile"...

// Símbolo y texto para mostrar el nivel sin depender del color.
const char *simboloNivel(Nivel n);   // "!!", "!", "?", ""
const char *textoNivel(Nivel n);     // "ALTA", "MEDIA", "BAJA", "ninguna"
const char *textoCategoria(Categoria c);
