# Manual de usuario — BugSweep

Este manual explica cómo **usar** el detector, paso a paso y sin conocimientos técnicos. Para compilar e instalar el programa, ver el [README](../README.md#compilar-e-instalar).

> Versión del manual: 2026-11-07. Corresponde al firmware con revisión guiada (fase 7).

---

## Índice

1. [Qué es y qué no es](#1-qué-es-y-qué-no-es)
2. [Conocer el aparato](#2-conocer-el-aparato)
3. [Encenderlo](#3-encenderlo)
4. [Revisión guiada (recomendada)](#4-revisión-guiada-recomendada)
5. [Entender los resultados](#5-entender-los-resultados)
6. [Modo experto](#6-modo-experto)
7. [Rastreo: encontrar el objeto](#7-rastreo-encontrar-el-objeto)
8. [Si encuentras algo](#8-si-encuentras-algo)
9. [Revisión manual complementaria](#9-revisión-manual-complementaria)
10. [Ver más detalles en el teléfono o el computador](#10-ver-más-detalles-en-el-teléfono-o-el-computador)
11. [Problemas y soluciones](#11-problemas-y-soluciones)
12. [Privacidad y uso responsable](#12-privacidad-y-uso-responsable)
13. [Glosario](#13-glosario)

---

## 1. Qué es y qué no es

BugSweep es un **detector de bolsillo** que escucha las señales WiFi y Bluetooth cercanas y te avisa si alguna se parece a la de una **cámara oculta**, un **micrófono que transmite** o un **rastreador** (como un AirTag).

**Solo escucha.** No transmite nada, no se conecta a ninguna red y no puede ver el contenido de lo que otros envían.

### Lo que puede encontrar

- Cámaras WiFi que crean su propia red (muy común en las cámaras espía baratas).
- Cámaras y otros equipos conectados al WiFi del lugar que **envían video sin parar**.
- Rastreadores Bluetooth: AirTag y otros de la red Find My de Apple, Tile, Samsung SmartTag y, posiblemente, los de la red de Google.

### Lo que NO puede encontrar

- Equipos que usan **WiFi de 5 GHz**.
- Cámaras que **solo graban** en una tarjeta de memoria, sin transmitir.
- Micrófonos de **radio analógica** (433 MHz, 900 MHz, etc.) o que usan la **red celular** (con chip SIM).
- Equipos conectados **por cable**.
- Equipos que están **apagados** o sin transmitir en ese momento.

> ⚠️ **Un resultado limpio no garantiza que no haya nada.** Completa siempre la revisión con la [revisión manual](#9-revisión-manual-complementaria).

---

## 2. Conocer el aparato

Con el conector USB hacia abajo:

```
┌───────────────────────────┐
│                           │
│         PANTALLA          │
│                           │
│                           │
├───────────────────────────┤
│  [EN]    [ USB-C ]  [BOOT]│
└───────────────────────────┘
```

| Parte | Para qué sirve |
|---|---|
| **BOOT** (derecha) | **El único botón que usarás.** Maneja todo el aparato |
| **EN** (izquierda) | Reinicia el aparato. Úsalo solo si se queda pegado |
| **USB-C** | Alimentación y conexión al teléfono o al computador |

### El botón BOOT

Hay dos formas de apretarlo:

| Gesto | Cómo | Para qué sirve en general |
|---|---|---|
| **Toque corto** | Apretar y soltar rápido (menos de medio segundo) | **Avanzar**: bajar en una lista, volver atrás |
| **Pulsación larga** | Mantener apretado hasta que la pantalla cambie (más de medio segundo) | **Elegir**: abrir, iniciar, confirmar |

No hace falta soltar el botón en la pulsación larga: la acción ocurre apenas se cumple el tiempo.

---

## 3. Encenderlo

El aparato no tiene batería: se enciende apenas recibe energía por el USB-C. Puedes alimentarlo de tres formas:

| Fuente | Cómo |
|---|---|
| **Batería externa** (*power bank*) | Cualquier cable USB a USB-C |
| **Teléfono Android** | Cable USB-C a USB-C. Si no enciende, usa un **adaptador OTG** (USB-C macho a USB-A hembra) con un cable USB-A a USB-C. En algunos teléfonos hay que activar "OTG" en Ajustes |
| **iPhone 15 o posterior** | Cable USB-C a USB-C |
| **Computador** | Cualquier puerto USB |

Consume poco: una hora de uso le resta al teléfono un 5 a 10 % de batería.

Al encender, aparece la **pantalla de inicio**:

```
BugSweep
Detector pasivo de camaras,
microfonos y rastreadores.
┌──────────────────────────────────┐
│ MANTEN apretado: revision guiada │
│ (recomendado, ~1 minuto)         │
└──────────────────────────────────┘
Toque corto: modo experto
```

---

## 4. Revisión guiada (recomendada)

Es la forma más fácil de revisar un lugar. Dura alrededor de **un minuto**.

### Antes de empezar

- **Ubícate en el centro** del lugar que quieres revisar.
- Si es posible, **apaga el WiFi y el Bluetooth de tus propios equipos**, o tenlos en cuenta: también van a aparecer.

### Paso a paso

1. En la pantalla de inicio, **mantén apretado BOOT**.
2. El aparato recorre **3 pasos automáticos**. Cada uno muestra qué está haciendo, una instrucción y una barra de progreso:

   | Paso | Duración | Qué hace | Qué haces tú |
   |---|---|---|---|
   | 1. Redes WiFi | 12 s | Busca redes creadas por cámaras | Quédate en el centro |
   | 2. Dispositivos WiFi | 27 s | Mide cuánto transmite cada equipo | **Quédate quieto** |
   | 3. Bluetooth | 15 s | Busca rastreadores | Quédate cerca |

3. Al final aparece el **resultado**.

Para **cancelar** en cualquier momento, toca el botón: vuelves al inicio.

### El resultado

**Si no encontró nada sospechoso**:

```
OK  Nada sospechoso
No garantiza que no haya nada:
- camaras sin WiFi, en 5 GHz o solo SD
- microfonos de radio analogica/GSM
Busca lentes con la linterna del celular.
```

Haz igual la [revisión manual](#9-revisión-manual-complementaria). Toca el botón para volver al inicio.

**Si encontró algo**, aparece una lista:

```
RESULTADO  Sospechosos: 2
 > Terminar (volver al INICIO)
!! ▂▄▆  -58  WiFi  HDWiFiCam_8F2A
!  ▂▄   -71  Disp  Bilian 4A:52:51
```

- **Toque corto**: mueve el marco amarillo a la siguiente fila.
- **Pulsación larga sobre un hallazgo**: abre su **detalle**, con los motivos de la sospecha. Desde el detalle, otra pulsación larga inicia el [rastreo](#7-rastreo-encontrar-el-objeto) para ubicarlo.
- **Pulsación larga sobre "Terminar"**: vuelve al inicio.

La columna `WiFi`, `Disp` o `BLE` indica qué paso lo encontró.

---

## 5. Entender los resultados

### Niveles de sospecha

Cada elemento tiene un puntaje de 0 a 100 que se muestra con un **símbolo**, además de un color:

| Símbolo | Nivel | Puntaje | Color | Qué significa |
|---|---|---|---|---|
| `!!` | **ALTA** | 60 a 100 | Magenta | Varias pistas apuntan a una cámara o un rastreador. **Investígalo** |
| `!` | **MEDIA** | 30 a 59 | Amarillo | Hay una pista importante. **Revísalo** |
| `?` | **BAJA** | 15 a 29 | Cian | Una pista débil. Suele ser un equipo inteligente común (enchufe, ampolleta, aspiradora) |
| *(nada)* | Ninguna | 0 a 14 | Blanco | Sin motivos de sospecha |

### Cómo leer una fila

```
!!  ▂▄▆█  -48  c6   HDWiFiCam_8F2A
│    │     │    │    └─ nombre
│    │     │    └────── dato corto (depende del modo)
│    │     └─────────── señal en dBm
│    └───────────────── barras de señal
└────────────────────── nivel de sospecha
```

**La señal (dBm)** se lee como un número negativo: **mientras más cerca de 0, más cerca está el equipo**.

| Señal | Distancia aproximada |
|---|---|
| −30 a −50 | Muy cerca: en la misma habitación, a pocos metros |
| −50 a −70 | Cerca: la misma casa o la habitación de al lado |
| −70 a −85 | Lejos: otro piso o un vecino |
| −85 a −100 | Muy lejos, en el límite de alcance |

Las paredes, los muebles y el cuerpo humano debilitan la señal, así que son solo referencias.

### Los motivos de sospecha

El **detalle** de cada elemento explica por qué es sospechoso:

| Motivo en pantalla | Qué significa | ¿Preocupa? |
|---|---|---|
| `Nombre de camara espia (HDWiFiCam)` / `(LookCam)` / `Nombre de camara espia` | La red tiene el nombre que usan las cámaras espía más vendidas | **Mucho** |
| `Nombre de camara V380`, `Nombre de camara WiFi`, `Nombre de mini camara`, `Nombre de camara IP` | El nombre de la red corresponde a una marca o un tipo de cámara | **Sí** |
| `Red de config. camara Yoosee (GW_AP)` | Una cámara Yoosee en modo de configuración | **Sí** |
| `Nombre tipo camara IP (IPC...)` | El nombre empieza como el de muchas cámaras IP | Bastante |
| `El nombre contiene 'camera'` / `'cam'` | Puede ser una cámara, o solo un nombre que lo contiene | Depende |
| `Red de config. Tuya/SmartLife` | Un equipo inteligente esperando ser configurado. Puede ser una cámara o un enchufe | Algo |
| `Red oculta` | La red no muestra su nombre. Es común en routers, pero también lo hacen algunas cámaras para pasar desapercibidas | Algo |
| `Fabricante de camaras` | El fabricante registrado se dedica a cámaras (Hikvision, Dahua, Ezviz, Reolink, Wyze…) | **Sí** |
| `Chip comun en camaras baratas` | El chip WiFi es muy usado en cámaras económicas, pero también en otros aparatos | Algo |
| `Chip IoT (camaras, enchufes...)` | Chip genérico de "cosas inteligentes" (Espressif, Tuya, Realtek) | Poco |
| `MAC aleatoria: fabricante oculto` | El equipo esconde su identidad. Es normal en celulares y notebooks modernos; las cámaras baratas casi nunca lo hacen | Poco |
| `Sube datos sin parar (como video)` | Envió más de 15 KB/s durante unos 15 segundos seguidos, como una cámara transmitiendo | **Mucho**, si no es tuyo |
| `Rastreador Find My lejos de su dueno` | Un AirTag (u otro accesorio Find My) separado de su dueño. **Si te acompaña, alguien podría estar siguiéndote** | **Mucho** |
| `Accesorio Find My (dueno cerca)` | Un AirTag u otro accesorio Find My con su dueño cerca. Normalmente es de alguien que está contigo o cerca | Poco |
| `Rastreador Tile` / `Rastreador Samsung SmartTag` | Un rastreador de esas marcas | **Sí**, si no es tuyo |
| `Posible rastreador red Google` | Señal compatible con los rastreadores de la red de Google (aún sin verificar) | Algo |
| `Te acompana hace 10+ minutos` | Un rastreador que sigue cerca después de 10 minutos | **Mucho**, si te has movido |

> 💡 **Lo más importante es la combinación de pistas.** Por ejemplo: un fabricante de cámaras **más** la subida continua de datos es mucho más grave que cualquiera de las dos por separado.

---

## 6. Modo experto

Desde la pantalla de inicio, haz un **toque corto**. El aparato muestra listas que se actualizan solas.

### Cambiar de modo

La **primera fila** de cada lista es una opción para cambiar de modo. Pon el marco amarillo sobre ella y haz una **pulsación larga**:

**REDES** → **DISPOSITIVOS** → **BLUETOOTH** → vuelve al **INICIO**

| Modo | Qué muestra | Se actualiza cada | Dato corto |
|---|---|---|---|
| **REDES** | Las redes WiFi, incluidas las que crean las cámaras | ~4 s | Canal: `c6` |
| **DISPOSITIVOS** | Los equipos conectados a cualquier red, y cuánto envían | ~5 s | Lo que **sube** por segundo: `850`, `12k`, `1.2M` |
| **BLUETOOTH** | Los equipos Bluetooth y los rastreadores | ~2 s | Tiempo que lleva visible: `<1m`, `12m` |

La barra superior muestra el modo, la cantidad de elementos y cuántos son sospechosos (`Sosp:`).

### Moverse por la lista

- **Toque corto**: baja el marco amarillo. Al llegar al final, vuelve al principio. Si hay más de 7 filas, la pantalla pasa a la página siguiente.
- **Pulsación larga**: abre el **detalle** de la fila marcada.

La lista está ordenada con **lo más sospechoso arriba**. El marco sigue al mismo equipo aunque la lista se reordene.

### El detalle

Muestra el nombre, el nivel y el puntaje, el fabricante, la dirección (MAC), la señal y **los motivos de la sospecha**. Según el modo, también aparecen:

- **DISPOSITIVOS**: a qué red está conectado, cuánto sube y cuánto baja por segundo.
- **BLUETOOTH**: si su dirección es pública o aleatoria, la empresa, y cuánto tiempo lleva visible.

| Gesto | Acción |
|---|---|
| Toque corto | Volver a la lista |
| Pulsación larga | Iniciar el **rastreo** |

---

## 7. Rastreo: encontrar el objeto

El rastreo hace que el aparato **escuche solo al equipo elegido**, todo el tiempo, para ayudarte a encontrarlo y confirmar si es una cámara.

```
┌──────────────────────────────────────────────────┐
│ RASTREO c5                    Bilian 4A:52:51    │
│                    ┌──────────────────────────┐  │
│   -58              │ max 48kB/s               │  │
│                    │            ▂▂▃▂▂▃▇█▇█   │  │
│ dBm (senal)        │▂▂▃▂▂▂▃▂▂▃▂▂▂             │  │
│ ^ MAS CERCA        └──────────────────────────┘  │
│ ████████░░░░       Ahora 42kB/s  base 9kB/s      │
│                    !! PICO: reacciona            │
│ Pulsa para salir                                 │
└──────────────────────────────────────────────────┘
```

### Localizar (lado izquierdo)

1. Camina **despacio** por el lugar con el aparato frente a ti.
2. Mira la tendencia:
   - `^ MAS CERCA`: vas bien, sigue en esa dirección.
   - `v MAS LEJOS`: date vuelta.
   - `= ESTABLE`: muévete un poco hacia otro lado para ver cómo cambia.
   - `SIN SENAL`: el equipo no ha transmitido en los últimos 3 segundos. Puede estar lejos o haberse apagado.
3. El objeto está donde **el número es más alto (más cerca de 0)** y la barra está más llena. A menos de un metro suele marcar entre −30 y −45.
4. **Revisa a mano esa zona**: enchufes, cargadores, detectores de humo, relojes, cuadros, peluches, adornos, parlantes, routers.

> 💡 Tu cuerpo bloquea la señal. Si la señal baja de golpe al girar, el objeto está **detrás de ti**.

### Confirmar si es una cámara (lado derecho, solo WiFi)

Las cámaras envían más datos cuando la imagen cambia. Aprovecha eso:

1. Espera unos **5 segundos** a que desaparezca "Midiendo base...".
2. **Mueve la mano rápido** frente al lugar sospechoso durante 3 segundos, o **apaga y prende la luz**.
3. Mira el gráfico:
   - **Si sube y aparece `!! PICO: reacciona`**: el equipo reacciona a lo que pasa frente a él. Es **muy probablemente una cámara** que está mirando esa zona.
   - **Si no cambia**: probablemente no es una cámara, o no está apuntando a donde moviste la mano. Prueba en otras zonas.

En los rastreadores **Bluetooth** no hay gráfico, solo el localizador: un rastreador no envía video que se pueda medir.

**Para salir**: cualquier pulsación. Vuelves al detalle y el modo anterior se reanuda.

---

## 8. Si encuentras algo

1. **Mantén la calma y no te pongas en riesgo.** No enfrentes a nadie.
2. **Documenta**: saca fotos del objeto sin moverlo y de la pantalla del aparato (nivel, motivos y señal). Anota la fecha, la hora y el lugar.
3. **No lo desarmes ni lo destruyas**: puede ser evidencia.
4. **Según el lugar**:
   - **Alojamiento de arriendo temporal**: avisa a la plataforma de arriendo (la mayoría prohíbe las cámaras en espacios privados) y busca otro lugar.
   - **Hotel**: avisa en la recepción y pide otra habitación.
   - **Trabajo**: consulta con recursos humanos o con un abogado laboral.
   - **Rastreador que te sigue**: en iPhone, revisa la alerta "AirTag encontrado contigo". En Android, la app "Detector de rastreadores" o la función equivalente del sistema. **Si temes por tu seguridad, contacta a las autoridades.**
5. **Denuncia** a las autoridades si corresponde. La grabación oculta en espacios privados es un delito en muchos países.

---

## 9. Revisión manual complementaria

El aparato no ve todo. Completa la revisión así:

- 🔦 **Reflejo del lente**: apaga la luz y recorre la habitación con la **linterna del celular** a la altura de los ojos. Un lente devuelve un **brillo pequeño**, azulado o morado, que no cambia al moverte.
- 📷 **Luces infrarrojas**: muchas cámaras con visión nocturna tienen LEDs infrarrojos que el ojo no ve. Con la luz apagada, mira la habitación a través de la **cámara frontal del celular**: los LEDs infrarrojos se ven como **puntos de luz morada o blanca**. La cámara trasera suele filtrar el infrarrojo; la frontal, muchas veces no.
- 🔌 **Objetos típicos**: cargadores USB, detectores de humo, relojes despertadores, adornos, peluches, enchufes múltiples, parlantes, ganchos de ropa, marcos de fotos, routers, cajas de TV.
- 🕳️ **Agujeros pequeños** en lugares raros: paredes, cielo, muebles o adornos apuntando a la cama, la ducha o el escritorio.
- 🌐 **Si tienes acceso al router**, revisa la lista de equipos conectados en su página de administración.

---

## 10. Ver más detalles en el teléfono o el computador

Además de la pantalla, el aparato envía un informe detallado por el cable USB: las direcciones completas, los puntajes y los motivos de **todo** lo que ve.

- **Teléfono Android**: instala la app gratuita **Serial USB Terminal** (de Kai Morich), conecta el aparato y configúralo a **115200** baudios.
- **Computador**: el monitor serie de VS Code con PlatformIO (🔌), o cualquier programa de terminal serie, a **115200** baudios.

Ejemplo:

```
[redes] barrido #3: 12 redes, 1 sospechosas
  !!  90   -48 dBm  c6   AA:BB:CC:DD:EE:FF  Bilian       HDWiFiCam_8F2A
```

Fotografiar o copiar estas líneas es la mejor forma de pedir ayuda para interpretar un hallazgo.

---

## 11. Problemas y soluciones

| Problema | Qué hacer |
|---|---|
| La pantalla no enciende | Revisa el cable (algunos solo cargan) y la fuente de energía. Con el teléfono, prueba el adaptador OTG o activa "OTG" en Ajustes |
| Se apaga al rato con el teléfono | Algunos teléfonos desactivan el OTG después de unos minutos. Vuelve a activarlo o usa una batería externa |
| El aparato no responde | Aprieta **EN** para reiniciarlo |
| No aparece nada en DISPOSITIVOS | Espera al menos 10 segundos: la primera lista se arma tras unos 5. Si no hay redes WiFi cerca, no habrá dispositivos |
| Aparecen muchísimos equipos | Es normal en edificios y en la calle. Fíjate solo en lo marcado con `!!` y `!` |
| Mi propio celular aparece como sospechoso | Durante una videollamada, el celular sube video y puede marcarse con "Sube datos sin parar". Es esperable |
| Los textos no tienen tildes | Es una limitación de las letras incluidas en la pantalla |

Para problemas al instalar el programa, ver [Problemas frecuentes en el README](../README.md#problemas-frecuentes).

---

## 12. Privacidad y uso responsable

- El aparato **no guarda nada**. Todo se borra al desconectarlo.
- **No transmite nada.** Cualquiera puede comprobarlo (ver el [README](../README.md#verificar-que-es-solo-receptor)).
- Úsalo para **proteger tu privacidad** en lugares que ocupas legítimamente: tu casa, tu habitación de hotel, tu arriendo, tu auto.
- **No lo uses para vigilar a otras personas.** Las direcciones que ves pertenecen a equipos de terceros: no las publiques.
- Las leyes cambian según el país. Infórmate antes de usarlo.

---

## 13. Glosario

| Término | Significado |
|---|---|
| **BLE** | *Bluetooth Low Energy*: Bluetooth de bajo consumo. Lo usan relojes, audífonos y rastreadores |
| **Beacon** | Anuncio que un router o una cámara emite unas 10 veces por segundo para decir "aquí estoy" |
| **Canal** | "Carril" del WiFi de 2.4 GHz, del 1 al 13. Cada red usa uno |
| **dBm** | Unidad de la intensidad de la señal. Siempre negativa: −40 es fuerte, −90 es débil |
| **Dirección MAC** | Identificador de cada equipo de red, como `AA:BB:CC:DD:EE:FF` |
| **MAC aleatoria** | Dirección inventada que cambia cada cierto tiempo, para proteger la privacidad. Es típica de celulares |
| **Modo promiscuo** | Modo en que el aparato escucha **todo** el tráfico WiFi del aire, no solo el dirigido a él |
| **OUI** | Los primeros 3 bytes de una MAC. Identifican al fabricante |
| **OTG** | Función del teléfono que le permite alimentar accesorios por USB |
| **Red oculta** | Red WiFi que no muestra su nombre |
| **Rastreador** | Pequeño objeto (AirTag, Tile, SmartTag) que informa su ubicación a través de los teléfonos cercanos |
| **SSID** | El nombre de una red WiFi |
