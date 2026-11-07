# Pruebas de terreno

Cómo capturar lo que ve el aparato y compartirlo **sin exponer datos de terceros**, para comprobar que la detección funciona y calibrar los umbrales.

## 1. Capturar

### Con un teléfono Android (en terreno)

1. Instala **Serial USB Terminal** (Kai Morich, gratis).
2. Conecta el aparato al teléfono (cable USB-C a USB-C, o adaptador OTG).
3. En la app: **Settings → Serial**: velocidad **115200**. Acepta el permiso USB y toca el ícono de conectar.
4. Usa el aparato normalmente. Todo lo que ve aparece en la app, con los segundos desde el encendido: `[  123s]`.
5. Al terminar, guarda o comparte la sesión desde el menú de la app (opción de guardar o compartir los datos). Llévate el archivo de texto al computador (por cable, Drive, o enviándotelo a ti mismo).

### Con el computador

```bash
pio device monitor | tee capturas/casa-$(date +%F-%H%M).txt
```

O en VS Code: Monitor (🔌), y luego copia y pega el texto en un archivo.

## 2. Anotar lo que hiciste

Los datos solos no bastan: necesito saber **qué era real**. Junto a cada captura, crea un archivo `.md` con el mismo nombre:

```markdown
# Captura: calle-2026-11-08

- Lugar (general, sin dirección exacta): calle comercial / mi casa / café
- Duración: ~15 min
- Mis propios equipos: mi celular (Android), mi router "MiCasa", mi reloj Mi Band

## Qué hice (con el segundo del aparato)
- [  0s] encendí, revisión guiada
- [ 70s] entré a DISPOSITIVOS
- [180s] empecé una videollamada con mi celular
- [240s] rastreo de mi celular, moví la cámara de la pared a mi cara

## Qué esperaba vs qué vi
- Esperaba: mi celular con "Sube datos sin parar" durante la videollamada
- Vi: …
- Dudas o cosas raras: …
```

## 3. Seudonimizar

Las capturas tienen direcciones MAC y nombres de redes y celulares **de otras personas**. Antes de compartirlas:

```bash
python3 -I tools/anonimizar.py capturas/calle-2026-11-08.txt
# -> capturas/calle-2026-11-08.anon.txt
```

- Las MAC conservan el fabricante (OUI) y el resto se reemplaza por un código estable: `A4:CF:12:x5e0c1d`.
- Los nombres de redes y dispositivos BLE **no sospechosos** se reemplazan por `red-…` o `nombre-…`. Los sospechosos se conservan, porque son los que hay que evaluar.
- La "sal" secreta queda en `capturas/.sal`. No la compartas.

**Revisa el `.anon.txt` antes de compartirlo**: si queda algo personal (por ejemplo, en el nombre de una red sospechosa o en tus notas), bórralo a mano.

> `capturas/` está en `.gitignore`: **nunca** se sube a GitHub.

## 4. Compartir para análisis

Comparte **el `.anon.txt` y tus notas `.md`**, nunca la captura original. Si trabajas con Claude Code en este repositorio, basta con decir: *"revisa `capturas/calle-2026-11-08.anon.txt` y sus notas"*.

## 5. Qué pruebas hacer

Los casos de prueba paso a paso (T01 a T28), con sus resultados esperados y la lista de dispositivos para conseguir, están en **[instrucciones_test.md](instrucciones_test.md)**.

## 6. Registro de resultados

La hoja de resultados está en [instrucciones_test.md](instrucciones_test.md#5-hoja-de-resultados). Aquí se anotan los **ajustes** que surjan de ella:

| Fecha | Test | Hallazgo | Ajuste en el firmware |
|---|---|---|---|
| | | | |
