# ChangeLog — LGA ThetaMacExplorer

v0.92:
- Build macOS / AGL: arreglo de raiz del warning `ld: framework 'AGL' not found` (y del posterior mismatch de version cuando el sistema exponia una stub). El fallback `-framework AGL` lo agrega `WrapOpenGL::WrapOpenGL` de Qt6 por transitividad via `Qt6::Gui`. En `CMakeLists.txt` se agregan dos bloques: el preventivo `set(OPENGL_agl_LIBRARY "")` antes de `find_package(Qt6 ...)` y el purgador de AGL de `WrapOpenGL::WrapOpenGL` despues. Se elimina la linea `target_link_options(... "LINKER:-weak_framework,AGL")` que quedaba como parche antiguo. `compilar.sh` y `compilar_dev.sh` dejan de crear el dummy `AGL.framework` y de pasar `-F $PWD` al linker; `compilar_dev.sh` incluye migracion one-shot para limpiar el `-F` heredado en la cache.
- [ Build macOS - Fix AGL sin dummy framework ]

v0.91:
- macOS: nuevo icono con el estandar glass de la suite LGA — squircle plano al 80% de la grilla de Apple (Tahoe le agrega el rim de Liquid Glass), glyph al 74% con padding, y fondo gris igual a PipeSync (gradiente 46→28, nunca negro). El glyph se toma de la fuente SVG limpia (`ThetaExplorer.svg`) por saturacion, no del icono horneado, para no arrastrar el borde del squircle viejo.
- macOS build: se arregla el fallo `'type_traits' file not found` que venia de un `CMakeCache` viejo apuntando a un SDK borrado tras un update de Xcode. `compilar_dev.sh` ahora limpia el build si el SDK cacheado no existe, y refresca el cache de iconos del bundle (`touch` + `lsregister -f`) tras compilar.
- [ Mac - Icono glass + fix build SDK viejo y cache de iconos ]

v0.90:
- Version inicial registrada; se empieza a mantener changelog y reglas `.codex` para el repo. [ Reglas - Se inicializa changelog y reglas de repo ]
