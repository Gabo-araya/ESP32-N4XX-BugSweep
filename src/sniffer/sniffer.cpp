#include "sniffer.h"
#include <algorithm>
#include <esp_wifi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

// Copia compacta de lo que importa de cada trama. Nunca se guarda el contenido
// (payload) de los datos: solo direcciones, tamaño, señal y, en beacons, el SSID.
struct Trama {
  uint8_t a1[6], a2[6];
  uint16_t largo;
  int8_t rssi;
  uint8_t canal, tipo, subtipo, flags, largoSsid;
  char ssid[32];
};

namespace {
constexpr size_t LARGO_COLA = 96;
QueueHandle_t cola = nullptr;
volatile uint32_t descartadas = 0;

constexpr uint8_t TIPO_GESTION = 0, TIPO_DATOS = 2;
// Subtipos de gestión que TRANSMITE una estación (no el AP):
// 0 = solicitud de asociación, 2 = reasociación, 4 = probe request.
constexpr uint8_t SUB_ASOC_REQ = 0, SUB_REASOC_REQ = 2, SUB_PROBE_REQ = 4, SUB_BEACON = 8;
constexpr uint8_t TO_DS = 0x01, FROM_DS = 0x02;

bool esMulticast(const uint8_t m[6]) { return m[0] & 0x01; }

// Corre en la tarea del WiFi: debe ser muy rápido y no bloquear nunca.
void alRecibir(void *buf, wifi_promiscuous_pkt_type_t) {
  const auto *p = static_cast<const wifi_promiscuous_pkt_t *>(buf);
  const uint16_t largo = p->rx_ctrl.sig_len;   // incluye 4 bytes de FCS
  if (largo < 28) return;
  const uint8_t *f = p->payload;

  Trama t;
  t.tipo = (f[0] >> 2) & 0x3;
  t.subtipo = (f[0] >> 4) & 0xF;
  t.flags = f[1];
  memcpy(t.a1, f + 4, 6);
  memcpy(t.a2, f + 10, 6);
  t.largo = largo;
  t.rssi = p->rx_ctrl.rssi;
  t.canal = p->rx_ctrl.channel;
  t.largoSsid = 0;

  // Beacon: cabecera 24 + campos fijos 12; luego el elemento SSID (id 0).
  if (t.tipo == TIPO_GESTION && t.subtipo == SUB_BEACON && largo >= 24 + 12 + 2 + 4) {
    const uint8_t *ie = f + 36;
    const uint8_t l = ie[1];
    if (ie[0] == 0 && l <= 32 && 36 + 2 + l <= largo - 4) {
      memcpy(t.ssid, ie + 2, l);
      t.largoSsid = l;
    }
  }
  if (xQueueSend(cola, &t, 0) != pdTRUE) descartadas++;
}
}  // namespace

bool Estacion::perfilVideo() const { return ventanasAltas >= Sniffer::VENTANAS_VIDEO; }

void Sniffer::begin() {
  if (!cola) cola = xQueueCreate(LARGO_COLA, sizeof(Trama));
  estaciones_.reserve(MAX_ESTACIONES);
  aps_.reserve(MAX_APS);
}

void Sniffer::iniciar(uint8_t canalFijo) {
  if (activo_) return;
  wifi_promiscuous_filter_t filtro = {};
  filtro.filter_mask = WIFI_PROMIS_FILTER_MASK_MGMT | WIFI_PROMIS_FILTER_MASK_DATA;
  esp_wifi_set_promiscuous_filter(&filtro);
  esp_wifi_set_promiscuous_rx_cb(alRecibir);
  esp_wifi_set_promiscuous(true);
  canalFijo_ = canalFijo;
  canal_ = canalFijo ? canalFijo : 1;
  esp_wifi_set_channel(canal_, WIFI_SECOND_CHAN_NONE);
  tCanal_ = millis();
  saltos_ = 0;
  memset(msEscuchado_, 0, sizeof msEscuchado_);
  // Descartar bytes acumulados en otro modo: si no, la primera ventana los
  // dividiría por un tiempo corto y daría falsas alarmas de video.
  for (Estacion &e : estaciones_) e.bytesSubidaV = e.bytesBajadaV = 0;
  activo_ = true;
}

void Sniffer::detener() {
  if (!activo_) return;
  esp_wifi_set_promiscuous(false);
  xQueueReset(cola);
  activo_ = false;
}

uint32_t Sniffer::descartadas() const { return ::descartadas; }

bool Sniffer::actualizar() {
  if (!activo_) return false;

  // Procesa lo acumulado, con tope para no acaparar el loop.
  Trama t;
  for (int i = 0; i < 64 && xQueueReceive(cola, &t, 0) == pdTRUE; i++) procesar(t);

  if (canalFijo_) return seguimiento_.tick();

  if (millis() - tCanal_ < MS_POR_CANAL) return false;
  saltarCanal();
  if (saltos_ < CANALES * VUELTAS_POR_VENTANA) return false;
  cerrarVentana();
  return true;
}

void Sniffer::saltarCanal() {
  const uint32_t ahora = millis();
  msEscuchado_[canal_] += ahora - tCanal_;
  canal_ = canal_ % CANALES + 1;
  esp_wifi_set_channel(canal_, WIFI_SECOND_CHAN_NONE);
  tCanal_ = ahora;
  saltos_++;
}

