#include "vistas.h"

namespace {

// Colores con alto contraste sobre negro. El nivel también se indica con
// símbolo y texto, así que el color nunca es la única pista.
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

void barrasSenal(TFT_eSPI &tft, int x, int y, int8_t rssi, uint16_t fondo) {
  const uint8_t nivel = nivelSenal(rssi);
  for (uint8_t i = 0; i < 4; i++) {
    const int h = 4 + i * 3, bx = x + i * 5, by = y + 14 - h;
    tft.fillRect(bx, by, 4, h, fondo);
    if (i < nivel) tft.fillRect(bx, by, 4, h, TFT_WHITE);
    else           tft.drawRect(bx, by, 4, h, TFT_DARKGREY);
  }
}

String nombreVisible(const RedWifi &r, unsigned maxCar) {
  String n = r.oculta() ? String("<oculta>") : r.ssid;
  if (r.abierta) n = "(abierta) " + n;
  if (n.length() > maxCar) n = n.substring(0, maxCar - 1) + "~";
  return n;
}

String macTexto(const uint8_t m[6]) {
  char b[18];
  snprintf(b, sizeof b, "%02X:%02X:%02X:%02X:%02X:%02X", m[0], m[1], m[2], m[3], m[4], m[5]);
  return b;
}

}  // namespace

namespace vistas {

void barraEstado(TFT_eSPI &tft, size_t redes, size_t sospechosas, uint32_t barrido, bool escuchando) {
  tft.fillRect(0, 0, tft.width(), Y_LISTA - 2, TFT_NAVY);
  tft.setTextFont(2);
  tft.setTextColor(TFT_YELLOW, TFT_NAVY);
  tft.drawString(String("Redes: ") + redes + "   Sospechosas: " + sospechosas, 6, 3);
  tft.setTextColor(TFT_WHITE, TFT_NAVY);
  tft.drawRightString(String(escuchando ? "~ " : "  ") + "#" + barrido, tft.width() - 4, 3, 2);
}

void lista(TFT_eSPI &tft, const std::vector<Hallazgo> &h, size_t primera, size_t seleccion) {
  tft.fillRect(0, Y_LISTA, tft.width(), FILAS * ALTO_FILA, TFT_BLACK);
  tft.setTextFont(2);
  if (h.empty()) {
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("Escuchando... aun sin redes", 6, Y_LISTA + 4);
    return;
  }
  for (int f = 0; f < FILAS && primera + f < h.size(); f++) {
    const size_t i = primera + f;
    const Hallazgo &x = h[i];
    const int y = Y_LISTA + f * ALTO_FILA;
    const bool sel = i == seleccion;
    const uint16_t fondo = sel ? TFT_DARKGREY : TFT_BLACK;
    if (sel) tft.fillRect(0, y - 1, tft.width(), ALTO_FILA, fondo);

    tft.setTextColor(colorNivel(x.eval.nivel), fondo);
    tft.drawString(simboloNivel(x.eval.nivel), 4, y);          // !!, !, ?
    barrasSenal(tft, 20, y, x.red.rssi, fondo);
    tft.setTextColor(TFT_WHITE, fondo);
    tft.drawString(String(x.red.rssi), 44, y);
    tft.setTextColor(TFT_CYAN, fondo);
    tft.drawString(String("c") + x.red.canal, 76, y);
    tft.setTextColor(x.eval.nivel >= Nivel::MEDIO ? colorNivel(x.eval.nivel) : TFT_WHITE, fondo);
    tft.drawString(nombreVisible(x.red, 24), 104, y);
  }
}

void detalle(TFT_eSPI &tft, const Hallazgo &h) {
  tft.fillRect(0, 0, tft.width(), Y_AYUDA, TFT_BLACK);
  const uint16_t c = colorNivel(h.eval.nivel);

  tft.setTextFont(4);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(nombreVisible(h.red, 18), 6, 2);

  tft.setTextFont(2);
  tft.setTextColor(c, TFT_BLACK);
  tft.drawString(String("Sospecha: ") + textoNivel(h.eval.nivel) + " (" + h.eval.puntaje + "/100) " +
                     simboloNivel(h.eval.nivel), 6, 30);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  String fab = h.eval.macAleatoria       ? String("Fabricante: oculto (MAC aleatoria)")
               : h.eval.fabricante.nombre ? String("Fabricante: ") + h.eval.fabricante.nombre + " (" +
                                                textoCategoria(h.eval.fabricante.categoria) + ")"
                                          : String("Fabricante: no esta en la tabla");
  tft.drawString(fab, 6, 48);
  tft.drawString(String("MAC ") + macTexto(h.red.bssid), 6, 66);
  tft.drawString(String("Canal ") + h.red.canal + "   Senal " + h.red.rssi + " dBm", 6, 84);

  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  if (h.eval.nMotivos == 0) tft.drawString("- Sin motivos de sospecha", 6, 104);
  for (uint8_t i = 0; i < h.eval.nMotivos && i < 2; i++)   // caben 2 líneas
    tft.drawString(String("- ") + h.eval.motivos[i], 6, 104 + i * 18);
}

void ayuda(TFT_eSPI &tft, const char *texto) {
  tft.fillRect(0, Y_AYUDA, tft.width(), 16, TFT_BLACK);
  tft.setTextFont(2);
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.drawString(texto, 6, Y_AYUDA);
}

}  // namespace vistas
