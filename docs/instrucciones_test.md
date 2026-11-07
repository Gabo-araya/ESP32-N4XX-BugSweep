# Instrucciones de test — BugSweep v0.9.0-beta

Este documento contiene los **casos de prueba paso a paso** y una **lista de dispositivos** con los que se puede probar el detector, ordenada por costo y utilidad.

- Cómo **capturar, anotar, seudonimizar y compartir** los resultados: ver [PRUEBAS.md](PRUEBAS.md).
- Significado de lo que muestra la pantalla: ver el [manual de usuario](MANUAL.md).

> Los resultados esperados salen de las reglas reales del firmware (`src/rules/sospecha.cpp` y `src/sniffer/sniffer.h`). Si esas reglas cambian, hay que actualizar este documento.

---

## Índice

1. [Antes de empezar](#1-antes-de-empezar)
2. [Tests sin comprar nada](#2-tests-sin-comprar-nada) (T01 a T13)
3. [Tests con dispositivos](#3-tests-con-dispositivos) (T20 a T28)
4. [Dispositivos para conseguir](#4-dispositivos-para-conseguir)
5. [Hoja de resultados](#5-hoja-de-resultados)

---

## 1. Antes de empezar

### 1.1 Haz un inventario de tus equipos

Antes de cualquier test, anota **tus propios equipos** que usan WiFi o Bluetooth. Sin esta lista no se puede saber si una alerta es real o un falso positivo:

| Equipo | WiFi / BT | Nombre que muestra (si tiene) |
|---|---|---|
| Mi celular | WiFi + BT | |
| Router | WiFi | Nombre de la red: |
| Smart TV / Chromecast | WiFi | |
| Reloj / pulsera | BT | |
| Audífonos | BT | |
| Enchufes, ampolletas, aspiradora | WiFi | |
| … | | |

### 1.2 ⚠️ Importante: tu celular podría estar en 5 GHz

**El ESP32 solo ve WiFi de 2.4 GHz.** Si tu router es de doble banda, tu celular probablemente se conecta en **5 GHz**, y entonces el aparato **no verá su tráfico**. Para los tests de tráfico (T05, T06 y T08), haz que tu celular use 2.4 GHz con alguna de estas opciones:

- Si el router tiene **dos redes separadas** (por ejemplo, `MiCasa` y `MiCasa-5G`), conéctate a la que **no** dice 5G.
- En la configuración del router, **desactiva temporalmente la banda de 5 GHz**.
- Usa el **hotspot de otro celular configurado en 2.4 GHz** (en Android: Ajustes → Zona Wi-Fi → Banda → 2.4 GHz).

Para comprobarlo: en el modo DISPOSITIVOS, tu celular debería aparecer con una red (`Red: MiCasa`) y con tráfico distinto de 0.

### 1.3 Para cada test

1. Lee el test completo antes de empezar.
2. **Inicia la captura** del monitor serie (ver [PRUEBAS.md](PRUEBAS.md#1-capturar)).
3. Anota el **segundo del aparato** (`[  123s]`) en cada acción que hagas.
4. Al terminar, completa la [hoja de resultados](#5-hoja-de-resultados): ✅ pasó, ⚠️ parcial, ❌ falló.

---

## 2. Tests sin comprar nada

### T01 — Arranque

| | |
|---|---|
| **Valida** | Instalación, pantalla, versión |
| **Necesitas** | Solo el aparato |
| **Duración** | 1 min |

**Pasos**
1. Conecta el aparato.
2. Observa la pantalla de inicio.
3. En el monitor serie, busca las primeras líneas.

**Resultado esperado**
- La pantalla muestra "BugSweep", `v0.9.0-beta` arriba a la derecha y las dos opciones (MANTEN / Toque corto).
- El monitor serie muestra `[bugsweep] ESP32-N4XX-BugSweep v0.9.0-beta` y `tabla OUI: 757 prefijos`.

**Pasa si**: todo se ve completo, legible y sin basura en los bordes.

---

### T02 — Botón: corto y largo

| | |
|---|---|
| **Valida** | Antirrebote, distinción corto/largo |
| **Necesitas** | Solo el aparato |
| **Duración** | 2 min |

**Pasos**
1. Desde el inicio, haz un **toque corto**: debe entrar a REDES.
2. Haz **10 toques cortos rápidos**: el marco amarillo debe bajar **exactamente 10 filas** (si hay menos filas, da la vuelta).
3. Mantén apretado sobre una red: debe abrirse el detalle **antes de soltar**.
4. En el detalle, haz un toque corto: debe volver a la lista **sin** abrir nada más.

**Pasa si**: ningún toque se pierde ni se cuenta doble, y una pulsación larga nunca dispara además una corta.

---

### T03 — Revisión guiada en casa (línea base)

| | |
|---|---|
| **Valida** | Revisión guiada completa; **falsos positivos** con tus propios equipos |
| **Necesitas** | Tu inventario (1.1) |
| **Duración** | 2 min |

**Pasos**
1. Ubícate en el centro de tu casa.
2. Desde el inicio, **mantén apretado** BOOT.
3. Observa los 3 pasos y su barra de progreso, sin tocar nada.
4. En el resultado, abre cada hallazgo (pulsación larga) y anota sus motivos.

**Resultado esperado**
- La revisión dura unos 55 s y avanza sola por los 3 pasos.
- Equipos inteligentes con chips Espressif, Tuya o Realtek aparecen como mucho con `?` (BAJA), así que **no** salen en el resultado, que solo muestra MEDIA o más.
- Si no tienes cámaras, el resultado esperado es "OK Nada sospechoso".

**Pasa si**: el resultado coincide con lo que sabes que hay en tu casa. **Anota cualquier alerta `!` o `!!` sobre un equipo que conoces**: es un falso positivo para calibrar.

---

### T04 — Hotspot con nombre de cámara

| | |
|---|---|
| **Valida** | Detección por nombre de red (fase 3) |
| **Necesitas** | Tu celular (hotspot / zona Wi-Fi) |
| **Duración** | 10 min |

**Pasos**
1. En el celular, activa el hotspot en **2.4 GHz** y nómbralo como la primera fila de la tabla.
2. En el aparato, modo REDES: espera 1 o 2 barridos (~8 s) y abre el detalle de esa red.
3. Anota el nivel, el puntaje y los motivos.
4. Repite con cada nombre de la tabla.

| Nombre del hotspot | Nivel esperado | Puntaje base | Motivo esperado |
|---|---|---|---|
| `HDWiFiCam_TEST` | `!!` ALTA | 75 | Nombre de camara espia (HDWiFiCam) |
| `V380-123456` | `!!` ALTA | 65 | Nombre de camara V380 |
| `GW_AP_1234` | `!!` ALTA | 65 | Red de config. camara Yoosee (GW_AP) |
| `IPC-Prueba` | `!` MEDIA | 45 | Nombre tipo camara IP (IPC...) |
| `Mi Camera` | `!` MEDIA | 45 | El nombre contiene 'camera' |
| `SmartLife-AB12` | `!` MEDIA | 30 | Red de config. Tuya/SmartLife |
| `Scam` | `?` BAJA | 25 | El nombre contiene 'cam' (falso positivo **esperado**: muestra el límite de la regla) |
| `Casa Perez` | (nada) | 0 | Sin motivos |

El "puntaje base" supone que el hotspot usa una **MAC aleatoria**, que es lo normal en Android 10 o posterior. Si en cambio usa la MAC real de un fabricante que está en la tabla, el puntaje sube.

**Pasa si**: cada nombre obtiene el nivel esperado.

---

### T05 — Videollamada: perfil de video

| | |
|---|---|
| **Valida** | Medición de tráfico y perfil de video (fase 4) |
| **Necesitas** | Tu celular **en 2.4 GHz** (ver 1.2) y alguien a quien llamar, o una app que transmita video |
| **Duración** | 5 min |

**Pasos**
1. Aparato en modo **DISPOSITIVOS**. Espera 2 ventanas (~10 s).
2. Busca tu celular: normalmente aparece como `aleatoria xx:xx:xx`. Para identificarlo, abre su detalle: debe decir `Red: <tu red>`.
3. **Sin hacer nada en el celular**, anota cuánto sube (columna corta). Debería ser bajo, menos de `5k`.
4. **Inicia una videollamada** con la cámara encendida. Anota el segundo.
5. Espera **30 s** sin mover el aparato.

**Resultado esperado**
- La subida de tu celular pasa de unos pocos cientos de B/s a **decenas de kB/s** (más de `15k`).
- Después de 3 ventanas seguidas (~15 s), aparece el motivo **"Sube datos sin parar (como video)"**: +45 puntos, nivel `!` MEDIA. En el monitor serie aparece `[VIDEO]`.
- Al cortar la llamada, la marca desaparece en la siguiente ventana.

**Pasa si**: aparece `[VIDEO]` durante la videollamada y no antes. **Si no aparece, anota cuánto sube igual**: el ESP32 tiene una sola antena y puede subestimar a los celulares modernos, y ese dato sirve para ajustar el umbral de 15 KB/s.

---

### T06 — Correlación: el pico

| | |
|---|---|
| **Valida** | Rastreo y detección de pico (fase 5) |
| **Necesitas** | Igual que en T05 |
| **Duración** | 5 min |

**Pasos**
1. Con la videollamada activa, abre el detalle de tu celular y haz una **pulsación larga** para entrar al RASTREO.
2. **Apoya el celular con la cámara mirando una pared lisa** y espera hasta que desaparezca "Midiendo base..." (~5 s).
3. Espera 10 s más sin mover nada.
4. **Mueve la mano rápido frente a la cámara** durante 3 s. Anota el segundo.
5. Espera 10 s, quieto.
6. Repite los pasos 4 y 5 tres veces.
7. Variante: en vez de la mano, **apaga y prende la luz**.

**Resultado esperado**
- En reposo: el gráfico se ve relativamente plano y no aparece "PICO".
- Al mover la mano: las barras recientes suben y aparece **`!! PICO: reacciona`**, en magenta.

**Pasa si**: el pico aparece en al menos 2 de los 3 intentos, y **nunca** en reposo.

> Algunas apps de videollamada ajustan la calidad del video y pueden mantener la tasa constante. Si no ves el pico, prueba con otra app: WhatsApp, Meet o Zoom.

---

### T07 — Localizador: acercarse y alejarse

| | |
|---|---|
| **Valida** | Tendencia de la señal (fase 5) |
| **Necesitas** | Tu router |
| **Duración** | 10 min |

**Pasos**
1. Modo REDES → detalle de tu red → **pulsación larga** para entrar al RASTREO.
2. Ponte a **~1 m del router**. Anota los dBm.
3. **Aléjate caminando despacio**, unos 5 m en 10 s. Observa la tendencia.
4. **Quédate quieto** 15 s.
5. **Vuelve caminando** hacia el router.
6. Anota los dBm a 1, 3, 5 y 10 m (y detrás de una pared, si puedes).

**Resultado esperado**
- Al alejarte: aparece principalmente `v MAS LEJOS`.
- Quieto: aparece principalmente `= ESTABLE`.
- Al acercarte: aparece principalmente `^ MAS CERCA`.

**Pasa si**: la tendencia acierta la mayor parte del tiempo **y no cambia de un estado a otro más de una vez cada ~2 s** estando quieto. Si se siente "nerviosa" o "lenta", anótalo: el umbral de ±2.5 dB se puede ajustar.

---

### T08 — Falso positivo: ver video sin transmitir

| | |
|---|---|
| **Valida** | Que **bajar** video no se confunda con **subirlo** |
| **Necesitas** | Tu celular en 2.4 GHz |
| **Duración** | 3 min |

**Pasos**
1. Modo DISPOSITIVOS. En tu celular, pon un video de YouTube en alta calidad.
2. Espera 30 s y abre el detalle de tu celular.

**Resultado esperado**: `Baja` alta (decenas de kB/s o más), `Sube` baja, y **sin** el motivo "Sube datos sin parar".

**Pasa si**: no aparece `[VIDEO]`.

---

### T09 — Entorno real: calle o café

| | |
|---|---|
| **Valida** | Cantidad de falsos positivos en un entorno lleno de equipos |
| **Necesitas** | Aparato alimentado por el celular o una batería |
| **Duración** | 15 min |

**Pasos**
1. Haz una **revisión guiada** en un café o un lugar público.
2. Anota cuántos resultados `!!` y `!` aparecen y qué motivos tienen.
3. Haz lo mismo en 2 o 3 lugares distintos.

**Resultado esperado**: pocas alertas. Las `!` deberían tener motivos plausibles: chips de cámara, routers con nombres raros. Lo normal es **0 o 1 `!!`** por lugar.

**Pasa si**: la cantidad de alertas es manejable para una persona (menos de 5). **Todas las capturas de este test son valiosas para calibrar**, aunque "fallen".

> Haz solo la revisión: no rastrees ni investigues equipos de otras personas.

---

### T10 — Bluetooth: equipos conocidos

| | |
|---|---|
| **Valida** | Escaneo BLE (fase 6) |
| **Necesitas** | Tus equipos Bluetooth (audífonos, reloj, celular) |
| **Duración** | 5 min |

**Pasos**
1. Modo BLUETOOTH. Espera 10 s.
2. Identifica tus equipos: muchos aparecen con su nombre ("Mi Band 7", "Buds2") y otros como "Apple x…", "Samsung x…" o "BLE x…".
3. **Apaga** uno de tus equipos y espera 2 minutos.

**Resultado esperado**
- Tus equipos con nombre aparecen con ese nombre.
- **Ninguno** aparece como rastreador si no lo es.
- El equipo apagado **desaparece** después de ~2 min.

**Pasa si**: no hay rastreadores falsos y la lista se actualiza.

---

### T11 — Bluetooth: localizar un equipo

| | |
|---|---|
| **Valida** | Rastreo BLE (solo localizador) |
| **Necesitas** | Audífonos o reloj Bluetooth |
| **Duración** | 5 min |

**Pasos**
1. **Que otra persona esconda** el equipo encendido en una habitación.
2. Modo BLUETOOTH → detalle del equipo → pulsación larga para entrar al RASTREO.
3. Busca el equipo **guiándote solo por la pantalla**. Mide el tiempo.

**Pasa si**: lo encuentras en menos de 3 minutos (métrica 3 del PRD).

---

### T12 — Revisión manual: luz infrarroja

| | |
|---|---|
| **Valida** | El consejo del manual sobre la cámara frontal |
| **Necesitas** | Un control remoto de TV (tiene un LED infrarrojo) |
| **Duración** | 2 min |

**Pasos**
1. Apunta el control remoto a la **cámara frontal** del celular y aprieta un botón del control.
2. Repite con la **cámara trasera**.

**Resultado esperado**: con la frontal se ve un **destello de luz** en el LED del control. Con la trasera, a menudo no se ve nada.

**Pasa si**: confirmas qué cámara de tu celular ve el infrarrojo. Anótalo en el manual si es la trasera.

---

### T13 — Estabilidad: una hora encendido

| | |
|---|---|
| **Valida** | Que el firmware no se cuelgue ni se reinicie |
| **Necesitas** | Aparato con monitor serie (en el teléfono o el computador) |
| **Duración** | 1 hora |

**Pasos**
1. Deja el aparato en modo DISPOSITIVOS durante 30 min y luego en BLUETOOTH durante 30 min.
2. Revisa el monitor serie.

**Pasa si**: los segundos `[  NNNNs]` **nunca vuelven a 0**, es decir, no hubo reinicios. Si vuelven a 0, guarda las líneas anteriores: probablemente haya un mensaje de error.

---

## 3. Tests con dispositivos

### T20 — ESP32-CAM como cámara de referencia

| | |
|---|---|
| **Valida** | Perfil de video y pico con una cámara **real** |
| **Necesitas** | ESP32-CAM + placa programadora ESP32-CAM-MB ([ver lista](#4-dispositivos-para-conseguir)) |
| **Duración** | 30 min (más la preparación) |

**Preparación**: se programa con el ejemplo `CameraWebServer` de Arduino. Te ayudo a prepararlo cuando lo tengas, con dos variantes:
- **(a) Modo AP** con la red `HDWiFiCam_REF`, como una cámara espía.
- **(b) Conectada a tu red de 2.4 GHz**, como una cámara IP.

**Pasos**
1. Variante (a): verifica en REDES que aparezca `HDWiFiCam_REF` con `!!`. El fabricante debería ser **Espressif**, y el puntaje 75 + 15 = 90.
2. Variante (b): abre el video de la cámara en el navegador del celular, en vivo.
3. En DISPOSITIVOS, la ESP32-CAM debe marcar **"Sube datos sin parar"** en ~15 s.
4. RASTREO de la ESP32-CAM: prueba de la mano frente al lente (igual que T06).
5. Escóndela en una habitación y encuéntrala con el localizador (igual que T11).

**Pasa si**: se cumplen las métricas 1, 2 y 3 del PRD: detectada a 5 m en menos de 60 s, distinguida frente a los demás equipos, y encontrada en menos de 3 min.

---

### T21 — Mini cámara espía WiFi

| | |
|---|---|
| **Valida** | El caso real más común |
| **Necesitas** | Mini cámara WiFi barata tipo "A9" o "X5", o un cargador o reloj con cámara |
| **Duración** | 30 min |

**Pasos**
1. **Antes de configurarla** con su app: enciéndela y revisa REDES. Casi todas crean una red propia (`HDWiFiCam…`, `MV…`, `LookCam…`, `IPC…`). **Anota el nombre exacto** y el fabricante que muestra el detalle.
2. Configúrala con su app para que se conecte a tu red de 2.4 GHz.
3. Abre la transmisión en vivo en la app.
4. Haz los pasos 3 a 5 de T20.
5. Haz una **revisión guiada** con la cámara escondida.

**Pasa si**: la revisión guiada la muestra en el resultado con `!` o `!!`.

> **Si el nombre de su red no está en la lista de patrones, ese es el dato más valioso**: con él se agrega una regla nueva.

---

### T22 — Cámara IP doméstica barata

| | |
|---|---|
| **Valida** | Detección por fabricante o chip (OUI) y por tráfico |
| **Necesitas** | Cámara Tuya, V380, Imou, Ezviz o similar |
| **Duración** | 30 min |

**Pasos**: los mismos de T21. Además, anota el **fabricante** que aparece en el detalle.

**Resultado esperado**: si es Ezviz o Imou (marcas de Hikvision y Dahua), "Fabricante de camaras", +60 puntos. Si es Tuya o V380, probablemente un chip Beken, Fn-Link, AltoBeam o Tuya, entre +15 y +35 puntos.

**Pasa si**: con el video en vivo, llega a `!` o más.

---

### T23 — Enchufe inteligente (control de falsos positivos)

| | |
|---|---|
| **Valida** | Que un equipo IoT inocente **no** se marque como cámara |
| **Necesitas** | Enchufe inteligente Tuya / Smart Life |
| **Duración** | 15 min |

**Pasos**
1. Ponlo en **modo configuración** (generalmente, manteniendo su botón apretado). En REDES debería aparecer una red `SmartLife-XXXX`: se espera `!` MEDIA, con el motivo "Red de config. Tuya/SmartLife".
2. Configúralo con su app y déjalo funcionando normalmente.
3. En DISPOSITIVOS, anota su nivel.

**Resultado esperado**: una vez configurado, como mucho `?` BAJA (chip IoT), **sin** "Sube datos sin parar".

**Pasa si**: en funcionamiento normal no supera `?`.

---

### T24 — Rastreador Apple Find My (AirTag o compatible)

| | |
|---|---|
| **Valida** | Detección de Find My y de los modos "dueño cerca" y "separado" |
| **Necesitas** | AirTag o un rastreador "compatible con Find My", **y un iPhone** para activarlo (tuyo o prestado) |
| **Duración** | Varias horas (el modo separado tarda en activarse) |

**Pasos**
1. Activa el rastreador con el iPhone.
2. **Con el iPhone cerca**: en BLUETOOTH debería aparecer como `FindMy (dueno)`, `?` BAJA, 25 puntos.
3. **Separa el rastreador del iPhone**: deja el iPhone en otra casa, o apágalo. Revisa cada 15 minutos.
4. Cuando cambie de modo, debería aparecer como **`AirTag/FindMy`**, `!!` ALTA, 70 puntos.
5. Llévalo contigo **10 minutos o más**: se agrega "Te acompana hace 10+ minutos", +15 puntos.
6. Escóndelo y búscalo con el RASTREO.

**Pasa si**: se cumplen los dos modos y el localizador lo encuentra.

> Apple cambia el modo "separado" después de un tiempo variable, desde minutos hasta horas. **Anota cuánto tardó**: es un dato útil.

---

### T25 — Tile

| | |
|---|---|
| **Valida** | Detección de Tile (UUID 0xFEED) |
| **Necesitas** | Tile Mate, Slim o Pro (funciona con Android y con iPhone) |

**Pasos**: actívalo con la app de Tile. Revisa BLUETOOTH y el detalle.

**Resultado esperado**: `Tile`, `!!` ALTA (60 puntos; 75 después de 10 minutos).

---

### T26 — Samsung SmartTag

| | |
|---|---|
| **Valida** | Detección de SmartTag (UUID 0xFD5A) |
| **Necesitas** | Galaxy SmartTag2 **y un teléfono Samsung Galaxy** |

**Resultado esperado**: `SmartTag`, `!!` ALTA (60 puntos).

---

### T27 — Rastreador de la red Google Find My Device

| | |
|---|---|
| **Valida** | La firma de Google FMDN, que **está sin verificar** |
| **Necesitas** | Un rastreador compatible con la red de Google: Chipolo POINT, Pebblebee (versión para Android) o Motorola Moto Tag. Requiere Android |

**Pasos**: actívalo y sepáralo de su teléfono, como en T24.

**Resultado esperado**: `Google FMDN`, `!` MEDIA (50 puntos).

**Este es el test más valioso de BLE**: la regla de Google se escribió sin poder probarla. **Si no se detecta, guarda la captura completa sin seudonimizar** (es tu propio equipo): con ella se corrige la regla.

---

### T28 — Otros equipos con cámara (opcional)

Si tienes acceso a ellos, suman datos: monitor de bebé WiFi, timbre con cámara, robot aspiradora con cámara, cámara de un auto (*dashcam*) con WiFi, dron de juguete con FPV por WiFi. Para cada uno, haz los pasos 1 y 3 de T21 y anota el resultado.

---

## 4. Dispositivos para conseguir

Precios **referenciales en USD** (2026, AliExpress o tiendas locales); pueden variar bastante. Ordenados por **valor para el proyecto respecto a su costo**:

| Prioridad | Dispositivo | Precio aprox. | Tests | Requisitos | Por qué |
|---|---|---|---|---|---|
| ⭐ 1 | **ESP32-CAM + ESP32-CAM-MB** (programadora USB) | 7 – 12 | T20 | Ninguno | Cámara de referencia **controlable**: puedes fijar su nombre, su modo y su calidad de video. Además sirve para aprender más ESP32. Compra el pack que trae la placa MB |
| ⭐ 2 | **Mini cámara espía WiFi** ("A9", "X5", "HD WiFi Camera") | 8 – 15 | T21 | Su app (generalmente HDWiFiCam Pro, LookCam o V380) | **El caso real que el proyecto quiere detectar.** Úsala solo en tu espacio y sin grabar a otras personas |
| ⭐ 3 | **Enchufe inteligente Tuya / Smart Life** | 5 – 8 | T23 | App Smart Life | Control barato de **falsos positivos**. Además queda útil en casa |
| 4 | **Rastreador "compatible con Find My"** (genérico) | 6 – 12 | T24 | **iPhone** para activarlo | Mucho más barato que un AirTag y usa el mismo protocolo |
| 4 | **Rastreador de la red de Google** (Chipolo POINT, Pebblebee, Moto Tag) | 20 – 30 | T27 | **Android** | Valida la regla de Google, que hoy está **sin verificar** |
| 5 | **Cámara IP doméstica** (Tuya, V380, Imou, Ezviz) | 15 – 30 | T22 | Su app | Detección por fabricante. Una Imou o Ezviz valida la regla "Fabricante de camaras" |
| 6 | **Tile Mate** | 20 – 25 | T25 | Android o iPhone | Valida Tile |
| 6 | **AirTag** original | 25 – 30 | T24 | iPhone | El genérico (prioridad 4) valida lo mismo por menos |
| 7 | **Galaxy SmartTag2** | 25 – 30 | T26 | Celular Samsung Galaxy | Solo si tienes un Galaxy |

### Recomendación según presupuesto

| Presupuesto | Compra | Cubre |
|---|---|---|
| **~USD 15** | ESP32-CAM con MB + enchufe Tuya | Cámara de referencia y falsos positivos: valida las **fases 3, 4 y 5** |
| **~USD 30** | Lo anterior + mini cámara espía | El caso real completo de cámaras |
| **~USD 55** | Lo anterior + el rastreador de **tu ecosistema**: Google si tienes Android, Find My genérico si tienes iPhone | **Todas las fases** |

> **Pídelos prestados** antes de comprar: es común que alguien tenga un AirTag, un Tile o una cámara Tuya. Los tests T24 a T27 no requieren que el rastreador sea tuyo, solo que el dueño esté de acuerdo.

### Antes de comprar una mini cámara

- Comprar y tener una cámara así es legal en la mayoría de los países. **Grabar a otras personas sin su consentimiento, en espacios privados, no lo es.**
- Úsala solo para probar el detector, en tu espacio, apuntando a objetos o a ti.
- Sus apps suelen pedir muchos permisos y mandar datos a servidores en el extranjero. Usa una cuenta de correo desechable y **no la conectes a tu red principal**: usa el hotspot de un celular viejo o una red de invitados.

---

## 5. Hoja de resultados

Copia esta tabla en las notas de cada captura, o complétala aquí.

| Test | Fecha | Resultado | Captura (`capturas/…`) | Observaciones / datos para calibrar |
|---|---|---|---|---|
| T01 Arranque | | | | |
| T02 Botón | | | | |
| T03 Revisión en casa | | | | Falsos positivos: |
| T04 Hotspot nombres | | | | |
| T05 Videollamada | | | | Subida medida (kB/s): |
| T06 Pico | | | | Aciertos __ / 3 |
| T07 Localizador | | | | dBm a 1 / 3 / 5 / 10 m: |
| T08 YouTube | | | | |
| T09 Calle / café | | | | Alertas por lugar: |
| T10 BLE conocidos | | | | |
| T11 BLE localizar | | | | Tiempo: |
| T12 Infrarrojo | | | | Cámara que ve IR: |
| T13 Estabilidad | | | | ¿Reinicios? |
| T20 ESP32-CAM | | | | |
| T21 Mini cámara | | | | Nombre de su red: |
| T22 Cámara IP | | | | Fabricante: |
| T23 Enchufe Tuya | | | | |
| T24 Find My | | | | Tiempo hasta "separado": |
| T25 Tile | | | | |
| T26 SmartTag | | | | |
| T27 Google FMDN | | | | |

Resultado: ✅ pasó · ⚠️ parcial · ❌ falló · — no aplica
