# ChangeLog — LGA ThetaMacExplorer

v0.91:
- macOS: nuevo icono con el estandar glass de la suite LGA — squircle plano al 80% de la grilla de Apple (Tahoe le agrega el rim de Liquid Glass), glyph al 74% con padding, y fondo gris igual a PipeSync (gradiente 46→28, nunca negro). El glyph se toma de la fuente SVG limpia (`ThetaExplorer.svg`) por saturacion, no del icono horneado, para no arrastrar el borde del squircle viejo.
- macOS build: se arregla el fallo `'type_traits' file not found` que venia de un `CMakeCache` viejo apuntando a un SDK borrado tras un update de Xcode. `compilar_dev.sh` ahora limpia el build si el SDK cacheado no existe, y refresca el cache de iconos del bundle (`touch` + `lsregister -f`) tras compilar.
- [ Mac - Icono glass + fix build SDK viejo y cache de iconos ]

v0.90:
- Version inicial registrada; se empieza a mantener changelog y reglas `.codex` para el repo. [ Reglas - Se inicializa changelog y reglas de repo ]
