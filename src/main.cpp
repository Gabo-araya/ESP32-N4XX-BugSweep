// ESP32-N4XX-BugSweep — detector pasivo: redes WiFi, dispositivos WiFi (promiscuo),
// Bluetooth LE y rastreo (localizador + correlación).
//
// Un solo botón (BOOT):
//   Lista:   corto = bajar el cursor | largo = abrir (detalle, o cambiar de modo en la 1.ª fila)
//   Detalle: corto = volver          | largo = rastrear
//   Rastreo: corto o largo = salir
//   Inicio:  corto = modo experto    | largo = revisión guiada
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <algorithm>
#include "ble/escaner_ble.h"
#include "guia/revision.h"
#include "rules/sospecha.h"
#include "sniffer/sniffer.h"
#include "ui/boton.h"
#include "ui/vistas.h"
#include "wifi/escaner_wifi.h"
#include "wifi/radio.h"

constexpr uint8_t PIN_BOTON = 0;   // BOOT, junto al USB-C

#ifndef BUGSWEEP_VERSION
#define BUGSWEEP_VERSION "dev"
#endif

// Mensaje del monitor serie con los segundos desde el encendido, para cruzarlo con notas de terreno.
#define LOG(fmt, ...) Serial.printf("[%5lus] " fmt, (unsigned long)(millis() / 1000), ##__VA_ARGS__)

TFT_eSPI tft;
TFT_eSprite grafico(&tft);   // gráfico del rastreo: se dibuja fuera de pantalla y se copia de una vez
Boton boton(PIN_BOTON);
EscanerWifi escaner;
Sniffer sniffer;
EscanerBle ble;
Revision revision;

enum class Modo : uint8_t { REDES, DISPOSITIVOS, BLUETOOTH };
constexpr uint8_t TOTAL_MODOS = 3;
// La revisión guiada recorre los pasos en este mismo orden (paso i = Modo(i)).
static_assert((uint8_t)Modo::REDES == 0 && (uint8_t)Modo::DISPOSITIVOS == 1 && (uint8_t)Modo::BLUETOOTH == 2 &&
                  Revision::PASOS == TOTAL_MODOS,
              "Revision::PASOS debe seguir el orden de Modo");
enum class Vista { INICIO, LISTA, DETALLE, RASTREO, REVISION, RESULTADO };
Modo modo = Modo::REDES;
Vista vista = Vista::INICIO;

struct Red { RedWifi red; Evaluacion eval; };
struct Dispositivo { Estacion est; Evaluacion eval; };
struct Ble { DispositivoBle dev; Evaluacion eval; };
std::vector<Red> redes;
std::vector<Dispositivo> dispositivos;
std::vector<Ble> bles;

// Resultado de la revisión guiada: lo sospechoso de los tres modos, juntos.
struct Resultado { Modo modo; uint8_t mac[6]; uint8_t puntaje; vistas::Fila fila; };
std::vector<Resultado> resultados;

// Fila 0 = opción de menú; filas 1..n = elementos (del modo actual o del resultado).
size_t seleccion = 0;

// ---------------------------------------------------------------- modelo ---

Modo siguienteModo() { return Modo(((uint8_t)modo + 1) % TOTAL_MODOS); }

const char *nombreModo(Modo m) {
  switch (m) {
    case Modo::REDES:        return "REDES";
    case Modo::DISPOSITIVOS: return "DISPOSITIVOS";
    case Modo::BLUETOOTH:    return "BLUETOOTH";
  }
  return "";
}

size_t cantidad() {
  switch (modo) {
    case Modo::REDES:        return redes.size();
    case Modo::DISPOSITIVOS: return dispositivos.size();
    case Modo::BLUETOOTH:    return bles.size();
  }
  return 0;
}

const uint8_t *macDe(size_t i) {   // i = índice de elemento (no de fila)
  switch (modo) {
    case Modo::REDES:        return redes[i].red.bssid;
    case Modo::DISPOSITIVOS: return dispositivos[i].est.mac;
    case Modo::BLUETOOTH:    return bles[i].dev.mac;
  }
  return nullptr;
}

