# Instrucciones Codex para LGA ThetaMacExplorer

- Todas las reglas de este repo (`.codex/`, `CLAUDE.md`) deben estar escritas en castellano.

## Versionado y changelog

- Changelog principal: `docs/ChangeLog.md`.
- Formato de version: **`0.XX`** (2 decimales). Ej: `0.90 → 0.91 → 0.92`.
- Despues de un cambio relevante (no trivial), subir la version **+0.01** usando `docs/ChangeLog.md`
  como fuente humana de verdad. **Cada entrada nueva de changelog implica un bump +0.01.**
- Agregar siempre una entrada nueva **arriba de todo**, con el nuevo numero de version. La entrada
  va inmediatamente debajo del numero de version.
- El changelog debe estar siempre escrito en castellano.
- Al final de la entrada nueva, agregar entre `[ ]` una sugerencia en castellano de nombre corto
  para el commit. Ejemplo: `[ Mac - Icono glass + fix build ]`.
- Debajo de la entrada nueva dejar una linea en blanco.
- Nunca reescribir ni modificar una entrada existente; siempre crear una entrada nueva.
- La version se sincroniza **a mano** (no hay `sync_version`): al bumpear, actualizar el mismo
  numero en `CMakeLists.txt` (`project(ThetaMacExplorer VERSION 1.XX ...)`) y en `src/main.cpp`
  (`app.setApplicationVersion("1.XX")`).

## Iconos de macOS

- Las reglas completas (reusables para todas las apps LGA) estan en
  `LGA_PipeSync_2/Docs/_Doc_Aprendizaje_QT_C.md`, seccion "Iconos de macOS":
  - Squircle **plano al 80%** de la grilla de Apple; macOS Tahoe agrega el rim (Liquid Glass).
  - **Glyph al 74%** del squircle, centrado, con padding (nunca toca el borde).
  - **Fondo = mismo gris que PipeSync** (gradiente `46 → 28`), **nunca negro**.
  - Usar el **glyph limpio transparente** (SVG rasterizado con `qlmanage`, o PNG transparente),
    **NO** extraerlo del icono horneado (arrastra el borde del squircle viejo como fantasma).
  - En macOS **no** setear el icono en runtime (`setWindowIcon`): manda el `.icns` del bundle
    (ya esta con `#ifndef Q_OS_MAC` en `src/main.cpp`).
- Tras cambiar el `.icns`, `compilar_dev.sh` refresca el cache de iconos del bundle
  (`touch` + `lsregister -f`). Si el cache de Tahoe sigue pegajoso, cerrar sesion y volver a entrar.

## Build

- `compilar_dev.sh` tiene un guard: si el `CMAKE_OSX_SYSROOT` cacheado ya no existe (tipico tras
  un update de Xcode que cambia `MacOSXNN.sdk`), limpia el build para reconfigurar y evita el
  error `'type_traits' file not found`.
