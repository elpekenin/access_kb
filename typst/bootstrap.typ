//! Set everything up before including the actual content

/// Packages
#import "@preview/codly:1.0.0": codly, codly-init
#import "@preview/glossarium:0.4.2": make-glossary, register-glossary
#import "@preview/hydra:0.5.1": hydra

/// Local files
#import "glossary.typ": entry-list

/// Set rules
#set heading(numbering: "1.")

#set page(
  paper: "a4",
  margin: (y: 6em),
  header: context {
    let location = hydra(use-last: true)

    if location != none {
      let location = emph(location)

      if calc.even(here().page()) {
        align(left, location)
      } else {
        align(right, location)
      }
      line(length: 100%)
    }
  },
  footer: context {
    let page_number = here().page()

    // no numbering in front page
    if page_number == 0 {
      return
    }

    let position = if calc.even(page_number) {
      left
    } else {
      right
    }

    text(8pt)[
      #align(position)[
        #counter(page).display(
          both: true,
          (current, total) => {
            let text = [Pág. #current / #total]

            let width = measure(text).width * current / total

            block[
              #text
              #align(left)[
                #line(length: width, stroke: rgb(0, 150, 200))
              ]
            ]
          },
        )
      ]
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

/// Show rules
#show bibliography: set heading(numbering: "1.")
#show heading.where(level: 1): it => {
  pagebreak(weak: true) + it
}
#show: make-glossary
#show: codly-init.with()

/// Set up packages
#register-glossary(entry-list)

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

/// Actual content
#include "content/main.typ"
