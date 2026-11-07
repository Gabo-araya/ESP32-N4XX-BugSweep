#!/usr/bin/env python3
"""Seudonimiza una captura del monitor serie antes de compartirla.

Uso:
    python3 -I tools/anonimizar.py capturas/calle-2026-11-07.txt
    -> capturas/calle-2026-11-07.anon.txt

Qué hace:
  - MAC completas: conserva el fabricante (OUI, 3 primeros bytes) y reemplaza el
    resto por un código estable:  A4:CF:12:9B:31:07 -> A4:CF:12:x5e0c1d
  - Finales de MAC sueltos ("Apple 3C:1F:A0"): mismo código que su MAC completa.
  - Nombres de redes WiFi y de dispositivos BLE: se reemplazan por un código
    SALVO en las filas marcadas como sospechosas (!!, !, ?), que son las que
    sirven para evaluar la detección.

El código se calcula con una "sal" secreta guardada en capturas/.sal (que no se
comparte): el mismo equipo tiene el mismo código en todas tus capturas, pero
nadie puede revertirlo a la MAC real.
"""
import hashlib
import hmac
import os
import pathlib
import re
import sys

HEX = r"[0-9A-Fa-f]{2}"
# Una sola pasada: primero MAC completa, si no, un final de 3 bytes aislado.
RE_MAC = re.compile(rf"(?<![0-9A-Fa-f:])(?:({HEX}:{HEX}:{HEX}):({HEX}:{HEX}:{HEX})|({HEX}:{HEX}:{HEX}))(?![0-9A-Fa-f:])")
# Filas de las listas (ver imprimirRedes / imprimirBle en src/main.cpp).
RE_RED = re.compile(r"^(  (.{2}) +\d+ +-?\d+ dBm +c\d+ +\S+ +\S+ +)(\(abierta\) )?(.*)$")
RE_BLE = re.compile(r"^(  (.{2}) +\d+ +-?\d+ dBm +\S+ +(?:pub|ale) \S+  )(.*)$")
# Nombres BLE generados por el firmware ("BLE x1a2b3c", "Apple x1a2b3c"): no son personales.
RE_BLE_GENERICO = re.compile(r"^[A-Za-z][\w/.-]* x[0-9a-f]{6}$")


def sal(carpeta: pathlib.Path) -> bytes:
    archivo = carpeta / ".sal"
    if not archivo.exists():
        carpeta.mkdir(parents=True, exist_ok=True)
        archivo.write_bytes(os.urandom(32))
        archivo.chmod(0o600)
    return archivo.read_bytes()


def codigo(clave: bytes, texto: str, largo: int = 6) -> str:
    return hmac.new(clave, texto.upper().encode(), hashlib.sha256).hexdigest()[:largo]


def anonimizar_linea(linea: str, clave: bytes, cuenta: dict) -> str:
    def mac(m):
        cuenta["mac"] += 1
        if m.group(1):
            return f"{m.group(1).upper()}:x{codigo(clave, m.group(2))}"
        return f"x{codigo(clave, m.group(3))}"

    linea = RE_MAC.sub(mac, linea)

    m = RE_RED.match(linea)
    if m and not m.group(2).strip() and m.group(4) != "<oculta>":
        cuenta["nombre"] += 1
        return f"{m.group(1)}{m.group(3) or ''}red-{codigo(clave, m.group(4))}"

    m = RE_BLE.match(linea)
    if m and not m.group(2).strip() and not RE_BLE_GENERICO.match(m.group(3)):
        cuenta["nombre"] += 1
        return f"{m.group(1)}nombre-{codigo(clave, m.group(3))}"
    return linea


def main():
    if len(sys.argv) != 2:
        sys.exit(__doc__)
    entrada = pathlib.Path(sys.argv[1])
    clave = sal(pathlib.Path(__file__).resolve().parent.parent / "capturas")
    salida = entrada.with_suffix(".anon" + entrada.suffix)
    cuenta = {"mac": 0, "nombre": 0}
    lineas = entrada.read_text(encoding="utf-8", errors="replace").splitlines()
    salida.write_text("\n".join(anonimizar_linea(l, clave, cuenta) for l in lineas) + "\n", encoding="utf-8")
    print(f"{salida}: {len(lineas)} líneas, {cuenta['mac']} MAC y {cuenta['nombre']} nombres seudonimizados")


if __name__ == "__main__":
    main()
