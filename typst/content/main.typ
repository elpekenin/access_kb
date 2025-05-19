#import "@preview/codly:1.3.0": codly-range
#import "@preview/glossarium:0.5.4": print-glossary

#import "@elpekenin/tfm:0.1.0": cli, config, snippet, glossary, tools

//
// start of content
//

#include "front_page.typ"

// White page
#page[
  // 0 to not render page number here
  #counter(page).update(0)
]

// Index
#outline(depth: 2)

#if (config.render_images) {
  outline(
    target: figure.where(kind: image),
    title: [Listado de imágenes],
  )
}

#if (config.render_code) {
  outline(
    target: figure.where(kind: "snippet"),
    title: [Listado de código],
  )
}

#if (config.render_commands) {
  outline(
    target: figure.where(kind: "cmd"),
    title: [Listado de comandos],
  )
}

#heading("Listado de acronimos", outlined: false, numbering: none)
// TODO: avoid centered text
#print-glossary(
  glossary,
  show-all: true,
  disable-back-references: true,
)

#heading("Lenguajes y herramientas", outlined: false, numbering: none)
#print-glossary(
  tools,
  show-all: true,
  disable-back-references: true,
)

//
// Document begins, finally
//
= Resumen
#include "summary.typ"

= Introducción
#include "introduction.typ"

= Estado del arte
#include "state_of_the_art.typ"

= Diseño hardware
#include "hardware/main.typ"

= Implementación firmware
#include "firmware/main.typ"

= Software en ordenador
#include "software.typ"

= Lineas futuras
He detectado varios fallos a mejorar en revisiones de la PCB:
- Añadir test points
- Mount points para los tornillos, mayor diámetro
- Exponer los pines usados para buses SPI
- Usar pantalla capacitiva
- Reubicar las pantallas zona central (no debajo muñecas)
- Posicion jack TRRS
- Usar LVGL para interfaces mas complejas
- Usar pines para backlight, en vez de conectar a VCC

= Anexo I: Instalación de MicroPython
#include "micropython.typ"

= Anexo II. Código fuente del informe.
Aquí están adjuntas las primeras líneas del código fuente con el que he generado este documento. Como se puede ver, la sintaxis es más amigable que LaTeX.

#text(size: 8pt)[
  #codly-range(1, end: 20);

  #raw(
    read("../content/main.typ")
      .split("\n")
      .slice(0, count: config.source_lines)
      .join("\n"),
    lang: "typst",
  )
]

#align(right)[
  #text(weight: "bold", style: "italic")[
    Nota: Compilado con Typst #sys.version
  ]
]

// Bibliography
#bibliography(
  "../bibliography.yml",
  title: "Bibliografía",
)
