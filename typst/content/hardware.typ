#import "@preview/big-todo:0.2.0": todo

#import "@elpekenin/tfm:0.1.0": h, images

La elección de @mcu ha sido el RP2040 @rp2040, ya que nos brinda la oportunidad de usar tanto QMK como ZMK. Además, para acelerar el prototipado y evitar posibles fallos de diseño, se ha optado por el RP-Pico #footnote[Se usará un clon con USB-C porque el producto oficial tiene un conector USB-Micro]:
#figure(
  images.rp_pico,
  caption: [Placa Raspberry Pi Pico],
)

Como vemos, esta placa que incorpora @mcu y el hardware para que pueda funcionar (conector USB, memoria flash, cristal, ...) Otro punto positivo es que si se daña algún componente podemos cambiar esta placa fácilmente; incluso se podría reemplazar con un RP-Pico W, que tiene un _pinout_ compatible e incorpora un segundo @mcu (CYW43439) con conectividad WiFi y @ble

Las teclas se van a colocar de forma ortolineal y _split_, eliminando del lateral derecho teclas que rara vez se usan y obteniendo un diseño simétrico con un bloque de 5x6 teclas en cada mano. NOTA: Cada mitad tiene su propia RP-Pico y se comunican entre sí
#figure(
  images.layout,
  caption: [Diseño aproximado del teclado],
)

Elegido el "cerebro" del sistema, pasamos a KiCad @kicad con la meta de que nuestra @pcb use la menor cantidad de @gpio posible. Dejaremos expuestos los restantes junto a líneas de alimentación (GND, 5V, 3.3V), esto permite conectar más elementos sin fabricar una nueva placa. Algunas posibilidades son: un pequeño altavoz, una tarjeta SD para almacenar información o un puente @uart para depurar el código haciendo `print`.
#todo[Imagen debugger conectado]

#h[Cableado de las teclas][
  La opción más sencilla que se puede considerar para conectar varios interruptores al microcontrolador es soldarlos directamente a los @gpio. Para ello, existen dos opciones:
  #figure(
    grid(
      columns: (auto, auto),
      gutter: 1em,
      figure(
        images.pull_down,
        caption: [Pull-Down],
        numbering: none,
        outlined: false,
      ),
      figure(
        images.pull_up,
        caption: [Pull-Up],
        numbering: none,
        outlined: false,
      ),
    ),
    caption: [Cableado directo],
  )

  Sin embargo, este enfoque presenta un problema: pronto necesitaremos un chip con muchos @gpio, o hacer un teclado con pocas teclas debido a la cantidad limitada de estos.

  Para solucionar este inconveniente, podemos cablear los botones mediante una matriz, usando un pin para cada fila y cada columna. Una dimensión serán salidas y la otra entradas, se activan los pines de salida uno a uno y se leen todas las entradas, obteniendo así el estado de cada intersección.
  #figure(
    images.matrix,
    caption: [Cableado en matriz],
  )

  Esta técnica tiene un problema conocido como _ghosting_, con el que se puede detectar como pulsada una tecla que realmente no lo está.
  #figure(
    images.ghosting,
    caption: [Ghosting en una matriz],
  )

  En este ejemplo, podemos ver como la tecla en *(COL0, ROW1)* se va a leer como pulsada debido a la corriente circulando a traves de varios interruptores que están cerrados. La solución consiste en añadir diodos para bloquear este flujo pero permitiendo detectar las pulsaciones.
  #figure(
    images.anti_ghosting,
    caption: [Matriz anti-ghosting],
  )

  Sin embargo, vamos a emplear otra idea, inspirada en el _ghoul_ @ghoul, consiste en el uso de registros de desplazamiento @piso (SN74HC589ADR2G) conectados en serie. Este circuito utiliza unos pocos pines controlar dichos chips y un único pin para leer el estado de las teclas. Lo mejor del diseño es que se pueden seguir añadiendo teclas sin aumentar la cantidad de @gpio necesarios.
]

