#include "escaner_ble.h"
#include <NimBLEDevice.h>
#include <algorithm>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

// Copia compacta de un anuncio BLE: se arma en la tarea de NimBLE y se procesa en loop().
struct AnuncioBle {
  uint8_t mac[6];
  bool publica;
  int8_t rssi;
  char nombre[21];
  uint16_t empresa;
  Rastreador rastreador;
};

namespace {
constexpr size_t LARGO_COLA = 48;
QueueHandle_t cola = nullptr;

// UUID de 16 bits asignados por el Bluetooth SIG.
constexpr uint16_t UUID_TILE = 0xFEED;
constexpr uint16_t UUID_SMARTTAG = 0xFD5A;
constexpr uint16_t UUID_EDDYSTONE = 0xFEAA;   // Google FMDN usa tramas 0x40/0x41
constexpr uint16_t EMPRESA_APPLE = 0x004C;
constexpr uint8_t APPLE_FIND_MY = 0x12;
constexpr uint8_t FIND_MY_LARGO_SEPARADO = 0x19;   // 25 bytes: lejos del dueño

Rastreador detectar(NimBLEAdvertisedDevice *d, const std::string &fab) {
  // Apple: [4C 00] [tipo 0x12] [largo] ...
  if (fab.size() >= 4 && (uint8_t)fab[0] == 0x4C && (uint8_t)fab[1] == 0x00 &&
      (uint8_t)fab[2] == APPLE_FIND_MY) {
    return (uint8_t)fab[3] == FIND_MY_LARGO_SEPARADO ? Rastreador::FIND_MY_SEPARADO
                                                     : Rastreador::FIND_MY_CERCA;
  }
  if (d->isAdvertisingService(NimBLEUUID(UUID_TILE))) return Rastreador::TILE;
  for (size_t i = 0; i < d->getServiceDataCount(); i++) {
    const NimBLEUUID uuid = d->getServiceDataUUID(i);
    if (uuid == NimBLEUUID(UUID_TILE)) return Rastreador::TILE;
    if (uuid == NimBLEUUID(UUID_SMARTTAG)) return Rastreador::SMARTTAG;
    if (uuid == NimBLEUUID(UUID_EDDYSTONE)) {
      const std::string datos = d->getServiceData(i);
      if (!datos.empty() && ((uint8_t)datos[0] == 0x40 || (uint8_t)datos[0] == 0x41))
        return Rastreador::GOOGLE_FMDN;
    }
  }
  return Rastreador::NINGUNO;
}

class AlRecibir : public NimBLEAdvertisedDeviceCallbacks {
  void onResult(NimBLEAdvertisedDevice *d) override {
    AnuncioBle a{};
    const uint8_t *nativa = d->getAddress().getNative();   // viene al revés (LSB primero)
    for (int i = 0; i < 6; i++) a.mac[i] = nativa[5 - i];
    a.publica = d->getAddressType() == BLE_ADDR_PUBLIC;
    a.rssi = d->getRSSI();
    if (d->haveName()) strlcpy(a.nombre, d->getName().c_str(), sizeof a.nombre);
    const std::string fab = d->haveManufacturerData() ? d->getManufacturerData() : std::string();
    a.empresa = fab.size() >= 2 ? (uint8_t)fab[0] | (uint8_t)fab[1] << 8 : 0xFFFF;
    a.rastreador = detectar(d, fab);
    xQueueSend(cola, &a, 0);   // si la cola está llena, se pierde este anuncio (llegará otro)
  }
};
AlRecibir alRecibir;
}  // namespace

const char *nombreEmpresaBle(uint16_t id) {
  switch (id) {
    case 0x004C: return "Apple";
    case 0x0006: return "Microsoft";
    case 0x0075: return "Samsung";
    case 0x00E0: return "Google";
    case 0x0087: return "Garmin";
    case 0x0059: return "Nordic";
    case 0x02E5: return "Espressif";
    case 0x038F: return "Xiaomi";
    case 0x0157: return "Huami";
    default:     return nullptr;
  }
}

void EscanerBle::iniciar() {
  if (activo_) return;
  if (!iniciado_) {
    cola = xQueueCreate(LARGO_COLA, sizeof(AnuncioBle));
    NimBLEDevice::init("");
    NimBLEScan *s = NimBLEDevice::getScan();
    s->setAdvertisedDeviceCallbacks(&alRecibir, true);   // true = también repetidos (RSSI al día)
    s->setDuplicateFilter(false);
    s->setActiveScan(false);     // PASIVO: no envía scan requests
    s->setInterval(100);
    s->setWindow(99);
    s->setMaxResults(0);         // no guardar resultados dentro de NimBLE: usamos la cola
    dispositivos_.reserve(MAX_DISPOSITIVOS);
    iniciado_ = true;
  }
  xQueueReset(cola);
  NimBLEDevice::getScan()->start(0, nullptr, false);   // 0 = escanear sin fin
  tRefresco_ = millis();
  activo_ = true;
}

void EscanerBle::detener() {
  if (!activo_) return;
  NimBLEDevice::getScan()->stop();
  activo_ = false;
}

bool EscanerBle::actualizar() {
  if (!activo_) return false;
  AnuncioBle a;
  for (int i = 0; i < 32 && xQueueReceive(cola, &a, 0) == pdTRUE; i++) procesar(a);

  if (seguimiento_.activo()) return seguimiento_.tick();

  const uint32_t ahora = millis();
  if (ahora - tRefresco_ < MS_REFRESCO) return false;
  tRefresco_ = ahora;
  dispositivos_.erase(std::remove_if(dispositivos_.begin(), dispositivos_.end(),
                                     [&](const DispositivoBle &d) { return ahora - d.ultimoMs > OLVIDAR_MS; }),
                      dispositivos_.end());
  return true;
}

void EscanerBle::procesar(const AnuncioBle &a) {
  const uint32_t ahora = millis();
  if (seguimiento_.esObjetivo(a.mac)) seguimiento_.registrar(0, a.rssi);   // en BLE solo interesa la señal

  auto it = std::find_if(dispositivos_.begin(), dispositivos_.end(),
                         [&](const DispositivoBle &d) { return memcmp(d.mac, a.mac, 6) == 0; });
  if (it == dispositivos_.end()) {
    if (dispositivos_.size() >= MAX_DISPOSITIVOS) {
      dispositivos_.erase(std::min_element(dispositivos_.begin(), dispositivos_.end(),
                                           [](const DispositivoBle &x, const DispositivoBle &y) {
                                             return x.ultimoMs < y.ultimoMs;
                                           }));
    }
    DispositivoBle d;
    memcpy(d.mac, a.mac, 6);
    d.publica = a.publica;
    d.rssi = a.rssi;
    d.primerMs = ahora;
    dispositivos_.push_back(d);
    it = dispositivos_.end() - 1;
  }
  DispositivoBle &d = *it;
  d.rssi = (d.rssi * 3 + a.rssi) / 4;
  d.ultimoMs = ahora;
  d.anuncios++;
  // Los anuncios alternan datos: conservar lo último conocido de cada campo.
  if (a.nombre[0]) strlcpy(d.nombre, a.nombre, sizeof d.nombre);
  if (a.empresa != 0xFFFF) d.empresa = a.empresa;
  if (a.rastreador != Rastreador::NINGUNO) d.rastreador = a.rastreador;
}
