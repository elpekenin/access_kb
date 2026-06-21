#import "@preview/codly:1.3.0": codly, codly-init
#import "@preview/glossarium:0.5.10": make-glossary, print-glossary, register-glossary

#import "@elpekenin/tfm:0.1.0": alignment, cli, glossary, h, images, snippet, vars

#set heading(numbering: "1.")
#set page(
  paper: "a4",
  margin: (y: 6em),
  // show current section's title
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
  // page counter with a progress bar
  footer: context {
    text(8pt)[
      #counter(page).display(
        both: true,
        (current, total) => {
          if (current <= locate(<__content_start>).page()) {
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
  let threshold = 90%
  block(breakable: false, height: threshold)
  v(-threshold, weak: true)
  it
}
#show: make-glossary
#show: codly-init.with()
#show figure: set block(breakable: true)

//
// configure packages
//
#register-glossary(glossary)
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

#heading(
  [Agradecimientos],
  numbering: none,
  outlined: false,
)
#include "content/acknowledgements.typ"

#heading(
  [Resumen],
  numbering: none,
  outlined: false,
)
#include "content/summary.typ"

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
#heading("Listado de acrónimos", outlined: false, numbering: none)
// TODO: avoid centered text
#print-glossary(
  glossary,
  show-all: true,
  disable-back-references: true,
)

<__content_start>

#h[Contexto][
  #include "content/context.typ"
]

#h[Estado del arte][
  #include "content/state_of_the_art.typ"
]

#h[Hardware][
  #include "content/hardware.typ"
]

#h[Firmware][
  #include "content/firmware.typ"
]

#h[Software de control][
  #include "content/software.typ"
]

#h[Integración IoT][
  #include "content/m5.typ"
]

#h[Líneas futuras][
  #include "content/future.typ"
]

#h[Anexo. Código fuente del informe][
  Aquí se pueden ver las primeras líneas del archivo en el que se ha escrito este documento, usando un lenguaje llamado Typst, es un proyecto en desarrollo que intenta ser un reemplazo moderno para LaTeX.

  #text(size: 8pt)[
    #raw(
      read("main.typ").split("\n").slice(vars.offset, count: vars.offset + vars.source_lines).join("\n"),
      lang: "typst",
    )
  ]

  #align(right)[
    #text(weight: "bold", style: "italic")[
      Informe generado con Typst #sys.version
    ]
  ]
]

// Bibliography
#bibliography(
  "bibliography.yml",
  title: "Enlaces",
  style: "american-physics-society",
)
