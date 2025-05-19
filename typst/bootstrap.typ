//! Set everything up before including the actual content

// Packages
#import "@preview/codly:1.3.0": codly, codly-init
#import "@preview/glossarium:0.5.4": make-glossary, register-glossary
#import "@preview/hydra:0.6.1": hydra

#import "@elpekenin/tfm:0.1.0": glossary, tools

#let alignment(value) = {
  return if calc.even(value) { left } else { right }
}

// Set rules
#set heading(numbering: "1.")

#set page(
  paper: "a4",
  margin: (y: 6em),
  header: context {
    let location = hydra(use-last: true)

    if location != none {
      let location = emph(location)
      let page = here().page()

      align(alignment(page), location)
      line(length: 100%)
    }
  },
  footer: context {
    text(8pt)[
      #counter(page).display(
        both: true,
        (current, total) => {
          if current == 0 {
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

// Show rules
#show bibliography: set heading(numbering: "1.")
#show heading.where(level: 1): it => {
  pagebreak(weak: true) + it
}
#show: make-glossary
#show: codly-init.with()

// Set up packages
#register-glossary(glossary)
#register-glossary(tools)

#codly(
  languages: (
    bash: (name: "Bash", icon: "🐧", color: rgb("#CACACA")),
    diff: (name: "Diff", icon: "± ", color: rgb("#00AA00")),
    rust: (name: "Rust", icon: "🦀", color: rgb("#CE412B")),
  ),
  zebra-fill: none,
  number-format: none,
  stroke: 0.5pt + black,
)

// Actual content
#include "content/main.typ"
