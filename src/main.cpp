// ESP32-N4XX-BugSweep — redes (escaneo pasivo), dispositivos (promiscuo) y rastreo.
//
// Un solo botón (BOOT):
//   Lista:   corto = bajar el cursor | largo = abrir (detalle, o cambiar de modo en la 1.ª fila)
//   Detalle: corto = volver          | largo = rastrear (localizador + correlación)
//   Rastreo: corto o largo = salir
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <algorithm>
#include "rules/sospecha.h"
#include "sniffer/sniffer.h"
#include "ui/boton.h"
#include "ui/vistas.h"
#include "wifi/escaner_wifi.h"
#include "wifi/radio.h"

constexpr uint8_t PIN_BOTON = 0;   // BOOT, junto al USB-C

TFT_eSPI tft;
TFT_eSprite grafico(&tft);   // gráfico del rastreo: se dibuja fuera de pantalla y se copia de una vez
Boton boton(PIN_BOTON);
EscanerWifi escaner;
Sniffer sniffer;

enum class Modo { REDES, DISPOSITIVOS };
enum class Vista { LISTA, DETALLE, RASTREO };
Modo modo = Modo::REDES;
Vista vista = Vista::LISTA;

struct Red { RedWifi red; Evaluacion eval; };
struct Dispositivo { Estacion est; Evaluacion eval; };
std::vector<Red> redes;
std::vector<Dispositivo> dispositivos;

// Fila 0 = opción "cambiar de modo"; filas 1..n = elementos del modo actual.
size_t seleccion = 0;

// ---------------------------------------------------------------- modelo ---

size_t cantidad() { return modo == Modo::REDES ? redes.size() : dispositivos.size(); }

const uint8_t *macDe(size_t i) {   // i = índice de elemento (no de fila)
  return modo == Modo::REDES ? redes[i].red.bssid : dispositivos[i].est.mac;
}

template <typename T>
size_t contarSospechosos(const std::vector<T> &v) {
  return std::count_if(v.begin(), v.end(), [](const T &x) { return x.eval.nivel >= Nivel::MEDIO; });
}

// Reconstruye las listas manteniendo el cursor sobre el mismo elemento (por MAC).
template <typename F>
void reconstruir(F &&rellenar) {
  uint8_t mac[6];
  const bool habia = seleccion > 0 && seleccion <= cantidad();
  if (habia) memcpy(mac, macDe(seleccion - 1), 6);

  rellenar();

  if (!habia) return;
  for (size_t i = 0; i < cantidad(); i++)
    if (memcmp(macDe(i), mac, 6) == 0) { seleccion = i + 1; return; }
  // El elemento ya no está: volver a la lista, al inicio.
  seleccion = 0;
  if (vista == Vista::DETALLE) {
    vista = Vista::LISTA;
    tft.fillScreen(TFT_BLACK);
  }
}

void procesarRedes() {
  reconstruir([] {
    redes.clear();
    for (const RedWifi &r : escaner.redes()) redes.push_back({r, evaluarRed(r.ssid, r.bssid)});
    std::stable_sort(redes.begin(), redes.end(), [](const Red &a, const Red &b) {
      return a.eval.puntaje != b.eval.puntaje ? a.eval.puntaje > b.eval.puntaje : a.red.rssi > b.red.rssi;
    });
  });
}

void procesarDispositivos() {
  reconstruir([] {
    dispositivos.clear();
    for (const Estacion &e : sniffer.estaciones())
      dispositivos.push_back({e, evaluarEstacion(e.mac, e.perfilVideo())});
    std::stable_sort(dispositivos.begin(), dispositivos.end(), [](const Dispositivo &a, const Dispositivo &b) {
      return a.eval.puntaje != b.eval.puntaje ? a.eval.puntaje > b.eval.puntaje
                                              : a.est.subidaBps > b.est.subidaBps;
    });
  });
}

// ------------------------------------------------------------- textos UI ---

String nombreRed(const RedWifi &r) {
  String n = r.oculta() ? String("<oculta>") : r.ssid;
  return r.abierta ? "(abierta) " + n : n;
}

String nombreFabricante(const Evaluacion &e) {
  if (e.macAleatoria) return "aleatoria";
  return e.fabricante.nombre ? e.fabricante.nombre : "otro";
}

String lineaFabricante(const Evaluacion &e) {
  if (e.macAleatoria) return "Fabricante: oculto (MAC aleatoria)";
  if (!e.fabricante.nombre) return "Fabricante: no esta en la tabla";
  return String("Fabricante: ") + e.fabricante.nombre + " (" + textoCategoria(e.fabricante.categoria) + ")";
}

