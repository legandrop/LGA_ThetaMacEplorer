# Instrucciones para LGA ThetaMacExplorer

- Todas las reglas de este repo (`AGENTS.md`, `.cursor/rules/`, `CLAUDE.md` si existiera) deben estar escritas en castellano.
- Este archivo es **espejo** de `.cursor/rules/instructions.mdc`. Al modificar acá, sincronizar tambien el `.mdc` — y viceversa.

## Layout del workspace

Esta app vive paralela al Base:

| Plataforma | Ubicacion del Base |
|---|---|
| macOS | `~/Desktop/Codin/LGA_Base_QT_C_Py` |
| Windows | `C:/portable/LGA_Base_QT_C_Py` |

Como estan paralelos, las referencias al Base usan path relativo `../LGA_Base_QT_C_Py` en las reglas de abajo.

## Hub de aprendizajes Qt/C++ (fuente de verdad compartida)

- `../LGA_Base_QT_C_Py/docs/_Doc_Aprendizaje_QT_C.md` es el hub central de aprendizajes Qt/C++ para todas las apps LGA. Tiene un INDICE al inicio, agrupado por topico (Painting, macOS especifico, Rich Text, Shortcuts, Threading, Layouts, Tooltips).
- **Antes de tocar codigo** en cualquiera de las areas listadas en el indice: **leer el INDICE del hub** (~30 segundos). Si el topico esta listado, ir a la seccion; si no esta, seguir con la tarea.
- **Escribir una entrada nueva** en el hub cuando un problema Qt/C++ cumple LAS TRES condiciones:
  1. Requirio 3+ iteraciones para resolver (o la causa raiz no era obvia).
  2. La solucion no aparece en la doc oficial de Qt, o es una trampa facil de caer.
  3. Es probable que reaparezca en otra app LGA.
- Fixes triviales NO van al hub. Al agregar una entrada, agregar TAMBIEN su linea al INDICE en la misma pasada.

## Docs unificadas del Base

Fuente de verdad para las convenciones LGA:

- `../LGA_Base_QT_C_Py/docs/_Doc_Aprendizaje_QT_C.md` — hub Qt/C++.
- `../LGA_Base_QT_C_Py/docs/Doc_CustomTooltip.md` — sistema de tooltips (cuando se porte a esta app, seguir esta guia).
- `../LGA_Base_QT_C_Py/readme.md` — arquitectura del template, `AppSettings` vs `SecureConfig`, layout del workspace cross-platform.

## Iconos de macOS

**El icono se disena en Icon Composer, no se genera.** El usuario exporta un `.icon` a `resources/icons/Alta/<Nombre>.icon` (es un directorio: `icon.json` + `Assets/`), se prueba con una app placeholder en `../LGA_IconLab`, y recien cuando el usuario aprueba se aplica aca. La fuente de verdad del pipeline es la seccion **5.2 Icon Composer** de `../LGA_IconLab/docs/Doc_Iconos_App.md`.

- Se compila con `actool` y **siempre** con `--standalone-icon-behavior all`: sin ese flag el `.icns` auxiliar trae solo 4 representaciones y llega hasta 256 px; con el son 10 y llega a 1024.
- Antes de compilar hay que copiar el `.icon` a `AppIcon.icon`: `actool` nombra el asset segun el archivo, y de ahi sale el valor de `CFBundleIconName`.
- Van **dos** recursos a `Contents/Resources/` y hacen falta los dos: `Assets.car` (el icono real, via `CFBundleIconName`) y `AppIcon.icns` (fallback, via `CFBundleIconFile`).
- **NO** convertir el `.icon` a un `.icns` suelto descartando el `Assets.car`: eso pierde el comportamiento moderno y reaparece el problema de tamano en Cmd+Tab.
- En macOS **no** setear el icono en runtime (`setWindowIcon`): manda el icono del bundle (ya esta con `#ifndef Q_OS_MAC` en `src/main.cpp`).
- Un build incremental **no** borra los recursos que el `CMakeLists` dejo de copiar: al cambiar de icono, revisar que no quede el `.icns` viejo adentro del bundle.