#h[Pantallas][
  Muchos teclados usan OLED de dos colores (SH1106/SSD1306), que tienen una resolución bastante reducida (128x32 o 128x64 píxeles y alrededor de una pulgada de diagonal).

  En nuestro diseño, vamos a incorporamos pantallas más potentes, donde mostraremos información y que aportarán mayor utilidad.

  Se han elegido la ILI9163 e ILI9341 porque son fáciles de encontrar en el mercado y QMK ya tiene soporte para ellas. La primera es más pequeña y tiene un formato cuadrado, en ella mostraremos pequeños indicadores o imágenes; la segunda tiene mayor resolución y es rectangular y se usará en horizontal para mostrar textos.

  El módulo ILI9341 seleccionado integra un sensor resistivo XPT2046 que nos permite usarla como pantalla táctil. Esto sigue la filosofía de teclados que incorporan joysticks u otros sensores para controlar el cursor desplazar constantemente la mano entre teclado y ratón.
  #figure(
    images.manuform,
    caption: [Teclado _Dactyl Manuform_ con trackball],
  )

  También usaremos una IL91874 puesto que la tenía por casa de experimentos anteriores. QMK no tiene soporte para ella, pero pude encontrar y adaptar drivers por internet. Es un componente antiguo y no tan sencillo de encontrar, pero su tecnología (tinta electrónica) resulta interesante porque solo consume al cambiar el contenido de la pantalla, y sería perfecta para hacer un teclado inalámbrico, por su reducido gasto de batería.

  Todas estas pantallas se controlan mediante el protocolo @spi, con pines adicionales para otras señales de control: @dc, @cs y @rst. Al compartir mismo bus, los @gpio necesarios son:
  - 3 para @spi (@sck, @miso, @mosi)
  - @dc se puede conectar en paralelo, ya que no se puede mandar información a varias pantallas simultáneamente
  - Las señales @cs y @rst son individuales para cada pantalla
  - Adicionalmente, el módulo ILI9341 necesita otra señal @rst, ya que incorpora un sensor táctil (XPT2046)

  Dado que esto consumiría muchos @gpio, usaremos registros de desplazamiento @sipo (SN74HC595) cuyas salidas serán las señales de control de las pantallas. Estos registros se conectarán al mismo bus que los registros que escanean las teclas y solo requieren una conexión extra (su señal @rst). Para los datos enviados a las pantallas necesitaremos un segundo bus @spi, lo cual no supone mayor problema, ya que el RP2040 dispone de dos controladores en hardware para este protocolo.

  Al igual que con el escaneo de teclas, este diseño permite añadir más pantallas (o cualquier otra señal de salida) sin necesidad de emplear más pines del @mcu; simplemente se conectan más registros en serie.
]

#h[Esquema][
  Habiendo elegido los componentes, y habiendo probado la mayoría de ellos en protoboard, procedemos a abrir KiCad.

  Ambas mitades se han diseñado en un único archivo, vamos a ver imágenes del lado izquierdo, por eso habrá un sufijo `_L`(_left_) en muchas de la etiquetas.

  Lo primero será acoplar nuestro RP-Pico con la propia @pcb, para esto, conectamos la alimentación (5V, 3V3 y GND) y seleccionamos los @gpio a usar para controlar los diversos periféricos, los "sobrantes" quedan expuestos
  #figure(
    grid(
      columns: (auto, auto),
      gutter: 1em,
      figure(
        images.mcu,
        numbering: none,
        outlined: false,
      ),
      figure(
        images.gpio,
        caption: [GPIO y alimentación expuestos],
        numbering: none,
        outlined: false,
      ),
    ),
    caption: [Conexiones del RP-Pico],
  )

  La lectura de teclas se realiza configurando un @piso para cada una de las 5 filas, se conectan en serie entre sí, y finalmente al @miso del controlador. Las conexiones son las siguientes:
  - Las entradas que no están conectadas a una tecla se llevan a tierra para que se lean como '0' de forma predecible.
  - Las teclas se conectan a 3V3 en un extremo y a la entrada del registro (con una resistencia _pulldown_ en medio) en el otro; leyendo '1' cuando están pulsadas y '0' cuando no lo están.
  - La salida en serie de cada registro de desplazamiento se conecta a la entrada en serie del siguiente.
    - En el último, se conecta al pin @miso para leer el estado.
  - Las señales @cs, _latch_ y @sck son comunes a todos los integrados.
  - La señal _latch_ resulta ser, a nivel lógico, la inversa de @cs por lo que no necesitamos más pines para ella.
  #figure(
    images.piso,
    caption: [SN74HC589ADR2G para una fila],
  )

  Los @sipo que gestionan las pantallas sólo necesitan las señales clásicas de @spi conectarse (@sck, @mosi, @cs). De nuevo, conectados entre ellos y exponiendo los pines que quedan libres
  #figure(
    images.sipo,
    caption: [SN74HC595 para señales de control],
  )

  Las pantallas incorporan conectores de 2.54mm (los mismos usados en protoboards), solo tendremos que poner un conector hembra en nuestra @pcb con las señales correspondientes.

  Como extra, se han añadido LEDs de colores (WS2812) bajo cada teclas. Solo tienen 4 pines: 5V y GND para alimentación; y un pin de entrada y otro de salida para la señal que los controla, ya que se conectan todos en una cadena
  #figure(
    images.leds,
    caption: [Cadena de LEDs RGB],
  )
]

