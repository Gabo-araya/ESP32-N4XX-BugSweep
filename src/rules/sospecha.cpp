#include "sospecha.h"
#include "oui_tabla.h"

namespace {

enum class Modo : uint8_t { PREFIJO, CONTIENE };

struct PatronSsid {
  const char *texto;   // en minúsculas y solo letras/números (ver normalizar())
  Modo modo;
  uint8_t puntos;
  const char *motivo;
};

// Nombres de red que crean cámaras al configurarse o al funcionar como punto de acceso.
// Se usa solo el patrón de mayor puntaje, para no sumar "hdwificam" + "wificam" + "cam".
const PatronSsid PATRONES[] = {
    {"hdwificam", Modo::CONTIENE, 75, "Nombre de camara espia (HDWiFiCam)"},
    {"spycam",    Modo::CONTIENE, 75, "Nombre de camara espia"},
    {"lookcam",   Modo::CONTIENE, 70, "Nombre de camara espia (LookCam)"},
    {"v380",      Modo::CONTIENE, 65, "Nombre de camara V380"},
    {"gwap",      Modo::PREFIJO,  65, "Red de config. camara Yoosee (GW_AP)"},
    {"wificam",   Modo::CONTIENE, 65, "Nombre de camara WiFi"},
    {"cloudcam",  Modo::CONTIENE, 65, "Nombre de camara WiFi"},
    {"minicam",   Modo::CONTIENE, 65, "Nombre de mini camara"},
    {"ipcam",     Modo::CONTIENE, 60, "Nombre de camara IP"},
    {"ipc",       Modo::PREFIJO,  45, "Nombre tipo camara IP (IPC...)"},
    {"camera",    Modo::CONTIENE, 45, "El nombre contiene 'camera'"},
    {"smartlife", Modo::PREFIJO,  30, "Red de config. Tuya/SmartLife"},
    {"cam",       Modo::CONTIENE, 25, "El nombre contiene 'cam'"},
};

// Puntos por categoría de fabricante.
uint8_t puntosCategoria(Categoria c) {
  switch (c) {
    case Categoria::CAMARA:      return 60;
    case Categoria::CHIP_CAMARA: return 35;
    case Categoria::IOT:         return 15;
  }
  return 0;
}

const char *motivoCategoria(Categoria c) {
  switch (c) {
    case Categoria::CAMARA:      return "Fabricante de camaras";
    case Categoria::CHIP_CAMARA: return "Chip comun en camaras baratas";
    case Categoria::IOT:         return "Chip IoT (camaras, enchufes...)";
  }
  return "";
}

constexpr uint8_t PUNTOS_OCULTA = 15;

// "HD-WiFi_Cam 01" -> "hdwificam01"
String normalizar(const String &s) {
  String r;
  r.reserve(s.length());
  for (char c : s) {
    if (isalnum((unsigned char)c)) r += (char)tolower((unsigned char)c);
  }
  return r;
}

void agregarMotivo(Evaluacion &e, const char *m) {
  if (e.nMotivos < Evaluacion::MAX_MOTIVOS) e.motivos[e.nMotivos++] = m;
}

}  // namespace

unsigned totalPrefijosOui() { return TOTAL_OUI; }

InfoFabricante buscarFabricante(const uint8_t mac[6]) {
  const uint32_t oui = (uint32_t)mac[0] << 16 | (uint32_t)mac[1] << 8 | mac[2];
  int bajo = 0, alto = (int)TOTAL_OUI - 1;
  while (bajo <= alto) {                 // búsqueda binaria: tabla ordenada
    const int medio = (bajo + alto) / 2;
    const uint32_t v = TABLA_OUI[medio].oui;
    if (v == oui) {
      const Fabricante &f = FABRICANTES[TABLA_OUI[medio].fabricante];
      return {f.nombre, f.categoria};
    }
    if (v < oui) bajo = medio + 1; else alto = medio - 1;
  }
  return {nullptr, Categoria::IOT};
}

Evaluacion evaluarRed(const String &ssid, const uint8_t bssid[6]) {
  Evaluacion e;
  uint16_t total = 0;

  // 1) Nombre de la red
  if (ssid.isEmpty()) {
    total += PUNTOS_OCULTA;
    agregarMotivo(e, "Red oculta");
  } else {
    const String n = normalizar(ssid);
    const PatronSsid *mejor = nullptr;
    for (const PatronSsid &p : PATRONES) {
      const bool coincide = p.modo == Modo::PREFIJO ? n.startsWith(p.texto) : n.indexOf(p.texto) >= 0;
      if (coincide && (!mejor || p.puntos > mejor->puntos)) mejor = &p;
    }
    if (mejor) {
      total += mejor->puntos;
      agregarMotivo(e, mejor->motivo);
    }
  }

  // 2) Fabricante
  e.macAleatoria = esMacAleatoria(bssid);
  if (e.macAleatoria) {
    agregarMotivo(e, "MAC aleatoria: fabricante oculto");
  } else {
    e.fabricante = buscarFabricante(bssid);
    if (e.fabricante.nombre) {
      total += puntosCategoria(e.fabricante.categoria);
      agregarMotivo(e, motivoCategoria(e.fabricante.categoria));
    }
  }

  e.puntaje = total > 100 ? 100 : total;
  e.nivel = e.puntaje >= 60 ? Nivel::ALTO
          : e.puntaje >= 30 ? Nivel::MEDIO
          : e.puntaje >= 15 ? Nivel::BAJO
                            : Nivel::NINGUNO;
  return e;
}

const char *simboloNivel(Nivel n) {
  switch (n) {
    case Nivel::ALTO:    return "!!";
    case Nivel::MEDIO:   return "!";
    case Nivel::BAJO:    return "?";
    case Nivel::NINGUNO: return "";
  }
  return "";
}

const char *textoNivel(Nivel n) {
  switch (n) {
    case Nivel::ALTO:    return "ALTA";
    case Nivel::MEDIO:   return "MEDIA";
    case Nivel::BAJO:    return "BAJA";
    case Nivel::NINGUNO: return "ninguna";
  }
  return "";
}

const char *textoCategoria(Categoria c) {
  switch (c) {
    case Categoria::CAMARA:      return "camaras";
    case Categoria::CHIP_CAMARA: return "chips de camara";
    case Categoria::IOT:         return "IoT";
  }
  return "";
}
