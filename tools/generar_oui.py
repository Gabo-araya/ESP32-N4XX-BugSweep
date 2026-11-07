#!/usr/bin/env python3
"""Genera src/rules/oui_tabla.h a partir del registro oficial de OUI del IEEE.

Uso:
    python3 -I tools/generar_oui.py                  # descarga oui.csv del IEEE
    python3 -I tools/generar_oui.py ruta/a/oui.csv   # usa un archivo ya descargado

Solo se incluyen bloques MA-L (prefijos de 24 bits). Para agregar un fabricante,
edita FABRICANTES y vuelve a ejecutar el script. No edites oui_tabla.h a mano.
"""
import csv
import datetime
import io
import pathlib
import re
import sys
import urllib.request

URL = "https://standards-oui.ieee.org/oui/oui.csv"
SALIDA = pathlib.Path(__file__).resolve().parent.parent / "src" / "rules" / "oui_tabla.h"

# (nombre corto, categoría, regex sobre "Organization Name" del IEEE)
# CAMARA      = fabricante de cámaras / videovigilancia
# CHIP_CAMARA = fabricante de chips o módulos muy usados en cámaras baratas
# IOT         = chip IoT genérico: está en cámaras, pero también en enchufes, ampolletas, etc.
FABRICANTES = [
    ("Hikvision",   "CAMARA",      r"hikvision"),
    ("Ezviz",       "CAMARA",      r"ezviz"),
    ("Dahua",       "CAMARA",      r"dahua"),
    ("Uniview",     "CAMARA",      r"uniview"),
    ("Axis",        "CAMARA",      r"^axis communications"),
    ("Hanwha",      "CAMARA",      r"hanwha vision|samsung techwin"),
    ("Reolink",     "CAMARA",      r"reolink"),
    ("Arlo",        "CAMARA",      r"^arlo technolog"),
    ("Ring",        "CAMARA",      r"^ring llc"),
    ("Wyze",        "CAMARA",      r"wyze labs"),
    ("Amcrest",     "CAMARA",      r"amcrest"),
    ("Gwell/Yoosee","CAMARA",      r"gwelltimes"),
    ("HiSilicon",   "CHIP_CAMARA", r"hisilicon"),
    ("Ingenic",     "CHIP_CAMARA", r"ingenic semiconductor"),
    ("Novatek",     "CHIP_CAMARA", r"novatek microelectronics"),
    ("Goke",        "CHIP_CAMARA", r"goke microelectronics"),
    ("AltoBeam",    "CHIP_CAMARA", r"altobeam"),
    ("Bilian",      "CHIP_CAMARA", r"shenzhen bilian"),
    ("Fn-Link",     "CHIP_CAMARA", r"fn-link"),
    ("Ogemray",     "CHIP_CAMARA", r"ogemray"),
    ("Beken",       "CHIP_CAMARA", r"beken"),
    ("Tuya",        "IOT",         r"tuya smart"),
    ("Espressif",   "IOT",         r"espressif"),
    ("Realtek",     "IOT",         r"realtek semiconductor"),
]


def leer_csv(argv):
    if len(argv) > 1:
        return pathlib.Path(argv[1]).read_text(encoding="utf-8")
    req = urllib.request.Request(URL, headers={"User-Agent": "ESP32-N4XX-BugSweep generar_oui.py"})
    with urllib.request.urlopen(req, timeout=60) as r:
        return r.read().decode("utf-8")


def main():
    regex = [(n, c, re.compile(p, re.I)) for n, c, p in FABRICANTES]
    entradas = {}
    for fila in csv.DictReader(io.StringIO(leer_csv(sys.argv))):
        if fila["Registry"] != "MA-L":
            continue
        org = fila["Organization Name"].strip()
        for i, (_, _, rx) in enumerate(regex):
            if rx.search(org):
                entradas[int(fila["Assignment"], 16)] = i
                break

    conteo = [0] * len(FABRICANTES)
    for i in entradas.values():
        conteo[i] += 1

    lineas = [
        "// GENERADO por tools/generar_oui.py — no editar a mano.",
        f"// Fuente: {URL} (bloques MA-L). Generado: {datetime.date.today().isoformat()}",
        "#pragma once",
        "#include <stdint.h>",
        '#include "categoria.h"',
        "",
        "struct Fabricante { const char *nombre; Categoria categoria; };",
        "struct EntradaOui { uint32_t oui; uint8_t fabricante; };",
        "",
        "static const Fabricante FABRICANTES[] = {",
    ]
    for (n, c, _), k in zip(FABRICANTES, conteo):
        lineas.append(f'    {{"{n}", Categoria::{c}}},  // {k} OUIs')
    lineas += ["};", "", "// Ordenada por OUI para búsqueda binaria. Vive en flash (const).",
               "static const EntradaOui TABLA_OUI[] = {"]
    for oui in sorted(entradas):
        lineas.append(f"    {{0x{oui:06X}, {entradas[oui]}}},")
    lineas += ["};", f"static const unsigned TOTAL_OUI = {len(entradas)};", ""]

    SALIDA.write_text("\n".join(lineas), encoding="utf-8")
    print(f"{SALIDA}: {len(entradas)} OUIs")
    for (n, _, _), k in zip(FABRICANTES, conteo):
        print(f"  {n:14} {k}")


if __name__ == "__main__":
    main()
