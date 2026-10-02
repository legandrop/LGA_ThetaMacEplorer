<p align="right"><a href="README.md">English</a> · <b>Español</b></p>

# ThetaMacExplorer

Explorá y descargá las fotos y videos de una cámara **RICOH THETA** desde tu Mac, por USB.
Pensada para trabajar con HDRIs: cada secuencia HDR aparece como **un solo ítem**, con la
vista previa de la exposición del medio, y se ve de un vistazo qué sets ya descargaste.

![ThetaMacExplorer con una RICOH THETA Z1 conectada: cada set HDR en un solo tile, los badges Saved y Partial y la vista previa del set seleccionado](doc/images/thetamacexplorer_main.jpg)

## Qué hace

- **Cada set HDR es un ítem.** Una secuencia de exposiciones (JPG o DNG) se agrupa en un
  solo tile con su cantidad de exposiciones, y se descarga o se borra entera.
- **Vista previa y datos** del ítem seleccionado: nombre, tipo, tamaño, fecha, resolución y
  ruta en la cámara.
- **Qué ya tenés.** Los ítems que están completos en tu carpeta de descarga se marcan
  *Saved*; los que tienen solo parte de sus archivos, *Partial*.
- **Lo reciente resalta.** La fecha de una captura arranca en verde claro (ámbar en DNG) y
  se apaga a lo largo de 28 horas, así una sesión recién hecha se encuentra enseguida.
- **Una carpeta por ítem.** Cada ítem se descarga en su propia carpeta, con la fecha de
  captura y sus archivos, por ejemplo `HDRI_260402_R21381-21389_dng` para un set HDR.
- **Volver a descargar sin pisar.** Si algunos archivos ya están en disco, podés saltearlos
  y bajar solo lo que falta (así se completa un set a medio bajar) o reemplazar la copia
  local.
- **Borrar de la cámara** con una confirmación que dice cuántos archivos se van. Enter
  cancela; borrar siempre pide un click.
- **Batería** de la cámara en la barra de herramientas.

## Requisitos

- macOS 12 o posterior, en Apple Silicon o Intel.
- Una RICOH THETA conectada por USB y encendida. Probada con la **THETA Z1**.

## Instalación

1. Bajá `LGA_ThetaMacExplorer_Mac_v<version>.dmg` de
   [Releases](../../releases/latest).
2. Abrilo y arrastrá **ThetaMacExplorer** a **Applications**.
3. Abrí la app desde Applications.

La app no está notarizada por Apple. Si macOS dice que no se puede abrir o que está
dañada, corré esto en Terminal y abrila de nuevo:

```bash
sudo xattr -cr "/Applications/ThetaMacExplorer.app"
```

Solo le quita a esa copia de la app la marca de cuarentena que macOS pone a lo que se
descarga. No desactiva Gatekeeper.

La primera vez, macOS puede pedir permiso para acceder a la cámara: la app lo necesita
para leer los archivos de la THETA.

## Uso

1. Conectá la cámara. Sus fotos y videos aparecen como tiles; las secuencias HDR, como un
   solo tile.
2. Elegí dónde guardar con **Save to…**.
3. Seleccioná uno o más ítems y apretá **Download**, o **Delete** para borrarlos de la
   cámara.

| Atajo | Acción |
|---|---|
| `⌘A` | Seleccionar todo |
| `Esc` | Deseleccionar |
| `⌘?` | Abrir la ayuda |

La ventana de Help muestra también dónde guarda la app sus archivos, y los abre en Finder:

- Settings: `~/Library/Application Support/LGA/ThetaMacExplorer/`
- Log: `~/Library/Logs/LGA/ThetaMacExplorer/debug.log`

## Compilar

Hace falta Xcode command line tools, CMake y Qt 6.5.3 en `~/Qt/6.5.3/macos`.

```bash
./compilar_dev.sh
```

compila un binario universal (arm64 + x86_64) en `build/` y lo lanza. `./deploy.sh` compila
la versión Release y arma el DMG.

## Créditos

Los HDRIs de la captura son de [Poly Haven](https://polyhaven.com) (CC0): Spiaggia di Mondello,
Sunflowers, Venice Sunset, Golden Gate Hills, Autumn Park, The Sky Is On Fire, Shanghai Bund,
Canary Wharf, Fireplace y Photo Studio Loft Hall.

## Autor

Desarrollado por Lega Pugliese — [lega.com.ar](https://lega.com.ar)
