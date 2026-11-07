#pragma once
#include <stdint.h>

// Qué tan asociado está un fabricante a cámaras. Ver tools/generar_oui.py.
enum class Categoria : uint8_t { CAMARA, CHIP_CAMARA, IOT };
