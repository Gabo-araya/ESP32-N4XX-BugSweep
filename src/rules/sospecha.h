// Reglas de sospecha: combinan nombre de red, fabricante (OUI) y otras pistas
// en un puntaje 0..100 con los motivos explicados en lenguaje simple.
#pragma once
#include <Arduino.h>
#include "categoria.h"

enum class Nivel : uint8_t { NINGUNO, BAJO, MEDIO, ALTO };

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

// Símbolo y texto para mostrar el nivel sin depender del color.
const char *simboloNivel(Nivel n);   // "!!", "!", "?", ""
const char *textoNivel(Nivel n);     // "ALTA", "MEDIA", "BAJA", "ninguna"
const char *textoCategoria(Categoria c);
