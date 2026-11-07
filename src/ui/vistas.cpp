#include "vistas.h"

namespace {

// Alto contraste sobre negro. El nivel también va con símbolo y texto:
// el color nunca es la única pista.
uint16_t colorNivel(Nivel n) {
  switch (n) {
    case Nivel::ALTO:  return TFT_MAGENTA;
    case Nivel::MEDIO: return TFT_YELLOW;
    case Nivel::BAJO:  return TFT_CYAN;
    default:           return TFT_WHITE;
  }
}

uint8_t nivelSenal(int8_t rssi) {
  if (rssi >= -55) return 4;
  if (rssi >= -67) return 3;
  if (rssi >= -75) return 2;
  if (rssi >= -85) return 1;
  return 0;
}

void barrasSenal(TFT_eSPI &tft, int x, int y, int8_t rssi) {
  const uint8_t nivel = nivelSenal(rssi);
  for (uint8_t i = 0; i < 4; i++) {
    const int h = 4 + i * 3, bx = x + i * 5, by = y + 14 - h;
    if (i < nivel) tft.fillRect(bx, by, 4, h, TFT_WHITE);
    else           tft.drawRect(bx, by, 4, h, TFT_LIGHTGREY);
  }
}

}  // namespace

namespace vistas {

String macTexto(const uint8_t m[6]) {
  char b[18];
  snprintf(b, sizeof b, "%02X:%02X:%02X:%02X:%02X:%02X", m[0], m[1], m[2], m[3], m[4], m[5]);
  return b;
}

String tasaCorta(float bps) {
  if (bps < 1000) return String((int)bps);
  if (bps < 1e6) return String((int)(bps / 1000)) + "k";
  return String(bps / 1e6, 1) + "M";
}

String recortar(String s, unsigned maxCar) {
  if (s.length() > maxCar) s = s.substring(0, maxCar - 1) + "~";
  return s;
}

void barraEstado(TFT_eSPI &tft, const String &izq, const String &der) {
  tft.fillRect(0, 0, tft.width(), Y_LISTA - 2, TFT_NAVY);
  tft.setTextFont(2);
  tft.setTextColor(TFT_YELLOW, TFT_NAVY);
  tft.drawString(izq, 6, 3);
  tft.setTextColor(TFT_WHITE, TFT_NAVY);
  tft.drawRightString(der, tft.width() - 4, 3, 2);
}

void lista(TFT_eSPI &tft, const std::vector<Fila> &filas, size_t seleccion, const char *vacio) {
  tft.fillRect(0, Y_LISTA - 1, tft.width(), FILAS * ALTO_FILA + 1, TFT_BLACK);
  tft.setTextFont(2);
  const size_t primera = seleccion - seleccion % FILAS;   // "página" que contiene la selección
  for (int f = 0; f < FILAS && primera + f < filas.size(); f++) {
    const size_t i = primera + f;
    const Fila &x = filas[i];
    const int y = Y_LISTA + f * ALTO_FILA;
    const uint16_t fondo = i == seleccion ? TFT_DARKGREY : TFT_BLACK;
    if (fondo != TFT_BLACK) tft.fillRect(0, y - 1, tft.width(), ALTO_FILA, fondo);

    if (x.rssi == SIN_SENAL) {                   // opción de menú: texto ancho
      tft.setTextColor(TFT_YELLOW, fondo);
      tft.drawString(x.nombre, 20, y);
      continue;
    }
    tft.setTextColor(colorNivel(x.nivel), fondo);
    tft.drawString(simboloNivel(x.nivel), 4, y);          // !!, !, ?
    barrasSenal(tft, 20, y, x.rssi);
    tft.setTextColor(TFT_WHITE, fondo);
    tft.drawString(String(x.rssi), 44, y);
    tft.setTextColor(TFT_CYAN, fondo);
    tft.drawString(x.dato, 76, y);
    tft.setTextColor(x.nivel >= Nivel::MEDIO ? colorNivel(x.nivel) : TFT_WHITE, fondo);
    tft.drawString(recortar(x.nombre, 24), 108, y);
  }
  if (filas.size() <= 1 && vacio) {              // solo la opción de menú
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(vacio, 6, Y_LISTA + ALTO_FILA + 4);
  }
}

void detalle(TFT_eSPI &tft, const Detalle &d) {
  tft.fillRect(0, 0, tft.width(), Y_AYUDA, TFT_BLACK);
  const Evaluacion &e = *d.eval;

  tft.setTextFont(4);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(recortar(d.titulo, 18), 6, 2);

  tft.setTextFont(2);
  tft.setTextColor(colorNivel(e.nivel), TFT_BLACK);
  tft.drawString(String("Sospecha: ") + textoNivel(e.nivel) + " (" + e.puntaje + "/100) " + simboloNivel(e.nivel), 6, 28);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  int y = 46;
  for (const String &l : d.lineas) {
    if (l.isEmpty()) continue;
    tft.drawString(l, 6, y);
    y += 17;
  }
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  if (e.nMotivos == 0) tft.drawString("- Sin motivos de sospecha", 6, y);
  for (uint8_t i = 0; i < e.nMotivos && y + 16 <= Y_AYUDA; i++, y += 17)
    tft.drawString(String("- ") + e.motivos[i], 6, y);
}

void ayuda(TFT_eSPI &tft, const char *texto) {
  tft.fillRect(0, Y_AYUDA, tft.width(), 16, TFT_BLACK);
  tft.setTextFont(2);
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.drawString(texto, 6, Y_AYUDA);
}

}  // namespace vistas
