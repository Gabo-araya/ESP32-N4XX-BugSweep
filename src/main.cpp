// Fase 2: escáner WiFi pasivo con lista en pantalla.
//   BOOT corto: bajar en la lista (vuelve al inicio al final)
//   BOOT largo: nuevo barrido
#include <Arduino.h>
#include <TFT_eSPI.h>
#include "ui/boton.h"
#include "wifi/escaner_wifi.h"

constexpr uint8_t PIN_BOTON = 0;      // BOOT, junto al USB-C
constexpr int ALTO_FILA = 18;         // fuente 2 = 16 px + 2 de separación
constexpr int Y_LISTA = 24;           // bajo la barra de estado
constexpr int Y_AYUDA = 170 - 16;     // última línea de la pantalla
constexpr int FILAS = (Y_AYUDA - Y_LISTA) / ALTO_FILA;  // filas visibles

TFT_eSPI tft;
Boton boton(PIN_BOTON);
EscanerWifi escaner;
size_t primera = 0;   // índice de la primera red visible

// Barras de señal 0..4: el número en dBm se muestra igual, la barra es solo apoyo visual.
uint8_t barras(int8_t rssi) {
  if (rssi >= -55) return 4;
  if (rssi >= -67) return 3;
  if (rssi >= -75) return 2;
  if (rssi >= -85) return 1;
  return 0;
}

void dibujarBarras(int x, int y, uint8_t nivel) {
  for (uint8_t i = 0; i < 4; i++) {
    const int h = 4 + i * 3;
    const int bx = x + i * 5, by = y + 14 - h;
    if (i < nivel) tft.fillRect(bx, by, 4, h, TFT_WHITE);
    else           tft.drawRect(bx, by, 4, h, TFT_DARKGREY);
  }
}

void dibujarEstado() {
  tft.fillRect(0, 0, tft.width(), Y_LISTA - 2, TFT_NAVY);
  tft.setTextFont(2);
  tft.setTextColor(TFT_YELLOW, TFT_NAVY);
  String s = escaner.escaneando()
                 ? String("Escuchando canales 1-13...")
                 : String("Redes: ") + escaner.redes().size() + "  barrido #" + escaner.barridos();
  tft.drawString(s, 6, 3);
}

void dibujarLista() {
  const auto &redes = escaner.redes();
  tft.fillRect(0, Y_LISTA, tft.width(), FILAS * ALTO_FILA, TFT_BLACK);
  tft.setTextFont(2);

  if (redes.empty()) {
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(escaner.escaneando() ? "Esperando resultados..." : "No se encontraron redes", 6, Y_LISTA + 4);
    return;
  }

  for (int f = 0; f < FILAS && primera + f < redes.size(); f++) {
    const RedWifi &r = redes[primera + f];
    const int y = Y_LISTA + f * ALTO_FILA;

    dibujarBarras(4, y, barras(r.rssi));
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(String(r.rssi), 28, y);                 // dBm
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawString(String("c") + r.canal, 60, y);          // canal

    // Red oculta: texto distinto + color, nunca solo color.
    String nombre = r.oculta() ? String("<oculta>") : r.ssid;
    if (r.abierta) nombre = "(abierta) " + nombre;
    if (nombre.length() > 26) nombre = nombre.substring(0, 25) + "~";
    tft.setTextColor(r.oculta() ? TFT_YELLOW : TFT_WHITE, TFT_BLACK);
    tft.drawString(nombre, 92, y);
  }
}

void dibujarAyuda() {
  tft.fillRect(0, Y_AYUDA, tft.width(), 16, TFT_BLACK);
  tft.setTextFont(2);
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  const size_t total = escaner.redes().size();
  String pos = total ? String(primera + 1) + "-" + min(primera + FILAS, total) + "/" + total : "";
  tft.drawString("Corto: bajar  Largo: re-escanear", 6, Y_AYUDA);
  tft.drawRightString(pos, tft.width() - 4, Y_AYUDA, 2);
}

void dibujarTodo() {
  dibujarEstado();
  dibujarLista();
  dibujarAyuda();
}

void imprimirSerie() {
  Serial.printf("[wifi] barrido #%lu: %u redes\n", (unsigned long)escaner.barridos(),
                (unsigned)escaner.redes().size());
  for (const RedWifi &r : escaner.redes()) {
    Serial.printf("  %4d dBm  c%-2u  %02X:%02X:%02X:%02X:%02X:%02X  %s%s\n", r.rssi, r.canal,
                  r.bssid[0], r.bssid[1], r.bssid[2], r.bssid[3], r.bssid[4], r.bssid[5],
                  r.abierta ? "(abierta) " : "", r.oculta() ? "<oculta>" : r.ssid.c_str());
  }
}

void nuevoBarrido() {
  escaner.iniciar();
  dibujarEstado();
  Serial.println("[wifi] escuchando (pasivo)...");
}

void setup() {
  Serial.begin(115200);
  delay(200);
  boton.begin();
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  escaner.begin();
  dibujarTodo();
  nuevoBarrido();
}

void loop() {
  if (escaner.actualizar()) {
    primera = 0;
    dibujarTodo();
    imprimirSerie();
  }

  switch (boton.leer()) {
    case Boton::Evento::Corto: {
      const size_t total = escaner.redes().size();
      if (total > FILAS) {
        primera += FILAS;
        if (primera >= total) primera = 0;   // vuelve al inicio
        dibujarLista();
        dibujarAyuda();
      }
      break;
    }
    case Boton::Evento::Largo:
      nuevoBarrido();
      break;
    case Boton::Evento::Ninguno:
      break;
  }
  delay(5);
}
