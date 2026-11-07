// Pantallas del detector. Solo dibujan: no guardan estado propio.
// Son genéricas: el modo "redes" y el modo "dispositivos" las usan igual.
#pragma once
#include <TFT_eSPI.h>
#include <vector>
#include "rules/sospecha.h"
#include "sniffer/seguimiento.h"

namespace vistas {

constexpr int ALTO_FILA = 18;                  // fuente 2 (16 px) + 2 px
constexpr int Y_LISTA = 24;                    // bajo la barra de estado
constexpr int Y_AYUDA = 170 - 16;              // última línea
constexpr int FILAS = (Y_AYUDA - Y_LISTA) / ALTO_FILA;
constexpr int8_t SIN_SENAL = INT8_MIN;         // fila sin barras (p. ej. la opción de cambiar modo)

struct Fila {
  Nivel nivel = Nivel::NINGUNO;
  int8_t rssi = SIN_SENAL;
  String dato;      // columna corta: canal ("c6") o tasa de subida ("12k")
  String nombre;
};

struct Detalle {
  String titulo;
  const Evaluacion *eval;
  String lineas[4];  // datos técnicos (hasta 4)
};

void barraEstado(TFT_eSPI &tft, const String &izquierda, const String &derecha);
void lista(TFT_eSPI &tft, const std::vector<Fila> &filas, size_t seleccion, const char *vacio);
void detalle(TFT_eSPI &tft, const Detalle &d);
void ayuda(TFT_eSPI &tft, const char *texto);

// Rastreo: señal grande + tendencia + gráfico de tráfico (en un sprite, sin parpadeo).
void prepararRastreo(TFT_eSPI &tft, TFT_eSprite &grafico, const String &titulo, uint8_t canal);
// conTrafico = false (BLE): solo localizador; el gráfico muestra la señal en vez de bytes.
void rastreo(TFT_eSPI &tft, TFT_eSprite &grafico, const Seguimiento &s, bool conTrafico);
constexpr int GRAF_X = 132, GRAF_Y = 26, GRAF_W = 184, GRAF_H = 94;

// Utilidades de formato compartidas
String macTexto(const uint8_t m[6]);
String tasaCorta(float bytesPorSegundo);   // "850", "12k", "1.2M"
String recortar(String s, unsigned maxCar);

}  // namespace vistas