Tras cambiar el icono, `compilar_dev.sh` refresca el cache del bundle (`touch` + `lsregister -f`). El bundle id es fijo, asi que el cache muerde mas que en un placeholder; si sigue pegajoso, cerrar sesion y volver a entrar.

El pipeline generativo viejo (squircle plano al 80 %, glyph al 74 %, fondo `46 → 28`) sigue documentado en `../LGA_Base_QT_C_Py/docs/_Doc_Aprendizaje_QT_C.md` y **sigue siendo el que produce el `.ico` de Windows** en las apps que lo tienen. Esta app es solo macOS.

## Build

- **No compilar por defecto.** Compilar SOLO cuando:
  - El usuario lo pide explicitamente ("compila", "probalo", "buildealo", "corre la app").
  - Es fundamental para validar un cambio no trivial (refactor grande, cambio de header publico que afecta otras clases, cambio que puede romper build en otra plataforma).
- Al compilar, usar SIEMPRE el script del repo — NUNCA `cmake`, `ninja`, `make` u otros comandos manuales (los scripts hacen compilar + copiar deps + lanzar el ejecutable en un solo paso):
  - macOS: `./compilar_dev.sh`
- No ejecutar manualmente el binario despues de correr el script.
- No hacer builds limpios automaticamente. No borrar, vaciar ni recrear `build/` salvo pedido explicito. Preservar la cache incremental.
- Si la compilacion falla, intentar corregir el problema SIN limpiar primero.
- **Guard de SDK**: `compilar_dev.sh` chequea si el `CMAKE_OSX_SYSROOT` cacheado ya no existe (tipico tras un update de Xcode que cambia `MacOSXNN.sdk`); si desapareció, limpia el build para reconfigurar y evita el error `'type_traits' file not found`.

## Politica de idioma

Convencion LGA cross-app:

- **Texto visible en UI** (labels, botones, titulos de ventana, mensajes al usuario, tooltips): siempre en **INGLES**.
- **Comentarios de codigo y mensajes de log de debug**: siempre en **CASTELLANO**.
- **Strings de error internas** que no se muestran al usuario final: pueden estar en castellano.

## Documentacion

- Mantener actualizada la documentacion relevante relacionada con los cambios realizados.
- Si un cambio afecta comportamiento, arquitectura, UI o flujo de usuario documentado, actualizar los `.md` correspondientes en `docs/`.

## Versionado y ChangeLog

- Changelog principal: `docs/ChangeLog.md`.
- Formato de version: **`0.XX`** (2 decimales). Ej: `0.90 → 0.91 → 0.92`.
- **Este repo usa el patron de bump-per-entrada**: cada entrada nueva del changelog implica bump **+0.01**. Fuente humana de verdad es el ChangeLog.
- Agregar siempre una entrada nueva **arriba de todo**, con el nuevo numero de version. La entrada va inmediatamente debajo del numero de version.
- El changelog debe estar siempre escrito en castellano.
- Al final de la entrada nueva, agregar entre `[ ]` una sugerencia en castellano de nombre corto para el commit. Ejemplo: `[ Mac - Icono glass + fix build ]`.
- Debajo de la entrada nueva dejar una linea en blanco.
- Nunca reescribir ni modificar una entrada existente; siempre crear una entrada nueva.
- Si el cambio evoluciona en la misma sesion, agregar OTRA entrada nueva arriba con su propia sugerencia entre `[ ]`.

### Fuente unica de verdad para el numero de version

- **`CMakeLists.txt` es la unica fuente de verdad** del numero de version (via `project(ThetaMacExplorer VERSION 0.XX ...)`).
- Expuesta al C++ como macro `THETAMACEXPLORER_VERSION` via `target_compile_definitions`. La usa `src/main.cpp` (`app.setApplicationVersion(QLatin1String(THETAMACEXPLORER_VERSION))`).
- **NO hardcodear** el numero de version en `main.cpp` ni en ningun otro archivo C++.
- Al bumpear, solo actualizar el `project(... VERSION ...)` de `CMakeLists.txt` — todo lo demas se recompila.

## Help / About tab (si se agrega)

Si se agrega un Help/About tab visible al usuario:

