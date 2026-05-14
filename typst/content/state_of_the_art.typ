#import "@elpekenin/tfm:0.1.0": h, images

#h[Firmware][
  En la actualidad existen diversos proyectos de código abierto que permiten a aficionados construir sus propios teclados, en vez de depender de dispositivos fabricados en masa por empresas.

  Hay multitud de otras librerías para desarrollar fimware de teclado, como FAK @fak, KMK @kmk, RMK @rmk o smart-keymap @smart-keymap, pero vamos a analizar las 2 opciones más completas: QMK @qmk y ZMK @zmk

  - Respecto a funcionalidad, QMK tiene una comunidad más grande y madura, por lo que ofrece muchas más funcionalidades "built-in" que ZMK. Por su lado, ZMK proporciona soporte inalámbrico (BLE)
  - En cuanto al hardware soportado, ambos son compatibles con multitud de MCUs distintos. ZMK soporta múltiples chips ARM de 32bit, QMK soporta un subconjunto de ellos además de algunos AVR
  - Para desarrollar código personalizado, QMK tiene la ventaja de que su _toolchain_ (compilador, linker, herramientras propias, ...) es mucho más sencillo de instalar y utilizar
]

#h[Integración con servicios][
  La opción más conocida del mercado es el Stream Deck. Su software permite hacer tareas básicas como abrir programas, control multimedia o escribir texto, pudiendo mostrar imágenes o iconos en las teclas, ya que posee una pantalla. Sin embargo, su elevado precio (150€) es un gran factor en contra

  #figure(
    images.elgato,
    caption: [Stream Deck elgato],
  )

  Otra opción muy conocida es AutoHotkey @ahk. Se trata de un programa de código abierto que permite potenciar cualquier teclado mapeando combinaciones de teclas con la ejecución de scripts. Estos scripts utilizan un lenguaje de programación propio que proporcionan multitud utilidades como funciones matemáticas, leer y escribir archivos, ejecutar DLLs, movimiento del raton, etc

  En el ámbito de código abierto existe Macro Deck @macro-deck, que permite usar un dispositivo móvil como un panel con varios botones, donde las acciones son similares a las ofrecidas por las alternativas nombradas anteriormente. Sin embargo, no parece tener una comunidad muy extensa ni ha recibido desarrollo en los últimos 2 años
]

