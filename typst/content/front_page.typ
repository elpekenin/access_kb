#import "@elpekenin/tfm:0.1.0": images, vars

#page[
  #align(center + horizon)[#images.front]

  #align(center)[
    #text(size: 19pt, weight: "bold")[#vars.title]
  ]

  #align(bottom + right)[
    #text(weight: "bold", size: 13pt)[
      Escula Técnica Superior de Ingeniería de Telecomunicación

      Máster Universitario en Ingeniería de Telecomunicación

      Autor: Pablo Martínez Bernal

      Director: José Alfonso Vera Repullo

      #datetime.today().display(vars.date_format)
    ]
  ]
]
