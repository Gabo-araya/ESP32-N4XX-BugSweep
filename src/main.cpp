// Paso 1b: leer el botón BOOT (GPIO0) y distinguir pulsación corta y larga.
#include <Arduino.h>
#include <TFT_eSPI.h>
#include "ui/boton.h"

constexpr uint8_t PIN_BOTON = 0;  // BOOT: junto al conector USB-C

TFT_eSPI tft;
Boton boton(PIN_BOTON);

uint32_t cortos = 0, largos = 0;

// Dibuja una línea de texto borrando el fondo; el relleno con espacios evita
// que queden restos cuando el texto nuevo es más corto que el anterior.
void linea(const String &texto, int y, uint16_t color, uint8_t fuente = 4) {
  tft.setTextFont(fuente);
  tft.setTextColor(color, TFT_BLACK);
  tft.setTextPadding(tft.width() - 20);
  tft.drawString(texto, 10, y);
}

void mostrarContadores() {
  linea(String("Cortas: ") + cortos, 70, TFT_WHITE);
  linea(String("Largas: ") + largos, 100, TFT_WHITE);
}

void setup() {
  Serial.begin(115200);
  delay(200);
  boton.begin();

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  linea("Prueba de boton", 6, TFT_YELLOW);
  linea("BOOT: corto <0.6s, largo >0.6s", 38, TFT_CYAN, 2);
  mostrarContadores();
  Serial.println("[paso1b] Listo. Aprieta BOOT.");
}

void loop() {
  switch (boton.leer()) {
    case Boton::Evento::Corto:
      cortos++;
      linea("> CORTO", 136, TFT_YELLOW);   // texto + color: no depende solo del color
      Serial.printf("[paso1b] CORTO (total %lu)\n", (unsigned long)cortos);
      mostrarContadores();
      break;
    case Boton::Evento::Largo:
      largos++;
      linea(">> LARGO", 136, TFT_CYAN);
      Serial.printf("[paso1b] LARGO (total %lu)\n", (unsigned long)largos);
      mostrarContadores();
      break;
    case Boton::Evento::Ninguno:
      break;
  }
  delay(5);  // cede tiempo al sistema; 5 ms no afecta la respuesta del botón
}
