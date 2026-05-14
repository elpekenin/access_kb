#import "@preview/big-todo:0.2.0": todo, todo_outline
#import "@preview/codly:1.3.0": codly, codly-init
#import "@preview/glossarium:0.5.4": make-glossary, print-glossary, register-glossary

#import "@elpekenin/tfm:0.1.0": alignment, cli, glossary, h, images, snippet, tools, vars

//
// rendering config
//
#set heading(numbering: "1.")
#set page(
  paper: "a4",
  margin: (y: 6em),
  header: context {
    let here = here()
    let page = here.page()

    let prev = query(selector(heading).before(here)).last(default: none)
    let next = query(selector(heading).after(here)).first(default: none)

    // if we have a previous heading to shown as "current section"
    // ... and the next section does not start on the current page
    // -> show current section (prev heading) at the top of the page
    if prev != none and (next == none or next.location().page() != page) {
      align(alignment(page))[
        #counter(heading).display(prev.numbering)
        #prev.body
      ]
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

// https://forum.typst.app/t/how-to-pagebreak-before-an-heading-only-if-a-certain-condition-is-achieved/1691/17
#show heading.where(level: 1): it => {
  let threshold = 80%
  block(breakable: false, height: threshold)
  v(-threshold, weak: true)
  it
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

#h[Agradecimientos][
  #include "content/acknowledgements.typ"
]

#if (vars.render_todos) {
  todo_outline
}

// index
#outline(depth: 2)

// list images
#if (vars.render_images) {
  outline(
    target: figure.where(kind: image),
    title: [Listado de imágenes],
  )
}

// list code blocks
#if (vars.render_code) {
  outline(
    target: figure.where(kind: "snippet"),
    title: [Listado de código],
  )
}

// list command blocks
#if (vars.render_commands) {
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

#h[Resumen][
  #include "content/summary.typ"
]

#h[Estado del arte][
  #include "content/state_of_the_art.typ"
]

#h[Diseño hardware][
  #include "content/hardware.typ"
]

#h[Implementación firmware][
  #include "content/firmware.typ"
]

#h[Software en ordenador][
  #include "content/software.typ"
]

#h[Lineas futuras][
  #if (vars.render_todos) {
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
]

// = Anexo I: Instalación de MicroPython
// #include "content/micropython.typ"

#h[Anexo I. Código fuente del informe][
  Aquí se pueden ver las primeras líneas del código fuente con el que he generado este documento. He utilizado un lenguaje llamado Typst, que es un proyecto en desarrollo que intenta ser un reemplazo moderno para LaTeX.

  #text(size: 8pt)[
    #raw(
      read("main.typ").split("\n").slice(0, count: vars.source_lines).join("\n"),
      lang: "typst",
    )
  ]

  #align(right)[
    #text(weight: "bold", style: "italic")[
      Compilado con Typst #sys.version
    ]
  ]
]

// Bibliography
#bibliography(
  "bibliography.yml",
  title: "Enlaces",
  style: "american-physics-society",
)
