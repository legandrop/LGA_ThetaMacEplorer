# Session and Download Behavior

## Persistencia

La app ahora guarda entre sesiones:

- geometria de ventana
- estado del splitter principal
- estado del splitter derecho
- carpeta de descarga seleccionada

Se usa `QSettings` con `organizationName` y `applicationName`.

## Logo

Se agrego un logo propio:

- `resources/icons/thetaexplorer_logo.svg`

Se usa como:

- icono de la aplicacion
- marca visual en la toolbar

## Browser y estado de descarga

Cuando llega el catalogo de la camara, la app:

1. agrupa el catalogo en `MediaAssetGroup`
2. calcula la carpeta esperada de descarga para cada grupo
3. revisa si ya existen en disco todos los archivos del grupo
4. marca el tile con badge `DL` si el grupo ya esta completamente descargado

## Estructura de descarga

Cada grupo se descarga en una subcarpeta propia dentro de la carpeta de destino.

Formato:

- `HDRI_AAMMDD_<nombre>_jpg`
- `HDRI_AAMMDD_<nombre>_dng`
- `video_AAMMDD_<nombre>_`

Donde `AAMMDD` es la fecha de captura del representativo (año 2 digitos, mes, dia).

Ejemplos:

- `HDRI_260402_R0021381-1389_jpg`
- `HDRI_260402_R0021381-1389_dng`
- `video_260401_R0010661.MP4_`

## Fechas en tiles

Cada tile muestra las fechas de los archivos del grupo.

Color:

- verde claro para elementos muy recientes
- degradado progresivo hasta verde oscuro dentro de una ventana de 28 horas
- gris para items mas viejos

Esto sirve para detectar rapidamente capturas recientes sin abrir metadata.