template <typename T>
size_t contarSospechosos(const std::vector<T> &v) {
  return std::count_if(v.begin(), v.end(), [](const T &x) { return x.eval.nivel >= Nivel::MEDIO; });
}

// Más sospechosos primero; a igual puntaje decide 'desempate' (true = a va antes).
template <typename T, typename D>
void ordenar(std::vector<T> &v, D desempate) {
  std::stable_sort(v.begin(), v.end(), [&](const T &a, const T &b) {
    return a.eval.puntaje != b.eval.puntaje ? a.eval.puntaje > b.eval.puntaje : desempate(a, b);
  });
}

// Reconstruye la lista del modo actual manteniendo el cursor sobre el mismo elemento (por MAC).
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
    ordenar(redes, [](const Red &a, const Red &b) { return a.red.rssi > b.red.rssi; });
  });
}

void procesarDispositivos() {
  reconstruir([] {
    dispositivos.clear();
    for (const Estacion &e : sniffer.estaciones())
      dispositivos.push_back({e, evaluarEstacion(e.mac, e.perfilVideo())});
    ordenar(dispositivos, [](const Dispositivo &a, const Dispositivo &b) { return a.est.subidaBps > b.est.subidaBps; });
  });
}

void procesarBle() {
  reconstruir([] {
    bles.clear();
    for (const DispositivoBle &d : ble.dispositivos())
      bles.push_back({d, evaluarBle(d.nombre, d.mac, d.publica, d.rastreador, d.minutosVisto())});
    ordenar(bles, [](const Ble &a, const Ble &b) { return a.dev.rssi > b.dev.rssi; });
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

String finMac(const uint8_t m[6]) {
  char b[9];
  snprintf(b, sizeof b, "%02X:%02X:%02X", m[3], m[4], m[5]);
  return b;
}

String nombreDispositivo(const Dispositivo &d) { return nombreFabricante(d.eval) + " " + finMac(d.est.mac); }

String redDeDispositivo(const Estacion &e) {
  if (!e.conBssid) return "Red: ninguna (solo busca redes)";
  const PuntoAcceso *ap = sniffer.buscarAp(e.bssid);
  if (ap && ap->ssid[0]) return String("Red: ") + vistas::recortar(ap->ssid, 24);
  return String("Red: ") + (ap ? "<oculta>" : vistas::macTexto(e.bssid));
}

// Lo más informativo que tengamos: rastreador > nombre > empresa > fabricante.
String nombreBle(const Ble &b) {
  if (b.dev.rastreador != Rastreador::NINGUNO) return textoRastreador(b.dev.rastreador);
  if (b.dev.nombre[0]) return b.dev.nombre;
  if (const char *e = nombreEmpresaBle(b.dev.empresa)) return String(e) + " " + finMac(b.dev.mac);
  if (b.dev.publica && b.eval.fabricante.nombre) return String(b.eval.fabricante.nombre) + " " + finMac(b.dev.mac);
  return String("BLE ") + finMac(b.dev.mac);
}

String minutos(uint32_t m) { return m ? String(m) + "m" : String("<1m"); }

// ---------------------------------------------------------------- dibujo ---

void dibujarLista() {
  std::vector<vistas::Fila> filas;
  vistas::Fila menu;
  menu.nombre = modo == Modo::BLUETOOTH ? String("> Volver al INICIO")
                                        : String("> Cambiar a ") + nombreModo(siguienteModo());
  filas.push_back(menu);

  String izq, der;
  const char *vacio = "";
  switch (modo) {
    case Modo::REDES:
      for (const Red &r : redes) filas.push_back({r.eval.nivel, r.red.rssi, String("c") + r.red.canal, nombreRed(r.red)});
      izq = String("REDES ") + redes.size() + "  Sosp: " + contarSospechosos(redes);
      der = String(escaner.escaneando() ? "~ #" : "  #") + escaner.barridos();
      vacio = "Escuchando beacons...";
      break;
    case Modo::DISPOSITIVOS:
      for (const Dispositivo &d : dispositivos)
        filas.push_back({d.eval.nivel, d.est.rssi, vistas::tasaCorta(d.est.subidaBps), nombreDispositivo(d)});
      izq = String("DISPOSITIVOS ") + dispositivos.size() + "  Sosp: " + contarSospechosos(dispositivos);
      der = String("c") + sniffer.canalActual() + " #" + sniffer.ventanas();
      vacio = "Escuchando trafico (~5 s)...";
      break;
    case Modo::BLUETOOTH:
      // Columna corta: hace cuánto lo vemos (un rastreador que te sigue se ve por mucho rato).
      for (const Ble &b : bles) filas.push_back({b.eval.nivel, b.dev.rssi, minutos(b.dev.minutosVisto()), nombreBle(b)});
      izq = String("BLUETOOTH ") + bles.size() + "  Sosp: " + contarSospechosos(bles);
      der = "BLE";
      vacio = "Escuchando anuncios BLE...";
      break;
  }
  vistas::barraEstado(tft, izq, der);
  vistas::lista(tft, filas, seleccion, vacio);
  vistas::ayuda(tft, "Corto: bajar   Largo: abrir");
}

void dibujarDetalle() {
  const size_t i = seleccion - 1;
  vistas::Detalle d;
  switch (modo) {
    case Modo::REDES: {
      const Red &r = redes[i];
      d = {nombreRed(r.red), &r.eval,
           {lineaFabricante(r.eval), "MAC " + vistas::macTexto(r.red.bssid),
            String("Canal ") + r.red.canal + "   Senal " + r.red.rssi + " dBm", ""}};
      break;
    }
    case Modo::DISPOSITIVOS: {
      const Dispositivo &x = dispositivos[i];
      d = {nombreDispositivo(x), &x.eval,
           {lineaFabricante(x.eval), redDeDispositivo(x.est),
            String("Sube ") + vistas::tasaCorta(x.est.subidaBps) + "B/s  Baja " + vistas::tasaCorta(x.est.bajadaBps) + "B/s",
            String("Canal ") + x.est.canal + "  Senal " + x.est.rssi + " dBm  " + x.est.tramas + " tramas"}};
      break;
    }
    case Modo::BLUETOOTH: {
      const Ble &b = bles[i];
      const char *emp = nombreEmpresaBle(b.dev.empresa);
      d = {nombreBle(b), &b.eval,
           {String(b.dev.publica ? "Direccion publica " : "Direccion aleatoria ") + vistas::macTexto(b.dev.mac).substring(9),
            String("Empresa: ") + (emp ? emp : b.dev.empresa == 0xFFFF ? "no informa" : "otra") +
                (b.dev.nombre[0] ? String("  \"") + vistas::recortar(b.dev.nombre, 12) + "\"" : String("")),
            String("Senal ") + b.dev.rssi + " dBm   " + b.dev.anuncios + " anuncios",
            String("Visto durante ") + minutos(b.dev.minutosVisto())}};
      break;
    }
  }
  vistas::detalle(tft, d);
  vistas::ayuda(tft, "Corto: volver   Largo: RASTREAR");
}

const Seguimiento &seguimientoActivo() {
  return modo == Modo::BLUETOOTH ? ble.seguimiento() : sniffer.seguimiento();
}

void dibujarRevision() {
  const PasoGuia &p = Revision::paso(revision.indice());
  size_t sosp = 0;
  switch (modo) {
    case Modo::REDES:        sosp = contarSospechosos(redes); break;
    case Modo::DISPOSITIVOS: sosp = contarSospechosos(dispositivos); break;
    case Modo::BLUETOOTH:    sosp = contarSospechosos(bles); break;
  }
  vistas::progresoRevision(tft, revision.indice(), Revision::PASOS, p.titulo, p.linea1, p.linea2,
                           revision.fraccion(), cantidad(), sosp);
}

void dibujarResultado() {
  if (resultados.empty()) { vistas::revisionLimpia(tft); return; }
  std::vector<vistas::Fila> filas;
  vistas::Fila menu;
  menu.nombre = "> Terminar (volver al INICIO)";
  filas.push_back(menu);
  for (const Resultado &r : resultados) filas.push_back(r.fila);
  vistas::barraEstado(tft, String("RESULTADO  Sospechosos: ") + resultados.size(), "!");
  vistas::lista(tft, filas, seleccion, nullptr);
  vistas::ayuda(tft, "Corto: bajar   Largo: ver detalle");
}

void dibujar() {
  switch (vista) {
    case Vista::INICIO:    vistas::inicio(tft, BUGSWEEP_VERSION); break;
    case Vista::REVISION:  dibujarRevision(); break;
    case Vista::RESULTADO: dibujarResultado(); break;
    case Vista::LISTA:   dibujarLista(); break;
    case Vista::DETALLE: dibujarDetalle(); break;
    case Vista::RASTREO: vistas::rastreo(tft, grafico, seguimientoActivo(), modo != Modo::BLUETOOTH); break;
  }
}

// ----------------------------------------------------------------- serie ---

void imprimirRedes() {
  LOG("[redes] barrido #%lu: %u redes, %u sospechosas\n", (unsigned long)escaner.barridos(),
                (unsigned)redes.size(), (unsigned)contarSospechosos(redes));
  for (const Red &r : redes)
    Serial.printf("  %-2s %3u  %4d dBm  c%-2u  %s  %-12s %s\n", simboloNivel(r.eval.nivel), r.eval.puntaje,
                  r.red.rssi, r.red.canal, vistas::macTexto(r.red.bssid).c_str(),
                  nombreFabricante(r.eval).c_str(), nombreRed(r.red).c_str());
}

void imprimirDispositivos() {
  LOG("[disp] ventana #%lu: %u dispositivos, %u sospechosos, %lu tramas descartadas\n",
                (unsigned long)sniffer.ventanas(), (unsigned)dispositivos.size(),
                (unsigned)contarSospechosos(dispositivos), (unsigned long)sniffer.descartadas());
  for (const Dispositivo &d : dispositivos)
    Serial.printf("  %-2s %3u  %4d dBm  c%-2u  sube %6s B/s  baja %6s B/s  %s  %s%s\n",
                  simboloNivel(d.eval.nivel), d.eval.puntaje, d.est.rssi, d.est.canal,
                  vistas::tasaCorta(d.est.subidaBps).c_str(), vistas::tasaCorta(d.est.bajadaBps).c_str(),
                  vistas::macTexto(d.est.mac).c_str(), nombreFabricante(d.eval).c_str(),
                  d.est.perfilVideo() ? "  [VIDEO]" : "");
}

void imprimirBle() {
  LOG("[ble] %u dispositivos, %u sospechosos\n", (unsigned)bles.size(), (unsigned)contarSospechosos(bles));
  for (const Ble &b : bles)
    Serial.printf("  %-2s %3u  %4d dBm  %-5s  %s %s  %s\n", simboloNivel(b.eval.nivel), b.eval.puntaje, b.dev.rssi,
                  minutos(b.dev.minutosVisto()).c_str(), b.dev.publica ? "pub" : "ale",
                  vistas::macTexto(b.dev.mac).c_str(), nombreBle(b).c_str());
}

// ---------------------------------------------------------------- control ---

// Apaga lo que use la radio en el modo actual y enciende lo del modo nuevo.
// Los modos son excluyentes: así cada uno tiene la radio el 100 % del tiempo.
void activarModo(Modo nuevo) {
  escaner.detener();
  sniffer.detener();
  ble.detener();
  modo = nuevo;
  switch (modo) {
    case Modo::REDES:        escaner.iniciar(); break;
    case Modo::DISPOSITIVOS: sniffer.iniciar(); procesarDispositivos(); break;
    case Modo::BLUETOOTH:    ble.iniciar(); procesarBle(); break;
  }
  seleccion = 0;
  LOG("[bugsweep] modo %s (solo recepcion)\n", nombreModo(modo));
}

// Se queda escuchando solo al objetivo: canal fijo en WiFi, escaneo continuo en BLE.
void entrarRastreo() {
  const size_t i = seleccion - 1;
  const uint8_t *mac = macDe(i);
  String titulo;
  uint8_t canal = 0;
  switch (modo) {
    case Modo::REDES:        titulo = nombreRed(redes[i].red); canal = redes[i].red.canal; break;
    case Modo::DISPOSITIVOS: titulo = nombreDispositivo(dispositivos[i]); canal = dispositivos[i].est.canal; break;
    case Modo::BLUETOOTH:    titulo = nombreBle(bles[i]); break;
  }

  if (modo == Modo::BLUETOOTH) {
    ble.seguimiento().iniciar(mac);
  } else {
    escaner.detener();
    sniffer.detener();
    sniffer.seguimiento().iniciar(mac);
    sniffer.iniciar(canal);
  }
  vista = Vista::RASTREO;
  vistas::prepararRastreo(tft, grafico, titulo, canal);
  LOG("[rastreo] %s canal %u (%s)\n", vistas::macTexto(mac).c_str(), canal, titulo.c_str());
}

// Vuelve al detalle y reanuda el modo en que estábamos.
void salirRastreo() {
  sniffer.seguimiento().detener();
  ble.seguimiento().detener();
  if (modo != Modo::BLUETOOTH) {
    const size_t sel = seleccion;
    activarModo(modo);       // reinicia el escáner o el sniffer con saltos
    seleccion = sel;
  }
  vista = Vista::DETALLE;
  LOG("[rastreo] fin\n");
}

void detenerRadios() {
  escaner.detener();
  sniffer.detener();
  ble.detener();
}

void irInicio() {
  revision.cancelar();
  detenerRadios();
  vista = Vista::INICIO;
  seleccion = 0;
}

void iniciarRevision() {
  // Partir de cero: que el resultado refleje solo este lugar y este momento.
  sniffer.limpiar();
  ble.limpiar();
  redes.clear();
  dispositivos.clear();
  bles.clear();
  revision.iniciar();
  activarModo(Modo::REDES);
  vista = Vista::REVISION;
  LOG("[revision] inicio\n");
}

template <typename T, typename N>
void agregarResultados(Modo m, const std::vector<T> &v, const char *etiqueta, N nombre) {
  for (const T &x : v) {
    if (x.eval.nivel < Nivel::MEDIO) continue;
    Resultado r;
    r.modo = m;
    r.puntaje = x.eval.puntaje;
    vistas::Fila f;
    f.nivel = x.eval.nivel;
    nombre(x, r.mac, f);
    f.dato = etiqueta;
    r.fila = f;
    resultados.push_back(r);
  }
}

void terminarRevision() {
  detenerRadios();
  revision.cancelar();
  resultados.clear();
  agregarResultados(Modo::REDES, redes, "WiFi", [](const Red &x, uint8_t *mac, vistas::Fila &f) {
    memcpy(mac, x.red.bssid, 6); f.rssi = x.red.rssi; f.nombre = nombreRed(x.red);
  });
  agregarResultados(Modo::DISPOSITIVOS, dispositivos, "Disp", [](const Dispositivo &x, uint8_t *mac, vistas::Fila &f) {
    memcpy(mac, x.est.mac, 6); f.rssi = x.est.rssi; f.nombre = nombreDispositivo(x);
  });
  agregarResultados(Modo::BLUETOOTH, bles, "BLE", [](const Ble &x, uint8_t *mac, vistas::Fila &f) {
    memcpy(mac, x.dev.mac, 6); f.rssi = x.dev.rssi; f.nombre = nombreBle(x);
  });
  std::stable_sort(resultados.begin(), resultados.end(),
                   [](const Resultado &a, const Resultado &b) { return a.puntaje > b.puntaje; });
  seleccion = 0;
  vista = Vista::RESULTADO;

  LOG("[revision] fin: %u sospechosos\n", (unsigned)resultados.size());
  for (const Resultado &r : resultados)
    Serial.printf("  %-2s %3u  %-4s  %s  %s\n", simboloNivel(r.fila.nivel), r.puntaje, r.fila.dato.c_str(),
                  vistas::macTexto(r.mac).c_str(), r.fila.nombre.c_str());
}

// Desde el resultado: abre el detalle del hallazgo en su modo (desde ahí se puede rastrear).
void abrirResultado(size_t i) {
  const Resultado r = resultados[i];
  activarModo(r.modo);
  vista = Vista::LISTA;
  for (size_t k = 0; k < cantidad(); k++)
    if (memcmp(macDe(k), r.mac, 6) == 0) { seleccion = k + 1; vista = Vista::DETALLE; break; }
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
  LOG("[bugsweep] ESP32-N4XX-BugSweep v%s\n", BUGSWEEP_VERSION);
  LOG("[bugsweep] tabla OUI: %u prefijos\n", totalPrefijosOui());
  irInicio();
  dibujar();
}

void loop() {
  bool redibujar = false;

  if (vista == Vista::RASTREO) {
    const bool nueva = modo == Modo::BLUETOOTH ? ble.actualizar() : sniffer.actualizar();
    if (nueva) {   // una muestra cada 250 ms
      const Seguimiento &s = seguimientoActivo();
      LOG("[rastreo] %4d dBm  tend %+d  ahora %6.0f B/s  base %6.0f B/s%s\n", s.rssi(),
                    s.tendencia(), s.bpsActual(), s.bpsBase(), s.pico() ? "  PICO" : "");
      redibujar = true;
    }
  } else if (vista == Vista::LISTA || vista == Vista::DETALLE || vista == Vista::REVISION) {
    switch (modo) {
      case Modo::REDES:
        if (escaner.actualizar()) { procesarRedes(); imprimirRedes(); redibujar = true; }
        if (!escaner.escaneando()) escaner.iniciar();   // continuo (y reintenta si falla)
        break;
      case Modo::DISPOSITIVOS:
        if (sniffer.actualizar()) { procesarDispositivos(); imprimirDispositivos(); redibujar = true; }
        break;
      case Modo::BLUETOOTH:
        if (ble.actualizar()) { procesarBle(); imprimirBle(); redibujar = true; }
        break;
    }
  }

  if (vista == Vista::REVISION) {
    if (revision.avanzar()) {
      if (revision.terminada()) terminarRevision();
      else activarModo(Modo(revision.indice()));   // los pasos siguen el orden de Modo
      tft.fillScreen(TFT_BLACK);
      redibujar = true;
    }
    static uint32_t tBarra = 0;                     // la barra de progreso avanza 2 veces por segundo
    if (millis() - tBarra >= 500) { tBarra = millis(); redibujar = true; }
  }

  const Boton::Evento ev = boton.leer();
  if (ev != Boton::Evento::Ninguno) {
    bool limpiar = true;   // prepararRastreo() ya limpia y dibuja su propio marco
    switch (vista) {
      case Vista::INICIO:
        if (ev == Boton::Evento::Largo) iniciarRevision();
        else { activarModo(Modo::REDES); vista = Vista::LISTA; }
        break;
      case Vista::REVISION:
        LOG("[revision] cancelada\n");
        irInicio();
        break;
      case Vista::RESULTADO:
        if (resultados.empty()) irInicio();
        else if (ev == Boton::Evento::Corto) seleccion = (seleccion + 1) % (resultados.size() + 1);
        else if (seleccion == 0) irInicio();
        else abrirResultado(seleccion - 1);
        break;
      case Vista::RASTREO:
        salirRastreo();
        break;
      case Vista::DETALLE:
        if (ev == Boton::Evento::Largo) { entrarRastreo(); limpiar = false; }
        else vista = Vista::LISTA;
        break;
      case Vista::LISTA:
        if (ev == Boton::Evento::Corto) seleccion = (seleccion + 1) % (cantidad() + 1);
        else if (seleccion == 0) {
          if (modo == Modo::BLUETOOTH) irInicio();
          else activarModo(siguienteModo());
        }
        else vista = Vista::DETALLE;
        break;
    }
    if (limpiar) tft.fillScreen(TFT_BLACK);
    redibujar = true;
  }

  if (redibujar) dibujar();
  delay(2);
}
