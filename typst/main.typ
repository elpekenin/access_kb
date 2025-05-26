#import "@preview/big-todo:0.2.0": todo, todo_outline
#import "@preview/codly:1.3.0": codly, codly-init
#import "@preview/glossarium:0.5.4": (
  make-glossary,
  register-glossary,
  print-glossary,
)
#import "@preview/hydra:0.6.1": hydra

#import "@elpekenin/tfm:0.1.0": cli, config, snippet, glossary, images, tools

//
// functions
//
#let alignment(value) = {
  return if calc.even(value) { left } else { right }
}

//
// rendering config
//
#set heading(numbering: "1.")
#set page(
  paper: "a4",
  margin: (y: 6em),
  header: context {
    let page = here().page()
    let location = hydra(use-last: true)

    if location != none {
      align(alignment(page), emph(location))
      line(length: 100%)
    }
  },
  footer: context {
    text(8pt)[
      #counter(page).display(
        both: true,
        (current, total) => {
          if (current <= locate(<__start__>).page()) {
            return
          }

          let text = [Pág. #current / #total]
          let progress = measure(text).width * current / total

          align(alignment(current))[
            #block[
              #text
              #align(left)[
                #line(length: progress, stroke: rgb(0, 84, 160))
              ]
            ]
          ]
        },
      )
    ]
  },
)
#set par(justify: true)
#set text(
  font: "Open Sans",
  lang: "es",
  ligatures: true,
  size: 12pt,
  slashed-zero: true,
)
#set outline.entry(fill: block(width: 100% - 1.5em)[ #repeat(" . ")])

#show bibliography: set heading(numbering: "1.")
#show heading.where(level: 1): it => {
  pagebreak(weak: true) + it
}
#show: make-glossary
#show: codly-init.with()

//
// configure packages
//
#register-glossary(glossary)
#register-glossary(tools)
#codly(
  languages: (
    bash: (name: "Bash", icon: "🐧", color: rgb("#CACACA")),
    c: (name: "C", color: rgb("#0000FF")),
    diff: (name: "Diff", icon: "± ", color: rgb("#00AA00")),
    rust: (name: "Rust", icon: "🦀", color: rgb("#CE412B")),
  ),
  zebra-fill: none,
  number-format: none,
  stroke: 0.5pt + black,
)

//
// start
//
#include "content/front_page.typ"

#page([])

#if (config.render_todos) {
  todo_outline
}

// index
#outline(depth: 2)

// list images
#if (config.render_images) {
  outline(
    target: figure.where(kind: image),
    title: [Listado de imágenes],
  )
}

// list code blocks
#if (config.render_code) {
  outline(
    target: figure.where(kind: "snippet"),
    title: [Listado de código],
  )
}

// list command blocks
#if (config.render_commands) {
  outline(
    target: figure.where(kind: "cmd"),
    title: [Listado de comandos],
  )
}

// acronyms
#heading("Listado de acronimos", outlined: false, numbering: none)
// TODO: avoid centered text
#print-glossary(
  glossary,
  show-all: true,
  disable-back-references: true,
)

// list languages and tools
#heading("Lenguajes y herramientas", outlined: false, numbering: none)
#print-glossary(
  tools,
  show-all: true,
  disable-back-references: true,
)

<__start__>

= Resumen
#include "content/summary.typ"

= Estado del arte
#include "content/state_of_the_art.typ"

= Diseño hardware
#include "content/hardware.typ"

= Implementación firmware
#include "content/firmware.typ"

= Software en ordenador
#include "content/software.typ"

= Lineas futuras
#if (config.render_todos) {
  todo("Desarrollar")
}

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
#include "content/micropython.typ"

= Anexo II. Código fuente del informe.
Aquín se puede ver las primeras líneas del código fuente con el que he generado este documento. He utilizado un lenguaje llamado Typst, que es un proyecto en desarrollo que intenta ser un reemplazo moderno para LaTeX.

#text(size: 8pt)[
  #raw(
    read("main.typ")
      .split("\n")
      .slice(0, count: config.source_lines)
      .join("\n"),
    lang: "typst",
  )
]

#align(right)[
  #text(weight: "bold", style: "italic")[
    Compilado con Typst #sys.version
  ]
]

// Bibliography
#bibliography(
  "bibliography.yml",
  title: "Bibliografía",
)
