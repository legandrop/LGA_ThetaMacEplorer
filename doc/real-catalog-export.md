# Export de catalogo real

## Objetivo

Antes de cerrar una heuristica HDR, la app necesita evidencia real del catalogo que devuelve la Z1.

Por eso se agrego un export de diagnostico desde la UI.

## Donde aparece

En la toolbar de la app hay un boton `Export catalog`.

Se habilita cuando:

- la camara esta conectada
- ya se cargo el catalogo actual

## Que exporta

Genera un JSON con dos bloques principales:

- `files`
  El catalogo crudo que hoy recibe la app desde `ThetaBridge`

- `analysis`
  Ayudas de inspeccion para ver si hay patrones reales de secuencia

## Datos por archivo

Cada item incluye:

- `name`
- `uti`
- `devicePath`
- `sizeBytes`
- `creationDateIso`
- `isVideo`
- `isRaw`
- `normalizedName`

Dentro de `normalizedName` se exporta:

- extension normalizada
- base name
- stem
- stem uppercase
- prefijo antes de un numero final
- numero final si existe
- ancho del numero final

## Analisis incluido

El JSON tambien agrega:

- `jpegAdjacencyHints`
- `rawAdjacencyHints`
- `jpegTemporalCandidateRuns`
- `rawTemporalCandidateRuns`

Eso no es la heuristica final de HDR. Es una capa de inspeccion para validar:

- si realmente hay prefijos repetidos
- si realmente hay numeracion consecutiva
- si realmente JPG y DNG aparecen en rafagas separables por tiempo

## Uso recomendado

1. conectar la Z1
2. cargar varios casos reales
3. exportar el catalogo
4. revisar ejemplos reales de sets JPG y DNG
5. ajustar la heuristica HDR en base a esos datos

## Importante

La herramienta no agrupa HDR todavia.

Su funcion es dejar trazabilidad y evidencia para que la siguiente etapa de implementacion se apoye en patrones reales de tu camara.
