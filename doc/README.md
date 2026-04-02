# ThetaMacExplorer Docs

Este directorio centraliza la documentacion funcional y tecnica del proyecto.

## Documentos

- `architecture-current-state.md`
  Describe como funciona hoy la app, que objetos maneja y donde estan los limites actuales para soportar HDR agrupado.

- `hdr-grouping-proposal.md`
  Propone la estrategia recomendada para detectar secuencias HDR, representarlas en la UI y operar sobre sets completos para descargar y borrar.

- `implementation-plan.md`
  Baja la propuesta a fases concretas de implementacion para evitar mezclar refactor, heuristica y cambios de UX en un mismo paso.

- `real-catalog-export.md`
  Explica la herramienta de exportacion de catalogo real desde la app para calibrar la deteccion HDR sobre datos de la Z1 y no sobre supuestos.

- `logging.md`
  Documenta el sistema de logging central, las categorias, las variables de entorno y el criterio de uso.

- `hdr-grouping-observed-patterns.md`
  Resume el patron real observado en el catalogo exportado de la Z1 y la regla de agrupacion implementada a partir de esa evidencia.

- `session-and-download-behavior.md`
  Documenta persistencia de ventana, carpeta de descarga, logo, badge de descargado y la estructura final de subcarpetas por grupo.

## Criterio

Antes de tocar comportamiento visible conviene mantener actualizada esta carpeta. En este proyecto es especialmente importante porque la app necesita separar:

- catalogo plano que entrega la camara
- assets agrupados que consume la UI
- acciones por archivo real versus acciones por set HDR

Sin esa distincion, la UI termina mezclando conceptos y aparecen errores de seleccion, preview y borrado.
