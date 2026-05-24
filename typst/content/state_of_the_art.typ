#import "@preview/big-todo:0.2.0": todo

#import "@elpekenin/tfm:0.1.0": h, images


#h[Diseño][
  Lo más notable es que el teclado apenas se ha modernizado desde la invención de su predecedora, la máquina de escribir; ni siquiera con las pantallas táctiles de los móviles.
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
    caption: [Progreso diseño teclado],
  )

  Los teclados presentan un pequeño desfase entre las filas de teclas, esto es un legado de sus antecesoras, donde por limitaciones mecánicas era necesario este diseño para evitar choques entre las piezas móviles de cada tecla.
  #figure(
    images.typewriter,
    caption: [Detalle de una máquina de escribir],
  )

  Aunque esta limitación ya no existe, no se ha reflejado en un cambio de diseño, esto es un inconveniente para escribir ya que la forma óptima de hacerlo sería la siguiente:
  #figure(
    images.mechanography,
    caption: [Diagrama de mecanografía],
  )

  Sin embargo, al escribir así, las muñecas terminan en posición forzada y los dedos realizan movimientos incómodos. Los teclados ortolineales surgen como alternativa, con teclas alineadas y movimientos rectos de los dedos. Otras alternativas de diseño incluyen el uso de teclas con el mismo tamaño (optimizando el uso del espacio) o prescindir de teclas menos usadas (teclado numérico, flechas, etc) para reducir dimensiones.
  #figure(
    images.ortholinear,
    caption: [Teclado ortolineal],
  )

  En los últimos años, los entusiastas han optado por partir el teclado en 2 piezas, para que las muñecas adopten una posición más natural, esto se conoce como teclado _split_. A veces se eleva la parte central para que la muñeca quede en una posición más cómoda.
  #figure(
    images.quefrency,
    caption: [Teclado _Quefrency_],
  )

  También hay teclados que dotan de mayor utilidad a los pulgares poniendo varias teclas a su alcance, lo que se denomina _thumb cluster_.
  #figure(
    images.ergodox,
    caption: [Teclado _Ergodox_],
  )

  Se han realizado varios estudios @paper sobre el impacto que tiene en la salud el diseño de los teclados

  Otra característica heredada de las máquinas de escribir es la distribución de las letras en las teclas (QWERTY). Fue diseñada en torno a 1860 y uno de sus objetivos era reducir los atascos mecánicos. Esto se consiguió poniendo las letras más usadas en la parte exterior, y alejando entre sí las parejas que aparecen juntas habitualmente. Al haberse roto estas limitaciones con la electrónica, aparecen opciones que minimizan la distancia media que recorren los dedos y permiten una mayor velocidad de escritura y reducción de cansancio. Las más conocidas son DVORAK y COLEMAK.
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
  Como implican un proceso de adaptación y aprendizaje, no vamos a usarlas de primeras, pero al tener firmware configurable, sería sencillo probarlas.
]

#h[Integraciones][
  Para tener funcionalidad extra al alcance de la mano, la opción más conocida del mercado es el Stream Deck. Es un dispositivo de unas pocas teclas cuyo software permite automatizar tareas como abrir programas, control multimedia o escribir texto. Permite mostrar imágenes en sus botones, ya que son transparentes y tienen una pantalla detrás. Sin embargo, su elevado precio (150€) es un gran factor en contra
  #figure(
    images.elgato,
    caption: [Stream Deck elgato],
  )

  Otra opción popular es AutoHotkey @ahk. Se trata de un programa de código abierto que permite potenciar cualquier teclado convencional, haciendo que combinaciones de teclas causen la ejecución de scripts. Estos scripts utilizan un lenguaje de programación propio que proporciona utilidades como funciones matemáticas, leer y escribir archivos, ejecutar DLLs, movimiento del raton, etc

  En el ámbito de código abierto existe Macro Deck @macro-deck, que permite usar un dispositivo móvil como un panel con varios botones, donde las acciones son similares a las ofrecidas por los anteriores. Sin embargo, no parece tener una comunidad muy extensa ni ha recibido desarrollo en los últimos años
]


#h[Firmware][
  Existen diversos proyectos de código abierto que permiten a aficionados construir sus propios teclados, y programarlos a placer, en vez de depender de dispositivos fabricados en masa por empresas donde las opciones de personalización son escasas o nulas.

  Aunque existen otras alternativas (FAK @fak, KMK @kmk, RMK @rmk, smart-keymap @smart-keymap, ...) vamos a analizar las 2 opciones más completas: QMK @qmk y ZMK @zmk

  En lo que respecta a funcionalidad, QMK tiene una comunidad más grande y madura, por lo que ofrece muchas más funcionalidades "built-in" que ZMK y también resulta más sencillo encontrar código de otros usuarios que implemente ideas que se nos pasen por la cabeza.

  En cuanto a hardware, ambos son compatibles con multitud de @mcu gracias los @rtos que utilizan. ZMK gana en este aspecto dado que usa Zephyr @zephyr, obteniendo acceso a un amplio catálogo @hw-zmk, soportando además @ble para hacer teclados inalámbricos. Por su parte, QMK puede usar ChibiOS @chibios o @lufa-acronym @lufa, lo que también le brinda buena compatibilidad @hw-qmk

  Para configuración, ZMK resulta más sencillo para cambios pequeños (p.ej.: modificar la letra asignada a una tecla), sólo se necesita modificar el archivo en GitHub y se compilará en la nube el nuevo binario que debemos cargar en el dispositivo. Sin embargo, QMK resulta más sencillo para cambios más grandes (p.ej: añadir soporte para un nuevo teclado) ya que su _toolchain_ (compilador, linker, herramientras propias, ...) es mucho más sencillo de instalar y utilizar que el paquete de software usado por ZMK
]
