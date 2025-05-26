#import "@elpekenin/tfm:0.1.0": config, images

#page[
  #align(center + horizon)[#images.front]

  #align(center)[
    #text(size: 25pt, weight: "bold")[#config.title]
  ]

  #align(bottom + right)[
    #text(weight: "bold", size: 13pt)[
      Autor: Pablo Martínez Bernal

      Director: José Alfonso Vera Repullo

      Máster Universitario en Ingeniería de Telecomunicación

      #datetime.today().display(config.date_format)
    ]
  ]
]