void Sniffer::cerrarVentana() {
  const uint32_t ahora = millis();
  for (Estacion &e : estaciones_) {
    const uint32_t ms = msEscuchado_[e.canal];
    if (ms > 0) {
      // Solo escuchamos su canal una fracción del tiempo: se escala por lo escuchado.
      const float sub = e.bytesSubidaV * 1000.0f / ms;
      const float baj = e.bytesBajadaV * 1000.0f / ms;
      e.subidaBps = 0.5f * e.subidaBps + 0.5f * sub;    // suavizado exponencial
      e.bajadaBps = 0.5f * e.bajadaBps + 0.5f * baj;
      if (sub >= UMBRAL_VIDEO_BPS) {
        if (e.ventanasAltas < 255) e.ventanasAltas++;
      } else {
        e.ventanasAltas = 0;
      }
    }
    e.bytesSubidaV = e.bytesBajadaV = 0;
  }
  // Olvidar dispositivos que ya no se ven.
  estaciones_.erase(std::remove_if(estaciones_.begin(), estaciones_.end(),
                                   [&](const Estacion &e) { return ahora - e.vistoMs > OLVIDAR_MS; }),
                    estaciones_.end());
  memset(msEscuchado_, 0, sizeof msEscuchado_);
  saltos_ = 0;
  ventanas_++;
}

const PuntoAcceso *Sniffer::buscarAp(const uint8_t bssid[6]) const {
  for (const PuntoAcceso &a : aps_)
    if (memcmp(a.bssid, bssid, 6) == 0) return &a;
  return nullptr;
}

bool Sniffer::esAp(const uint8_t mac[6]) const { return buscarAp(mac) != nullptr; }

void Sniffer::registrarAp(const Trama &t) {
  if (esAp(t.a2) || aps_.size() >= MAX_APS) return;
  PuntoAcceso a;
  memcpy(a.bssid, t.a2, 6);
  memcpy(a.ssid, t.ssid, t.largoSsid);
  a.ssid[t.largoSsid] = '\0';   // SSID vacío o relleno de ceros = red oculta
  a.canal = t.canal;
  aps_.push_back(a);
  // Si la habíamos tomado por estación (p. ej. por una trama de gestión), sacarla.
  estaciones_.erase(std::remove_if(estaciones_.begin(), estaciones_.end(),
                                   [&](const Estacion &e) { return memcmp(e.mac, t.a2, 6) == 0; }),
                    estaciones_.end());
}

Estacion *Sniffer::estacion(const uint8_t mac[6], bool crear) {
  if (esMulticast(mac) || esAp(mac)) return nullptr;
  for (Estacion &e : estaciones_)
    if (memcmp(e.mac, mac, 6) == 0) return &e;
  if (!crear) return nullptr;
  if (estaciones_.size() >= MAX_ESTACIONES) {
    // Tabla llena: reemplazar la que lleva más tiempo sin verse.
    auto viejo = std::min_element(estaciones_.begin(), estaciones_.end(),
                                  [](const Estacion &a, const Estacion &b) { return a.vistoMs < b.vistoMs; });
    estaciones_.erase(viejo);
  }
  Estacion e;
  memcpy(e.mac, mac, 6);
  memset(e.bssid, 0, 6);
  estaciones_.push_back(e);
  return &estaciones_.back();
}

void Sniffer::procesar(const Trama &t) {
  // Rastreo: todo lo que TRANSMITE el objetivo (a2), sea estación o AP.
  // Una cámara en modo AP manda su video como AP, así que también cuenta.
  if (seguimiento_.esObjetivo(t.a2)) seguimiento_.registrar(t.largo, t.rssi);

  if (t.tipo == TIPO_GESTION) {
    if (t.subtipo == SUB_BEACON) { registrarAp(t); return; }
    if (t.subtipo != SUB_ASOC_REQ && t.subtipo != SUB_REASOC_REQ && t.subtipo != SUB_PROBE_REQ) return;
    Estacion *e = estacion(t.a2, true);   // a2 = la estación que transmite
    if (!e) return;
    e->rssi = e->tramas ? (e->rssi * 3 + t.rssi) / 4 : t.rssi;
    e->canal = t.canal;
    e->vistoMs = millis();
    e->tramas++;
    if (t.subtipo != SUB_PROBE_REQ && !esMulticast(t.a1)) {   // asociación: a1 = AP
      memcpy(e->bssid, t.a1, 6);
      e->conBssid = true;
    }
    return;
  }
  if (t.tipo != TIPO_DATOS) return;

  const bool toDs = t.flags & TO_DS, fromDs = t.flags & FROM_DS;
  if (toDs && !fromDs) {
    // Estación -> AP: SUBIDA. a1 = AP (BSSID), a2 = estación.
    Estacion *e = estacion(t.a2, true);
    if (!e) return;
    e->bytesSubidaV += t.largo;
    e->rssi = e->tramas ? (e->rssi * 3 + t.rssi) / 4 : t.rssi;
    e->canal = t.canal;
    e->vistoMs = millis();
    e->tramas++;
    memcpy(e->bssid, t.a1, 6);
    e->conBssid = true;
  } else if (fromDs && !toDs) {
    // AP -> estación: BAJADA. a1 = estación, a2 = AP. El RSSI es del AP: no se usa.
    Estacion *e = estacion(t.a1, true);
    if (!e) return;
    e->bytesBajadaV += t.largo;
    e->canal = t.canal;
    e->vistoMs = millis();
    e->tramas++;
    memcpy(e->bssid, t.a2, 6);
    e->conBssid = true;
  }
  // toDS y fromDS juntos (malla/WDS) o ninguno (ad-hoc): se ignoran por ahora.
}