#h[Hardware][
  Lo primero que debemos notar es que el panorama apenas ha variado a lo largo de los años, ni siquiera para adaptarse a las pequeñas pantallas táctiles de los móviles. Seguimos con las mismas ideas, a pesar de contar con tecnologías que superan antiguas limitaciones, permiten nuevas funcionalidades o proponen innovaciones.

  #figure(
    block[
      #align(horizon)[
        #grid(
          columns: (auto, auto),
          figure(
            images.model_m,
            caption: [IBM Model M (1984)],
            numbering: none,
            outlined: false,
          ),
          figure(
            images.logitech,
            caption: [Logitech MX Mechanical (2022)],
            numbering: none,
            outlined: false,
          ),
        )
      ]
      #figure(
        images.android,
        caption: [Teclado Android],
        numbering: none,
        outlined: false,
      )
    ],
    caption: [Comparativa teclado antiguo vs actual],
  )


  #h[Disposición de las teclas][
    Es posible que nunca nos lo hayamos preguntado, ya que tenemos muy interiorizada la forma de los teclados, pero si lo pensamos un poco, es peculiar la posición relativa de las teclas. Cada fila tiene un pequeño desfase respecto a las demás en vez de estar alineadas. Esto es un legado de sus antecesoras, las máquinas de escribir, donde por limitaciones mecánicas era necesario este diseño para evitar choques entre las piezas móviles de cada tecla.

    #figure(
      images.typewriter,
      caption: [Detalle de las letras en una máquina de escribir],
    )

    Este posicionamiento relativo de las teclas supone un problema a la hora de escribir, ya que la forma óptima de hacerlo sería la siguiente:

    #figure(
      images.mechanography,
      caption: ["Mapa" de mecanografía],
    )

    Sin embargo, para escribir así, las muñecas terminan en posiciones algo forzadas y los dedos realizan movimientos incómodos. Para solucionar esto surgieron los teclados ortolineales, donde todas las filas están alineadas y los dedos se mueven en línea recta. Estos teclados suelen tener todas las teclas del mismo tamaño, optimizando así la cantidad de teclas que se pueden incluir en el mismo espacio (donde antes había una barra espaciadora pueden entrar varias teclas). Como se puede ver en la siguiente imagen, normalmente también prescinden del teclado numérico para reducir el tamaño. Este modelo se conoce como "75%" ya que tiene 75 teclas, mientras que los teclados comunes ("100%") tienen 104/105 teclas. Otras variantes comunes son "40%", "60%" y "65%".

    #figure(
      images.ortholinear,
      caption: [Teclado ortolineal],
    )

    Otro legado que nos dejaron las máquinas de escribir es la distribución QWERTY, que probablemente sea la única que hemos visto a lo largo de nuestra vida. El problema con esta disposición es que, si bien distribuye las letras de forma que se usan las dos manos por igual, fue diseñada en la década de 1860 y uno de sus objetivos era reducir los atascos en las máquinas de escribir separando las teclas más usadas de la parte central.

    En cambio, ahora que gracias a la electrónica no tenemos estas limitaciones, se han diseñado distribuciones que minimizan la distancia media que se debe recorrer al escribir, permitiendo escribir más rápido y reduciendo la fatiga en los dedos una vez acostumbrados a ellas. Las dos más extendidas son DVORAK y COLEMAK.

    #figure(
      [
        #figure(
          images.qwerty,
          caption: [QWERTY],
          numbering: none,
          outlined: false,
        )
        #align(horizon)[
          #grid(
            columns: (auto, auto),
            gutter: 1em,
            figure(
              images.dvorak,
              caption: [DVORAK],
              numbering: none,
              outlined: false,
            ),
            figure(
              images.colemak,
              caption: [COLEMAK],
              numbering: none,
              outlined: false,
            ),
          )
        ]
      ],
      caption: [Comparativa de distribuciones],
    )
  ]

  #h[Ergonomía][

    Como hemos comentado anteriormente, uno de los problemas más importantes del diseño actual es su falta de adaptación a la anatomía humana.

    En los últimos años, entusiastas y pequeñas empresas han diseñado diferentes mejoras. Una de las ideas más básicas consiste en partir el teclado por la mitad para que las muñecas adopten una posición más natural, lo que se conoce como _split_.

    #figure(
      images.quefrency,
      caption: [Teclado _Quefrency_],
    )

    Un extra común en estos teclados consiste en elevar la parte central, de forma que la mano descanse y esté más cómoda. Un gran punto a favor de esta mejora es que se puede aplicar a cualquier teclado añadiendo algún sistema para ganar altura. Además, se puede ajustar esta altura según las necesidades del usuario.

    #figure(
      images.dygma_raise,
      caption: [Teclado _Dygma Raise_],
    )

    Muchos teclados otorgan mayor utilidad a los pulgares, ya que normalmente solo los utilizamos para la barra espaciadora; a esto se le llama _thumb cluster_.

    #figure(
      images.ergodox,
      caption: [Teclado _Ergodox_],
    )

    El mayor exponente de esta filosofía es el _Dactyl Manuform_, que tiene en cuenta la forma de las manos y ubica las teclas acorde al movimiento de los dedos. Algunos usuarios integran un _trackball_ para poder controlar el cursor sin tener que mover la mano entre el teclado y el ratón.

    #figure(
      images.manuform,
      caption: [Teclado _Dactyl Manuform_ con trackball],
    )

    <img:dactyl>

    En esta web @paper se pueden consultar varios estudios sobre la relación entre el diseño del teclado y sus efectos en la salud.
  ]
]