#h[PCB][
  Hecho nuestro diagrama, procedemos a plasmarlo en una placa. Puesto que el diseño es relativamente sencillo, sólo usamos 2 capas, esto simplifica el modelado y abarata la fabricación. Nos apoyaremos en la librería marbastlib @marbastlib, que contiene huellas y modelos 3D de multitud de componentes comúnmente utilizados en teclados.

  Aunque no se han mostrado en los esquemas, los circuitos integrados irán acompañados de un condensador para estabilizar su alimentación, los veremos cerca de ellos, entre 3V3 y GND.

  He optado por dejar gran parte de los componentes a la vista en la parte superior; el motivo principal de esta decisión es estético, pero también permite que la cara trasera quede prácticamente lisa, con lo que es mucho más sencillo poder fabricar una caja donde montar el teclado, reduciendo además la altura necesaria sobre la mesa.

  Lo primero que ponemos es la RP-Pico y los conectores para modularidad. Como vemos en la imagen, la cara frontal (roja) va a ser un plano de tierra, mientras que la trasera (azul) será 3V3. Esto nos facilita la conexión de todos los componentes a lo largo de la placa
  #figure(
    images.rp_pico_pcb,
    caption: [RP-Pico y conectores],
  )

  Después, colocamos las teclas, que son el elemento principal que define y limita el emplazamiento de piezas. Siguiendo las medidas especificadas por el fabricante, las separaremos 19.05mm. Por cada una tenemos también en la cara opuesta de la @pcb un LED y el recorte por donde asomará
  #figure(
    images.keys_pcb,
    caption: [Teclas y sus LEDs],
  )

  A continuación, conectamos todas las teclas a los registros @sipo que las leen, cada una con su resistencia pull-down. Asimismo, conectamos los registros al bus @spi y entre ellos. Aquí podemos ver como queda el cableado de 2 filas
  #figure(
    images.piso_pcb,
    caption: [Lectura de las teclas],
  )

  Como ya comentamos previamente, para la señal _latch_ tenemos que invertir @cs, esto lo logramos con un MOSFET
  #figure(
    images.inverter,
    caption: [MOSFET como inversor],
  )

  Un detalle que no hemos comentado aún es cómo se comunican las @mcu de ambas mitades. QMK permite la conexión mediante 1 o 2 líneas de datos, hemos optado por la primera opción reduciendo el consumo de pines. Si bien podemos usar cualquier cable para transmitir estas señales, se ha optado por la elección más habitual para este fin: un jack de 3.5mm. Simplemente conectamos el @gpio encargado de enviar y recibir datos, 5V y GND (que alimentarán la segunda placa)
  #figure(
    images.jack,
    caption: [Conexión entre mitades],
  )

  Terminado el diseño, movemos una de las placas para que ocupen el menor área posible y las conectamos con pequeños "puentes" para que se puedan mandar a fabricar como una pieza, más barato que hacerlas por separado. Añadimos con serigrafía un enlace al repositorio donde encontrar el diseño y la versión de la placa (v0.0), útiles a futuro como recordatorio.
  #figure(
    images.pcbs,
    caption: [PCBs terminadas],
  )
]
