# Interfaz

Como esta armada la UI desde v0.994, cuando se alineo con el resto de las apps LGA.

## Paleta y estilos

- La paleta vive en `include/thetaexplorer/ColorUtils.h`, con los nombres de `COLOR_VARS` de
  `LGA_Base_QT_C_Py` (`bg_principal`, `bg_items`, `bg_tabs`, `txt_principal`, `txt_secundario`,
  `violeta_oscuro`, `violeta_claro`...) mas los de estado (`color_success`, `color_warning`,
  `color_error`).
- `resources/styles/dark_theme.qss` usa esos nombres como variables; `ColorUtils::loadStyleSheet()`
  los reemplaza (claves largas primero) y `main.cpp` aplica el resultado a la app.
- Los widgets se estilan por `objectName` y propiedades dinamicas (`state`, `kind`, `variant`,
  `low`). No hay `setStyleSheet` con colores sueltos en los widgets; lo que se pinta por codigo
  (fondo de los tiles, glyphs) toma sus colores de constantes `k*` al principio del `.cpp`.
- Texto secundario: `#8f8f8f` como minimo, nunca los `#333`/`#444` de antes (contraste 1.5:1 a
  1.9:1 sobre los fondos oscuros).

## Fuente

Inter embebida en `resources/fonts` (Regular, Medium, Bold). El QSS y `main.cpp` declaran **una
sola familia**: con una lista de familias Qt carga todas las fuentes del sistema al arrancar (hub
Qt/C++ del Base, "font-family con lista"). Si Inter no queda registrada, `main.cpp` lo avisa en el
log.

## Iconos

Glyphs SVG monocromos en `resources/icons/ui`, dibujados en negro con margen dentro del
`viewBox`. `UiIcons::pixmap()` los rasteriza con supersampling y rect logico explicito y los tine
con `CompositionMode_SourceIn` (hub Qt/C++, "SVG chico tenido sin recorte en QLabel");
`UiIcons::buttonIcon()` arma el `QIcon` con color normal y deshabilitado. Nada de emojis ni
simbolos Unicode como iconos: macOS pinta los emojis en color.

Hace falta `Qt6::Svg`. `compilar_dev.sh` y `compilar.sh` re-deployan si el bundle no trae
`QtSvg.framework`.

## Ventana principal

- **Toolbar**, de izquierda a derecha: estado de la camara (`StatusDot`: punto verde conectada, arco
  girando buscando) y bateria (icono + %, en rojo con 20 % o menos); «Save to…» con la ruta
  recortada por el principio; Refresh; progreso; **Download N** (accion principal, con la cantidad
  seleccionada); **Delete** separado y solo con borde; boton «?» del Help.
- **Grilla**: un `QStackedWidget` alterna la grilla y `EmptyStateWidget`. Sin camara: «Connect your
  RICOH THETA» con la fila «Searching for camera…». Camara conectada sin archivos: «The camera is
  empty».
- **Barra de estado**: `#101010`, texto 12 px en `txt_secundario` o en el color de estado.

## Carteles

`ConfirmDialog` (`src/ui/ConfirmDialog.cpp`) copia el chrome de los dialogos del Player de
MediaTools (Replace track, Import Shot Review): fondo `bg_principal`, icono de 44 px, titulo 15 px
en negrita, subtitulo gris, fila de atajos con icono de teclado y teclas en violeta, y botones del
mismo ancho sin borde (neutral `#2a2a2a`, accion `violeta_oscuro`, borrar en rojo apagado). **Sin
contorno de foco**: que tecla hace que lo dicen el texto del boton («Cancel (esc)», «Skip Existing
(enter)») y la fila de atajos. En macOS los botones piden `Qt::StrongFocus` para que Tab los
recorra.

No se usa `QMessageBox`.

## Avisos flotantes

`MessagePopover` (`include/thetaexplorer/popover/`, portado de `LGA_FileManagerS3`, ver
`../LGA_Base_QT_C_Py/docs/Doc_MessagePopover.md`). `MainWindow::notifySuccess()` lo muestra 4 s;
`notifyError()` lo deja hasta que se cierra, y un error nuevo reemplaza al que este abierto. Las
descargas avisan una sola vez por lote.

## Help

`HelpDialog`, modal, se abre con el boton «?», con **Cmd+?** (`QKeySequence::HelpContents`) y con
«About ThetaMacExplorer» del menu de la app (`QAction::AboutRole`). Se cierra con Esc o con el
boton de la ventana, sin boton Close, como el Help del Player.

Contenido: nombre en violeta de marca `rgb(127, 98, 170)` con la version de
`QApplication::applicationVersion()`, «Developed by Lega Pugliese», link a **lega.com.ar** (violeta
apagado `#8676a3`, `#a796c6` subrayado al pasar el mouse), atajos y rutas clicables (descargas,
logs, settings) que abren la carpeta en Finder. Los colores son constantes `kHelp*` al principio del
`.cpp`. Si se agrega un atajo, se lista ahi en la misma pasada.

## Pendiente

- El selector de carpeta usa el dialogo de Qt (`DontUseNativeDialog`) y no esta documentado por
  que. Antes de pasarlo al nativo hay que probar que no choque con la sesion de ImageCaptureCore.
- No hay tooltips: si hacen falta, portar `CustomTooltip` del Base
  (`../LGA_Base_QT_C_Py/docs/Doc_CustomTooltip.md`), nunca `setToolTip` nativo.
