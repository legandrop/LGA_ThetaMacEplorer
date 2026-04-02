# Estado actual de la arquitectura

## Resumen

Hoy la aplicacion trabaja exclusivamente con archivos individuales `CameraFileInfo`. La camara enumera items planos y la UI construye una grilla donde cada tile representa un archivo fisico.

Eso funciona para browsing simple, pero no para HDR porque:

- un HDR real esta compuesto por varias tomas
- puede existir tanto en JPG como en DNG
- la operacion esperada por el usuario no es por archivo, sino por set

## Flujo actual

### 1. Enumeracion

`ThetaBridge.mm` recorre `camera.mediaFiles` y emite un `QList<CameraFileInfo>` plano.

Campos relevantes hoy:

- `name`
- `uti`
- `devicePath`
- `sizeBytes`
- `creationDate`
- `isVideo`
- `isRaw`

No existe ningun campo para:

- grupo HDR
- burst
- variante JPG/DNG del mismo set
- indice dentro del set
- archivo representativo

### 2. Servicio

`ThetaCameraService` conserva la lista plana y tambien cola thumbnails por archivo. No hay transformacion de dominio entre "file list" y "view model".

### 3. UI

`ThumbnailGridWidget` crea un tile por archivo.
`MainWindow` selecciona archivos individuales.
`PreviewPanel` y `FileMetadataPanel` muestran una unica imagen o una multi-seleccion generica.

### 4. Acciones

`downloadFiles()` y `deleteFiles()` reciben `QList<CameraFileInfo>`.

Eso implica que:

- descargar opera sobre la seleccion plana
- borrar opera sobre la seleccion plana
- el usuario puede borrar solo una exposicion de un HDR sin querer
- la grilla no puede mostrar "1 item = 1 HDR"

## Limitacion principal

El problema no esta en ImageCaptureCore sino en el modelo interno.

La app hoy usa un unico tipo para todo:

- enumeracion
- presentacion
- seleccion
- descarga
- borrado

Para soportar HDR bien hace falta introducir una capa intermedia que represente assets agrupados.
