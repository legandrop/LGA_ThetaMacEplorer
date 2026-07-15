# ChangeLog — LGA ThetaMacExplorer

v0.95:
- Version / infra: se implementa el patron LGA "CMake single source". Se agrega `target_compile_definitions(ThetaMacExplorer PRIVATE THETAMACEXPLORER_VERSION="${PROJECT_VERSION}")` en `CMakeLists.txt`, y `src/main.cpp` cambia el hardcode `app.setApplicationVersion("0.93")` por `app.setApplicationVersion(QLatin1String(THETAMACEXPLORER_VERSION))`. Estado final: la version esta en UN solo lugar (`project(ThetaMacExplorer VERSION ...)` en CMakeLists), no requiere sincronizacion manual entre CMake y main.cpp. Las reglas del repo quedan actualizadas con la seccion "Fuente unica de verdad" al estado nuevo (ya no "pendiente de migrar").
- [ Version - CMake single source (THETAMACEXPLORER_VERSION macro) ]

v0.94:
- Reglas: las reglas del repo se alinean con el estandar unificado LGA (hub Qt/C++ y docs del Base, politica de idioma, Help/About, fuente unica de la version, autoria en GitHub), preservando lo propio del repo: versionado `0.XX` con bump `+0.01` por entrada, sincronizacion manual entre `CMakeLists.txt` y `main.cpp` hasta migrar a macro y el guard de SDK en `compilar_dev.sh`.
- [ Reglas - Unificar con el estandar LGA ]

v0.93:
- Build macOS: `compilar_dev.sh` y `compilar.sh` dejaban de reiniciar el extension host de VSCode al ejecutarse. Ambos usaban `pkill -f "$APP_NAME"` (con `APP_NAME="ThetaMacExplorer"`), un patrón demasiado genérico: `-f` matchea contra la command line completa de todos los procesos y puede pegar a extensiones/helpers de VSCode cuya command line contenga esa palabra. Se reemplaza por `pkill -f "${APP_NAME}.app/Contents/MacOS/${APP_NAME}"` (path completo del ejecutable dentro del `.app`), mismo criterio que usan las otras apps LGA. Se agrega comentario preventivo en ambos scripts para no volver al patrón genérico.
- [ Build macOS - Fix pkill genérico en compilar/compilar_dev que reiniciaba extensiones de VSCode ]

v0.92:
- Build macOS / AGL: arreglo de raiz del warning `ld: framework 'AGL' not found` (y del posterior mismatch de version cuando el sistema exponia una stub). El fallback `-framework AGL` lo agrega `WrapOpenGL::WrapOpenGL` de Qt6 por transitividad via `Qt6::Gui`. En `CMakeLists.txt` se agregan dos bloques: el preventivo `set(OPENGL_agl_LIBRARY "")` antes de `find_package(Qt6 ...)` y el purgador de AGL de `WrapOpenGL::WrapOpenGL` despues. Se elimina la linea `target_link_options(... "LINKER:-weak_framework,AGL")` que quedaba como parche antiguo. `compilar.sh` y `compilar_dev.sh` dejan de crear el dummy `AGL.framework` y de pasar `-F $PWD` al linker; `compilar_dev.sh` incluye migracion one-shot para limpiar el `-F` heredado en la cache.
- [ Build macOS - Fix AGL sin dummy framework ]

v0.91:
- macOS: nuevo icono con el estandar glass de la suite LGA — squircle plano al 80% de la grilla de Apple (Tahoe le agrega el rim de Liquid Glass), glyph al 74% con padding, y fondo gris igual a PipeSync (gradiente 46→28, nunca negro). El glyph se toma de la fuente SVG limpia (`ThetaExplorer.svg`) por saturacion, no del icono horneado, para no arrastrar el borde del squircle viejo.
- macOS build: se arregla el fallo `'type_traits' file not found` que venia de un `CMakeCache` viejo apuntando a un SDK borrado tras un update de Xcode. `compilar_dev.sh` ahora limpia el build si el SDK cacheado no existe, y refresca el cache de iconos del bundle (`touch` + `lsregister -f`) tras compilar.
- [ Mac - Icono glass + fix build SDK viejo y cache de iconos ]

v0.90:
- Version inicial registrada; se empieza a mantener changelog y reglas para el repo. [ Reglas - Se inicializa changelog y reglas de repo ]