String nombreDispositivo(const Dispositivo &d) {
  const uint8_t *m = d.est.mac;
  char fin[9];
  snprintf(fin, sizeof fin, "%02X:%02X:%02X", m[3], m[4], m[5]);
  return nombreFabricante(d.eval) + " " + fin;
}

String redDeDispositivo(const Estacion &e) {
  if (!e.conBssid) return "Red: ninguna (solo busca redes)";
  const PuntoAcceso *ap = sniffer.buscarAp(e.bssid);
  if (ap && ap->ssid[0]) return String("Red: ") + vistas::recortar(ap->ssid, 24);
  return String("Red: ") + (ap ? "<oculta>" : vistas::macTexto(e.bssid));
}

// ---------------------------------------------------------------- dibujo ---

void dibujar() {
  if (vista == Vista::RASTREO) {
    vistas::rastreo(tft, grafico, sniffer.seguimiento());
    return;
  }
  if (vista == Vista::LISTA) {
    std::vector<vistas::Fila> filas;
    vistas::Fila menu;
    menu.nombre = modo == Modo::REDES ? "> Cambiar a DISPOSITIVOS" : "> Cambiar a REDES WiFi";
    filas.push_back(menu);

    if (modo == Modo::REDES) {
      for (const Red &r : redes) filas.push_back({r.eval.nivel, r.red.rssi, String("c") + r.red.canal, nombreRed(r.red)});
      vistas::barraEstado(tft, String("REDES ") + redes.size() + "  Sosp: " + contarSospechosos(redes),
                          String(escaner.escaneando() ? "~ #" : "  #") + escaner.barridos());
      vistas::lista(tft, filas, seleccion, "Escuchando beacons...");
    } else {
      for (const Dispositivo &d : dispositivos)
        filas.push_back({d.eval.nivel, d.est.rssi, vistas::tasaCorta(d.est.subidaBps), nombreDispositivo(d)});
      vistas::barraEstado(tft, String("DISPOSITIVOS ") + dispositivos.size() + "  Sosp: " + contarSospechosos(dispositivos),
                          String("c") + sniffer.canalActual() + " #" + sniffer.ventanas());
      vistas::lista(tft, filas, seleccion, "Escuchando trafico (~5 s)...");
    }
    vistas::ayuda(tft, "Corto: bajar   Largo: abrir");
    return;
  }

  const size_t i = seleccion - 1;
  vistas::Detalle d;
  if (modo == Modo::REDES) {
    const Red &r = redes[i];
    d = {nombreRed(r.red), &r.eval,
         {lineaFabricante(r.eval), "MAC " + vistas::macTexto(r.red.bssid),
          String("Canal ") + r.red.canal + "   Senal " + r.red.rssi + " dBm", ""}};
  } else {
    const Dispositivo &x = dispositivos[i];
    d = {nombreDispositivo(x), &x.eval,
         {lineaFabricante(x.eval),
          redDeDispositivo(x.est),
          String("Sube ") + vistas::tasaCorta(x.est.subidaBps) + "B/s  Baja " + vistas::tasaCorta(x.est.bajadaBps) + "B/s",
          String("Canal ") + x.est.canal + "  Senal " + x.est.rssi + " dBm  " + x.est.tramas + " tramas"}};
  }
  vistas::detalle(tft, d);
  vistas::ayuda(tft, "Corto: volver   Largo: RASTREAR");
}

// ----------------------------------------------------------------- serie ---

void imprimirRedes() {
  Serial.printf("[redes] barrido #%lu: %u redes, %u sospechosas\n", (unsigned long)escaner.barridos(),
                (unsigned)redes.size(), (unsigned)contarSospechosos(redes));
  for (const Red &r : redes) {
    Serial.printf("  %-2s %3u  %4d dBm  c%-2u  %s  %-12s %s\n", simboloNivel(r.eval.nivel), r.eval.puntaje,
                  r.red.rssi, r.red.canal, vistas::macTexto(r.red.bssid).c_str(),
                  nombreFabricante(r.eval).c_str(), nombreRed(r.red).c_str());
  }
}

