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
    const uint16_t fondo = TFT_BLACK;
    if (i == seleccion) {                        // marco doble: visible sin depender del color de fondo
      tft.drawRect(0, y - 1, tft.width(), ALTO_FILA, TFT_YELLOW);
      tft.drawRect(1, y, tft.width() - 2, ALTO_FILA - 2, TFT_YELLOW);
    }

    if (x.rssi == SIN_SENAL) {                   // opción de menú: texto ancho
      tft.setTextColor(TFT_YELLOW, fondo);
      tft.drawString(x.nombre, 8, y);
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

void prepararRastreo(TFT_eSPI &tft, TFT_eSprite &g, const String &titulo, uint8_t canal) {
  tft.fillScreen(TFT_BLACK);
  barraEstado(tft, String("RASTREO c") + canal, recortar(titulo, 22));
  if (!g.created()) g.createSprite(GRAF_W, GRAF_H);
  ayuda(tft, "Pulsa para salir");
}

void rastreo(TFT_eSPI &tft, TFT_eSprite &g, const Seguimiento &s, bool conTrafico) {
  // --- Izquierda: localizador ---
  const bool senal = s.conSenal();
  tft.setTextFont(4);
  tft.setTextSize(2);                              // ~52 px de alto
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextPadding(124);
  tft.drawString(senal ? String(s.rssi()) : String("--"), 4, 26);
  tft.setTextSize(1);

  tft.setTextFont(2);
  tft.setTextPadding(124);
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.drawString("dBm (senal)", 4, 80);

  // Tendencia: símbolo + palabra + color (nunca solo color).
  const int8_t tend = s.tendencia();
  const char *txt = !senal ? "SIN SENAL" : tend > 0 ? "^ MAS CERCA" : tend < 0 ? "v MAS LEJOS" : "= ESTABLE";
  tft.setTextColor(!senal ? TFT_LIGHTGREY : tend > 0 ? TFT_YELLOW : tend < 0 ? TFT_CYAN : TFT_WHITE, TFT_BLACK);
  tft.drawString(txt, 4, 100);
  tft.setTextPadding(0);

  // Barra de intensidad: -100 dBm = vacía, -30 dBm = llena.
  const int bx = 4, by = 122, bw = 120, bh = 14;
  const int lleno = senal ? constrain((s.rssi() + 100) * bw / 70, 0, bw) : 0;
  tft.fillRect(bx, by, bw, bh, TFT_BLACK);
  tft.fillRect(bx, by, lleno, bh, TFT_WHITE);
  tft.drawRect(bx, by, bw, bh, TFT_LIGHTGREY);

  if (!conTrafico) {   // BLE: no hay tráfico útil que medir, solo señal
    tft.setTextFont(2);
    tft.setTextPadding(GRAF_W);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("Camina y sigue la", GRAF_X, 40);
    tft.drawString("tendencia de la senal.", GRAF_X, 58);
    tft.drawString("El objeto esta donde", GRAF_X, 84);
    tft.drawString("la senal es mas alta", GRAF_X, 102);
    tft.drawString("(cerca de -40 dBm).", GRAF_X, 120);
    tft.setTextPadding(0);
    return;
  }

  // --- Derecha: gráfico de bytes/s transmitidos ---
  uint32_t maximo = 2000;                          // escala mínima: 2 KB/s
  for (size_t i = 0; i < s.cantidad(); i++) maximo = max(maximo, s.muestra(i));
  const bool pico = s.pico();
  const size_t n = s.cantidad();
  const int alto = GRAF_H - 20;                    // deja espacio para la etiqueta

  g.fillSprite(TFT_BLACK);
  g.drawRect(0, 0, GRAF_W, GRAF_H, TFT_DARKGREY);
  for (size_t i = 0; i < n; i++) {
    const int h = (uint64_t)s.muestra(i) * alto / maximo;
    const bool reciente = i + Seguimiento::RECIENTES >= n;
    const uint16_t c = reciente ? (pico ? TFT_MAGENTA : TFT_YELLOW) : TFT_CYAN;
    const int x = 2 + (Seguimiento::MUESTRAS - n + i) * 2;   // lo nuevo, a la derecha
    g.fillRect(x, GRAF_H - 2 - h, 2, h, c);
  }
  g.setTextFont(2);
  g.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  g.drawString(String("max ") + tasaCorta(maximo) + "B/s", 4, 2);
  g.pushSprite(GRAF_X, GRAF_Y);

  // --- Bajo el gráfico: números y veredicto de correlación ---
  tft.setTextFont(2);
  tft.setTextPadding(GRAF_W);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(String("Ahora ") + tasaCorta(s.bpsActual()) + "B/s  base " + tasaCorta(s.bpsBase()), GRAF_X, 122);
  if (pico) {
    tft.setTextColor(TFT_MAGENTA, TFT_BLACK);
    tft.drawString("!! PICO: reacciona", GRAF_X, 138);
  } else {
    tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    tft.drawString(s.cantidad() < 20 ? "Midiendo base..." : "Prueba: mano enfrente", GRAF_X, 138);
  }
  tft.setTextPadding(0);
}

void inicio(TFT_eSPI &tft, const char *version) {
  tft.fillScreen(TFT_BLACK);
  tft.setTextFont(4);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString("BugSweep", 6, 4);
  tft.setTextFont(2);
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.drawRightString(String("v") + version, tft.width() - 6, 10, 2);
  tft.setTextFont(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Detector pasivo de camaras,", 6, 34);
  tft.drawString("microfonos y rastreadores.", 6, 52);

  // Las dos opciones, con el gesto escrito en palabras.
  tft.drawRect(4, 76, tft.width() - 8, 34, TFT_YELLOW);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString("MANTEN apretado: revision guiada", 10, 78);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("(recomendado, ~1 minuto)", 10, 93);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString("Toque corto: modo experto", 10, 118);
  ayuda(tft, "Solo escucha: no transmite nada.");
}

void progresoRevision(TFT_eSPI &tft, uint8_t paso, uint8_t total, const char *titulo, const char *l1,
                      const char *l2, float fraccion, size_t encontrados, size_t sospechosos) {
  barraEstado(tft, String("REVISION  paso ") + (paso + 1) + " de " + total, "");
  tft.setTextPadding(tft.width() - 12);
  tft.setTextFont(4);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(titulo, 6, 28);
  tft.setTextFont(2);
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.drawString(l1, 6, 58);
  tft.drawString(l2, 6, 74);

  // Barra de progreso con el porcentaje escrito.
  const int bx = 6, by = 98, bw = tft.width() - 12, bh = 16;
  tft.fillRect(bx, by, bw, bh, TFT_BLACK);
  tft.fillRect(bx, by, (int)(bw * fraccion), bh, TFT_YELLOW);
  tft.drawRect(bx, by, bw, bh, TFT_WHITE);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(String((int)(fraccion * 100)) + " %", 6, 118);

  tft.setTextColor(sospechosos ? TFT_MAGENTA : TFT_WHITE, TFT_BLACK);
  tft.drawString(String("Vistos: ") + encontrados + "   Sospechosos: " + sospechosos + (sospechosos ? " !" : ""), 6, 134);
  tft.setTextPadding(0);
  ayuda(tft, "Pulsa para cancelar");
}

void revisionLimpia(TFT_eSPI &tft) {
  tft.fillScreen(TFT_BLACK);
  barraEstado(tft, "RESULTADO", "");
  tft.setTextFont(4);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("OK  Nada sospechoso", 6, 28);
  tft.setTextFont(2);
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.drawString("No garantiza que no haya nada:", 6, 60);
  tft.drawString("- camaras sin WiFi, en 5 GHz o solo SD", 6, 78);
  tft.drawString("- microfonos de radio analogica/GSM", 6, 96);
  tft.drawString("Busca lentes con la linterna del celular.", 6, 120);
  ayuda(tft, "Pulsa para volver al inicio");
}

void ayuda(TFT_eSPI &tft, const char *texto) {
  tft.fillRect(0, Y_AYUDA, tft.width(), 16, TFT_BLACK);
  tft.setTextFont(2);
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.drawString(texto, 6, Y_AYUDA);
}

}  // namespace vistas
