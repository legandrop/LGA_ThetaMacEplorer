# Session and Download Behavior

## Persistencia

La app ahora guarda entre sesiones:

- geometria de ventana
- estado del splitter principal
- estado del splitter derecho
- carpeta de descarga seleccionada

Se guardan en un INI: `~/Library/Application Support/LGA/ThetaMacExplorer/settings.ini`
(`AppPaths::settingsFile()`), igual que el `AppSettings` de las apps LGA. Hasta v0.993 iban al
plist `~/Library/Preferences/com.lga.ThetaMacExplorer.plist` (`QSettings` nativo): la primera vez
que arranca sin `settings.ini`, `AppPaths::migrateLegacySettings()` copia las claves `window/*` y
`downloads/*` del plist. El plist no se borra.

## Icono

El icono de la app es el del bundle (`Assets.car` + `AppIcon.icns`); en macOS no se setea en
runtime. La toolbar ya no lleva logo: arranca con el estado de la camara.

## Browser y estado de descarga

Cuando llega el catalogo de la camara, la app:

1. agrupa el catalogo en `MediaAssetGroup`
2. calcula la carpeta esperada de descarga para cada grupo
3. revisa si ya existen en disco todos los archivos del grupo
4. marca el tile con el badge **Saved** (check) si el grupo ya esta completamente descargado, o
   **Partial** (medio circulo) si hay algo en disco pero no todo

## Descargar sobre algo que ya existe

Si algun item seleccionado ya tiene algo en la carpeta de descarga, se pregunta antes de bajar:

- **Skip Existing** (Enter, la opcion por defecto): se saltean los archivos que ya estan en disco
  con el mismo tamano y se baja el resto, asi que un set a medio bajar se completa. Hasta v0.993
  este boton calculaba la lista pero encolaba todos los grupos igual.
- **Replace**: borra la subcarpeta local del grupo y la vuelve a bajar. Solo con click (no entra
  en el Tab).
- **Cancel** (Esc).

## Borrar de la camara

El cartel dice cuantos items y cuantos archivos se borran en total (un set HDR son varios
archivos). Enter y Esc cancelan; borrar requiere el click en **Delete**, que tampoco entra en el
Tab. Al terminar se saca lo borrado del catalogo sin esperar a que la camara lo vuelva a
enumerar; si no queda nada, aparece «The camera is empty».

## Desconexion

Si la camara se desconecta con una descarga en curso, se resetea el estado (progreso y botones).
Si se desconecta con el cartel de archivos existentes abierto, no se borra ni se baja nada.

## Avisos

Ademas de la barra de estado, los errores y los finales de lote muestran un aviso flotante
(`MessagePopover`, portado de FileManagerS3): el de exito se cierra solo a los 4 s, el de error
queda hasta que se lo cierra. Una descarga muestra un solo aviso al terminar el lote, no uno por
archivo.

## Estructura de descarga

Cada grupo se descarga en una subcarpeta propia dentro de la carpeta de destino.

Formato:

- `HDRI_AAMMDD_<nombre>_jpg`
- `HDRI_AAMMDD_<nombre>_dng`
- `video_AAMMDD_<nombre>_`

Donde `AAMMDD` es la fecha de captura del representativo (año 2 digitos, mes, dia).

Ejemplos:

- `HDRI_260402_R21381-21389_jpg` (set HDR: el rango pierde los ceros de adelante)
- `HDRI_260402_R21381-21389_dng`
- `HDRI_260402_R0021390.JPG_jpg` (foto suelta: el nombre completo)
- `video_260401_R0010661.MP4_`

## Fechas en tiles

Cada tile muestra las fechas de los archivos del grupo.

Color:

- verde claro para elementos muy recientes
- degradado progresivo hasta verde oscuro dentro de una ventana de 28 horas
- gris (`#8f8f8f`) para items mas viejos

Fecha y hora van en una sola linea: `Thu 1 October · 17:42`.

Esto sirve para detectar rapidamente capturas recientes sin abrir metadata.