void imprimirDispositivos() {
  Serial.printf("[disp] ventana #%lu: %u dispositivos, %u sospechosos, %lu tramas descartadas\n",
                (unsigned long)sniffer.ventanas(), (unsigned)dispositivos.size(),
                (unsigned)contarSospechosos(dispositivos), (unsigned long)sniffer.descartadas());
  for (const Dispositivo &d : dispositivos) {
    Serial.printf("  %-2s %3u  %4d dBm  c%-2u  sube %6s B/s  baja %6s B/s  %s  %s%s\n",
                  simboloNivel(d.eval.nivel), d.eval.puntaje, d.est.rssi, d.est.canal,
                  vistas::tasaCorta(d.est.subidaBps).c_str(), vistas::tasaCorta(d.est.bajadaBps).c_str(),
                  vistas::macTexto(d.est.mac).c_str(), nombreFabricante(d.eval).c_str(),
                  d.est.perfilVideo() ? "  [VIDEO]" : "");
  }
}

// ---------------------------------------------------------------- control ---

void cambiarModo() {
  if (modo == Modo::REDES) {
    escaner.detener();
    sniffer.iniciar();
    modo = Modo::DISPOSITIVOS;
    procesarDispositivos();
    Serial.println("[bugsweep] modo DISPOSITIVOS (promiscuo, solo recepcion)");
  } else {
    sniffer.detener();
    escaner.iniciar();
    modo = Modo::REDES;
    Serial.println("[bugsweep] modo REDES (escaneo pasivo)");
  }
  seleccion = 0;
}

// Se fija en el canal del objetivo y sigue todo lo que transmite.
void entrarRastreo() {
  const size_t i = seleccion - 1;
  const uint8_t *mac = macDe(i);
  const uint8_t canal = modo == Modo::REDES ? redes[i].red.canal : dispositivos[i].est.canal;
  const String titulo = modo == Modo::REDES ? nombreRed(redes[i].red) : nombreDispositivo(dispositivos[i]);

  escaner.detener();
  sniffer.detener();
  sniffer.seguimiento().iniciar(mac);
  sniffer.iniciar(canal);
  vista = Vista::RASTREO;
  vistas::prepararRastreo(tft, grafico, titulo, canal);
  Serial.printf("[rastreo] %s en canal %u (%s)\n", vistas::macTexto(mac).c_str(), canal, titulo.c_str());
}

// Vuelve al modo anterior (escaneo de redes o sniffer con saltos).
void salirRastreo() {
  sniffer.detener();
  sniffer.seguimiento().detener();
  if (modo == Modo::REDES) escaner.iniciar();
  else sniffer.iniciar();
  vista = Vista::DETALLE;
  Serial.println("[rastreo] fin");
}

void setup() {
  Serial.begin(115200);
  delay(200);
  boton.begin();
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  radio::iniciar();
  escaner.begin();
  sniffer.begin();
  Serial.printf("[bugsweep] tabla OUI: %u prefijos\n", totalPrefijosOui());
  escaner.iniciar();
  dibujar();
}

void loop() {
  bool redibujar = false;

  if (vista == Vista::RASTREO) {
    if (sniffer.actualizar()) {          // una muestra nueva cada 250 ms
      const Seguimiento &s = sniffer.seguimiento();
      Serial.printf("[rastreo] %4d dBm  tend %+d  ahora %6.0f B/s  base %6.0f B/s%s\n", s.rssi(),
                    s.tendencia(), s.bpsActual(), s.bpsBase(), s.pico() ? "  PICO" : "");
      redibujar = true;
    }
  } else if (modo == Modo::REDES) {
    if (escaner.actualizar()) { procesarRedes(); imprimirRedes(); redibujar = true; }
    if (!escaner.escaneando()) escaner.iniciar();   // continuo (y reintenta si falla)
  } else if (sniffer.actualizar()) {
    procesarDispositivos();
    imprimirDispositivos();
    redibujar = true;
  }

  const Boton::Evento ev = boton.leer();
  if (ev != Boton::Evento::Ninguno) {
    bool limpiar = true;   // prepararRastreo() ya limpia y dibuja su propio marco
    switch (vista) {
      case Vista::RASTREO:
        salirRastreo();
        break;
      case Vista::DETALLE:
        if (ev == Boton::Evento::Largo) { entrarRastreo(); limpiar = false; }
        else vista = Vista::LISTA;
        break;
      case Vista::LISTA:
        if (ev == Boton::Evento::Corto) seleccion = (seleccion + 1) % (cantidad() + 1);
        else if (seleccion == 0) cambiarModo();
        else vista = Vista::DETALLE;
        break;
    }
    if (limpiar) tft.fillScreen(TFT_BLACK);
    redibujar = true;
  }
  if (redibujar) dibujar();
  delay(2);
}
