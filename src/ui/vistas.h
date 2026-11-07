// Pantallas del detector. Solo dibujan: no guardan estado propio.
#pragma once
#include <TFT_eSPI.h>
#include <vector>
#include "wifi/escaner_wifi.h"
#include "rules/sospecha.h"

// Una red con su evaluación de sospecha.
struct Hallazgo {
  RedWifi red;
  Evaluacion eval;
};

namespace vistas {

constexpr int ALTO_FILA = 18;                  // fuente 2 (16 px) + 2 px
constexpr int Y_LISTA = 24;                    // bajo la barra de estado
constexpr int Y_AYUDA = 170 - 16;              // última línea
constexpr int FILAS = (Y_AYUDA - Y_LISTA) / ALTO_FILA;

void barraEstado(TFT_eSPI &tft, size_t redes, size_t sospechosas, uint32_t barrido, bool escuchando);
void lista(TFT_eSPI &tft, const std::vector<Hallazgo> &h, size_t primera, size_t seleccion);
void detalle(TFT_eSPI &tft, const Hallazgo &h);
void ayuda(TFT_eSPI &tft, const char *texto);

}  // namespace vistas
