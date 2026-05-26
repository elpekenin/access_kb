#import "@elpekenin/tfm:0.1.0": h, images

#h[Hardware][
  Lo más notable es que el teclado apenas se ha modernizado desde su aparición; ni siquiera con las pantallas táctiles de los móviles.
  #figure(
    block[
      #align(bottom)[
        #grid(
          columns: (auto, auto, auto),
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
          figure(
            images.android,
            caption: [Teclado Android],
            numbering: none,
            outlined: false,
          ),
        )
      ]
    ],
    caption: [Progreso diseño teclado],
  )

  Los teclados presentan un pequeño desfase entre sus filas, esto es un legado de sus predecesoras las máquinas de escribir; donde, por limitaciones mecánicas, esto era necesario para evitar choques entre las piezas móviles de cada tecla.
  #figure(
    images.typewriter,
    caption: [Detalle de una máquina de escribir],
  )

  Aunque esta limitación ya no existe, no se ha reflejado en un cambio de diseño, esto es un inconveniente para escribir ya que la forma óptima de hacerlo sería la siguiente:
  #figure(
    images.mechanography,
    caption: [Diagrama de mecanografía],
  )

  Sin embargo, al escribir así, las muñecas terminan en posición forzada y los dedos realizan movimientos incómodos. Los teclados ortolineales surgen como alternativa, con teclas alineadas y movimientos rectos de los dedos. Incluso existen estudios @paper sobre el impacto del diseño de los teclados en la salud

  Estos diseños suelen tener todas las teclas del mismo tamaño, optimizando el uso del espacio y eliminando las teclas menos usadas (teclado numérico, flechas, etc) para reducir dimensiones.
  #figure(
    images.ortholinear,
    caption: [Teclado ortolineal],
  )

  También surgen teclados divididos (_split_), permitiendo una posición más natural de las muñecas. Algunos modelos también elevan la parte central para mayor comodidad.
  #figure(
    images.quefrency,
    caption: [Teclado _Quefrency_],
  )

  Otros dotan de mayor utilidad a los pulgares poniendo varias teclas a su alcance, lo que se denomina _thumb cluster_.
  #figure(
    images.ergodox,
    caption: [Teclado _Ergodox_],
  )

  #pagebreak()

  También herencia de las máquinas de escribir es la distribución de las letras en las teclas (QWERTY). Patentada en 1868, su objetivo era evitar los atascos mecánicos separando entre sí las combinaciones de letras más frecuentes.

  Al desaparecer estas limitaciones con la electrónica, aparecen opciones que minimizan la distancia media que recorren los dedos y permiten una mayor velocidad de escritura y reducir la fatiga.
  #figure(
    align(horizon)[
      #grid(
        columns: (auto, auto, auto),
        gutter: 1em,
        figure(
          images.qwerty,
          caption: [QWERTY],
          numbering: none,
          outlined: false,
        ),
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

    ],
    caption: [Comparativa de distribuciones],
  )
]

#h[Firmware][
  Existen diversos proyectos de código abierto que permiten a aficionados construir sus propios teclados y programarlos a placer, en vez de depender de dispositivos fabricados en masa por empresas donde las opciones de personalización son escasas o nulas.

  Aunque existen otras alternativas (FAK @fak, KMK @kmk, RMK @rmk, smart-keymap @smart-keymap, ...) vamos a analizar las 2 opciones más maduras y conocidas: QMK @qmk y ZMK @zmk

  En lo que respecta a funcionalidad, QMK tiene una comunidad mayor, por lo que ofrece más funcionalidades _built-in_ que ZMK y resulta sencillo encontrar código de otros usuarios que adaptar y añadir a nuestro firmware.

  En cuanto a hardware, ambos son compatibles con multitud de MCU gracias los RTOS que utilizan. ZMK gana en este aspecto dado que usa Zephyr @zephyr, obteniendo acceso a un amplio catálogo @hw-zmk de dispositivos, soportando además BLE para hacer teclados inalámbricos. Por su parte, QMK puede usar ChibiOS @chibios o LUFA @lufa, lo que también le brinda buena compatibilidad @hw-qmk

  Para configuración, ZMK resulta más sencillo en cambios pequeños (p.ej: modificar la letra asignada a una tecla), tan sólo necesitamos modificar el archivo en GitHub y se compilará en la nube el nuevo binario. Sin embargo, QMK resulta más sencillo para cambios grandes (p.ej: añadir soporte para nuestro diseño hardware) ya que su _toolchain_ (compilador, linker, herramientras propias, ...) es mucho más sencillo de instalar y utilizar que el paquete de software empleado por ZMK
]

#h[Integraciones][
  #block(breakable: false)[
    Para tener funcionalidad extra al alcance de la mano, la opción más conocida del mercado es el Stream Deck. Es un dispositivo con pocas teclas cuyo software permite automatizar tareas como abrir programas, control multimedia o escribir texto. Se pueden mostrar imágenes en sus botones ya que son transparentes y tienen una pantalla detrás. Sin embargo, su elevado precio (150€) es un gran factor en contra
    #figure(
      images.elgato,
      caption: [Stream Deck elgato],
    )
  ]

  Otra opción popular es AutoHotkey @ahk, se trata de un programa de código abierto que puede potenciar cualquier teclado haciendo que combinaciones de teclas causen la ejecución de scripts. Estos scripts utilizan un lenguaje de programación propio y proporciona funciones para leer y escribir archivos, ejecutar DLLs, mover el ratón, etc

  Otra opción _open source_ es Macro Deck @macro-deck, que permite usar un dispositivo móvil como si fuera un panel con varios botones, las acciones ofrecidas son similares a los programas anteriores. Sin embargo, no parece tener una comunidad muy extensa ni ha recibido desarrollo en los últimos años
]