- Version del Help sale de macro CMake (una vez migrada), NO hardcodear.
- Colores con constantes `k*` al top del `.cpp`, no literales hex sueltos.
- Iconos en `resources/icons/help/`, declarados en el `.qrc`, renderizados con `QSvgRenderer`.
- Al bumpear version, revisar en la misma tanda que los textos descriptivos del Help sigan coherentes con lo que la app hace.

## Debug logs

- **No** agregar debug flags nuevos ni logs de debug por defecto en cada cambio.
- Crear flags especificos SOLO cuando haga falta diagnosticar un bug importante, un problema real dificil de reproducir o una zona con riesgo tecnico claro.
- Para features normales, cambios triviales o ajustes de UI, NO agregar logs de debug nuevos.
- Errores reales y warnings recuperables deben quedar siempre visibles (no callarse). Si un error impide completar una accion del usuario, mostrar tambien una notificacion breve; el detalle tecnico va al log.

## Commits

- **NUNCA** hacer commits automaticamente. Solo commitear cuando el usuario lo pide explicitamente.
- Cuando el usuario pide commit, usar la sugerencia entre `[ ]` de la ultima entrada del ChangeLog como base del mensaje.

## GitHub, autoria y menciones

- En GitHub, el unico colaborador/autorizado esperado para este repo es `legandrop`.
- No agregar, invitar, aceptar, configurar ni modificar colaboradores, equipos, bots, apps, deploy keys, permisos de repo, reglas de acceso o cualquier ajuste que pueda hacer aparecer otro colaborador o identidad con acceso al repo.
- Si una herramienta, integracion o flujo pide autenticar, autorizar o agregar otro usuario/app con permisos sobre GitHub, detenerse y consultar al usuario antes de continuar.
- Todos los commits y pushes deben hacerse con la identidad de Git del usuario `legandrop`. No usar coautores, autores alternativos ni footers de atribucion automatica.
- No mencionar herramientas, agentes, modelos de IA ni asistentes automatizados en commits, changelogs, README, documentacion, PRs, issues, releases, comentarios de codigo, logs visibles o cualquier texto versionado del repo.
- Los mensajes de commit deben describir solamente el cambio funcional o documental hecho en el repo, sin referencias a quien o que lo genero.

### Que cuenta como "herramienta AI" (lista NO exhaustiva, ampliar si aparece algo nuevo)

- **Editores AI**: Cursor, Windsurf, Zed (con features AI), Fleet.
- **Extensiones/plugins AI**: GitHub Copilot, Codeium, Continue, Cody, Tabnine, Supermaven, Amazon Q.
- **Agentes CLI / IDE con agente**: Claude Code, Codex, Aider, Plandex, OpenHands, Devin, Roo Cline.
- **Modelos / chatbots**: ChatGPT, Claude, GPT-4, Gemini, LLaMA, "un LLM", "un modelo".
- **El proceso de descubrimiento con AI**: "mientras el agente compilaba", "cuando pedi a la IA que revise", "el modelo sugirio".

Editores NO AI (VS Code puro sin Copilot, IntelliJ, Vim, Emacs, Sublime, nano) se pueden mencionar SOLO si es estrictamente necesario para describir un problema real de esa herramienta especifica. En 99% de los casos no hace falta.

### Como reescribir un mensaje que menciona AI (aunque sea de reojo)

**Regla raiz**: el commit describe el cambio del PROYECTO en sus propios terminos, no del entorno de desarrollo de quien lo hizo. Si sacas el nombre de la herramienta y el mensaje sigue teniendo sentido, ese es el mensaje correcto.

- ❌ Mal: `Fix pkill generico en compilar.sh que reiniciaba extensiones de Cursor`
- ✅ Bien: `Fix pkill generico en compilar.sh (mataba procesos ajenos por match parcial de nombre)`

- ❌ Mal: `Refactor icono siguiendo sugerencias del review de la IA`
- ✅ Bien: `Refactor icono: glyph 74% + fondo gradiente para consistencia con PipeSync`

Esta regla aplica IGUAL a: commits, entradas del ChangeLog (incluyendo la sugerencia entre `[ ]`), issues, PRs, comentarios en codigo, mensajes de log visibles al usuario, y todo texto versionado del repo.
