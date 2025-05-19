#import "@elpekenin/tfm:0.1.0": images

#page[
  #align(center + horizon)[
    #(images.front)
  ]

  #align(center)[
    #text(size: 25pt, weight: "bold")[Revisitando el diseño del teclado]
  ]

  #align(bottom + right)[
    #text(weight: "bold", size: 13pt)[
      Autor: Pablo Martínez Bernal

      Director: José Alfonso Vera Repullo

      Máster Universitario en Ingeniería de Telecomunicación

      #datetime.today().display("[year]") // [month repr:short]
    ]
  ]

  // 0 to not render page number here
  #counter(page).update(0)
]
