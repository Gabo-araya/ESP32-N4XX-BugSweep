// Fase 3: escáner WiFi pasivo continuo + reglas de sospecha.
//   Lista:   BOOT corto = siguiente red   | BOOT largo = ver detalle
//   Detalle: BOOT corto o largo = volver a la lista
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <algorithm>
#include "ui/boton.h"
#include "ui/vistas.h"
#include "wifi/escaner_wifi.h"
#include "rules/sospecha.h"

constexpr uint8_t PIN_BOTON = 0;   // BOOT, junto al USB-C

TFT_eSPI tft;
Boton boton(PIN_BOTON);
EscanerWifi escaner;

enum class Vista { LISTA, DETALLE };
Vista vista = Vista::LISTA;
std::vector<Hallazgo> hallazgos;
size_t seleccion = 0;

size_t contarSospechosas() {
  return std::count_if(hallazgos.begin(), hallazgos.end(),
                       [](const Hallazgo &h) { return h.eval.nivel >= Nivel::MEDIO; });
}

// Primera fila visible: la "página" que contiene la selección.
size_t primeraFila() { return seleccion - seleccion % vistas::FILAS; }

void dibujar() {
  if (vista == Vista::LISTA) {
    vistas::barraEstado(tft, hallazgos.size(), contarSospechosas(), escaner.barridos(), escaner.escaneando());
    vistas::lista(tft, hallazgos, primeraFila(), seleccion);
    vistas::ayuda(tft, "Corto: siguiente   Largo: detalle");
  } else {
    vistas::detalle(tft, hallazgos[seleccion]);
    vistas::ayuda(tft, "Pulsa para volver   (se actualiza solo)");
  }
}

void imprimirSerie() {
  Serial.printf("[wifi] barrido #%lu: %u redes, %u sospechosas\n", (unsigned long)escaner.barridos(),
                (unsigned)hallazgos.size(), (unsigned)contarSospechosas());
  for (const Hallazgo &h : hallazgos) {
    const uint8_t *m = h.red.bssid;
    Serial.printf("  %-2s %3u  %4d dBm  c%-2u  %02X:%02X:%02X:%02X:%02X:%02X  %-12s %s\n",
                  simboloNivel(h.eval.nivel), h.eval.puntaje, h.red.rssi, h.red.canal,
                  m[0], m[1], m[2], m[3], m[4], m[5],
                  h.eval.macAleatoria ? "(aleatoria)" : h.eval.fabricante.nombre ? h.eval.fabricante.nombre : "-",
                  h.red.oculta() ? "<oculta>" : h.red.ssid.c_str());
    for (uint8_t i = 0; i < h.eval.nMotivos; i++) Serial.printf("        - %s\n", h.eval.motivos[i]);
  }
}

// Reconstruye la lista con el último barrido, manteniendo seleccionada la misma red.
void procesarBarrido() {
  uint8_t bssidSel[6] = {};
  const bool habiaSel = seleccion < hallazgos.size();
  if (habiaSel) memcpy(bssidSel, hallazgos[seleccion].red.bssid, 6);

  hallazgos.clear();
  for (const RedWifi &r : escaner.redes()) hallazgos.push_back({r, evaluarRed(r.ssid, r.bssid)});

  // Más sospechosas primero; a igual puntaje, la señal más fuerte.
  std::stable_sort(hallazgos.begin(), hallazgos.end(), [](const Hallazgo &a, const Hallazgo &b) {
    return a.eval.puntaje != b.eval.puntaje ? a.eval.puntaje > b.eval.puntaje : a.red.rssi > b.red.rssi;
  });

  seleccion = 0;
  if (habiaSel) {
    for (size_t i = 0; i < hallazgos.size(); i++)
      if (memcmp(hallazgos[i].red.bssid, bssidSel, 6) == 0) { seleccion = i; break; }
  }
  // Si la red del detalle desapareció, volver a la lista.
  if (vista == Vista::DETALLE && (hallazgos.empty() || (habiaSel && memcmp(hallazgos[seleccion].red.bssid, bssidSel, 6)))) {
    vista = Vista::LISTA;
    tft.fillScreen(TFT_BLACK);
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);
  boton.begin();
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  escaner.begin();
  Serial.printf("[bugsweep] tabla OUI: %u prefijos\n", totalPrefijosOui());
  escaner.iniciar();
  dibujar();
}

void loop() {
  if (escaner.actualizar()) {
    procesarBarrido();
    imprimirSerie();
    dibujar();
  }
  if (!escaner.escaneando()) escaner.iniciar();   // escaneo continuo (también reintenta si un barrido falla)

  const Boton::Evento ev = boton.leer();
  if (ev == Boton::Evento::Ninguno) { delay(5); return; }

  if (vista == Vista::LISTA) {
    if (hallazgos.empty()) return;
    if (ev == Boton::Evento::Corto) {
      seleccion = (seleccion + 1) % hallazgos.size();
    } else {
      vista = Vista::DETALLE;
    }
  } else {
    vista = Vista::LISTA;
  }
  tft.fillScreen(TFT_BLACK);
  dibujar();
}
