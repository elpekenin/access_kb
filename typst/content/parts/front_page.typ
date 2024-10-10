#page[
  #align(center + horizon)[
    #image("../../images/UPCT-front.jpg", width: 70%)
  ]

  #align(center)[
    #text(size: 25pt, weight: "bold")[Revisitando el diseño del teclado]
  ]

  #align(bottom + right)[
    #text(weight: "bold", size: 13pt)[
      Autor: Pablo Martínez Bernal

      Director: José Alfonso Vera Repullo

      Máster Universitario en Ingeniería de Telecomunicación

      #datetime.today().display("[month repr:short]-[year]")
    ]
  ]

  // 0 to not render page number here
  #counter(page).update(0)
]
