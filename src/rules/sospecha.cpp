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
constexpr uint8_t PUNTOS_VIDEO = 45;
constexpr uint8_t PUNTOS_ACOMPANA = 15;      // rastreador visto por 10+ minutos
constexpr uint32_t MINUTOS_ACOMPANA = 10;

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

// Partes comunes a redes y dispositivos -------------------------------------
// Nombre (SSID o nombre BLE): solo cuenta el patrón de mayor puntaje.
static void sumarNombre(Evaluacion &e, const String &nombre, uint16_t &total) {
  const String n = normalizar(nombre);
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

static void sumarFabricante(Evaluacion &e, const uint8_t mac[6], uint16_t &total) {
  e.macAleatoria = esMacAleatoria(mac);
  if (e.macAleatoria) {
    agregarMotivo(e, "MAC aleatoria: fabricante oculto");
    return;
  }
  e.fabricante = buscarFabricante(mac);
  if (e.fabricante.nombre) {
    total += puntosCategoria(e.fabricante.categoria);
    agregarMotivo(e, motivoCategoria(e.fabricante.categoria));
  }
}

static void cerrar(Evaluacion &e, uint16_t total) {
  e.puntaje = total > 100 ? 100 : total;
  e.nivel = e.puntaje >= 60 ? Nivel::ALTO
          : e.puntaje >= 30 ? Nivel::MEDIO
          : e.puntaje >= 15 ? Nivel::BAJO
                            : Nivel::NINGUNO;
}

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
    sumarNombre(e, ssid, total);
  }

  // 2) Fabricante
  sumarFabricante(e, bssid, total);
  cerrar(e, total);
  return e;
}

Evaluacion evaluarEstacion(const uint8_t mac[6], bool perfilVideo) {
  Evaluacion e;
  uint16_t total = 0;
  if (perfilVideo) {
    total += PUNTOS_VIDEO;
    agregarMotivo(e, "Sube datos sin parar (como video)");
  }
  sumarFabricante(e, mac, total);
  cerrar(e, total);
  return e;
}

namespace {
struct InfoRastreador { uint8_t puntos; const char *motivo; const char *corto; };

InfoRastreador infoRastreador(Rastreador r) {
  switch (r) {
    case Rastreador::FIND_MY_SEPARADO: return {70, "Rastreador Find My lejos de su dueno", "AirTag/FindMy"};
    case Rastreador::FIND_MY_CERCA:    return {25, "Accesorio Find My (dueno cerca)", "FindMy (dueno)"};
    case Rastreador::TILE:             return {60, "Rastreador Tile", "Tile"};
    case Rastreador::SMARTTAG:         return {60, "Rastreador Samsung SmartTag", "SmartTag"};
    case Rastreador::GOOGLE_FMDN:      return {50, "Posible rastreador red Google", "Google FMDN"};
    case Rastreador::NINGUNO:          break;
  }
  return {0, "", ""};
}
}  // namespace

const char *textoRastreador(Rastreador r) { return infoRastreador(r).corto; }

Evaluacion evaluarBle(const String &nombre, const uint8_t mac[6], bool publica, Rastreador r,
                      uint32_t minutosVisto) {
  Evaluacion e;
  uint16_t total = 0;
  if (r != Rastreador::NINGUNO) {
    const InfoRastreador i = infoRastreador(r);
    total += i.puntos;
    agregarMotivo(e, i.motivo);
    if (minutosVisto >= MINUTOS_ACOMPANA && r != Rastreador::FIND_MY_CERCA) {
      total += PUNTOS_ACOMPANA;
      agregarMotivo(e, "Te acompana hace 10+ minutos");
    }
  }
  if (!nombre.isEmpty()) sumarNombre(e, nombre, total);
  // En BLE la dirección aleatoria es lo normal: no se anota como motivo.
  if (publica) sumarFabricante(e, mac, total);
  cerrar(e, total);
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
