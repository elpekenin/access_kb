#import "@elpekenin/tfm:0.1.0": h, images

Las teclas se van a colocar de forma ortolineal y _split_, eliminando del lateral derecho teclas que rara vez se usan y obteniendo un diseño simétrico con un bloque de 5x6 teclas en cada mano.
#figure(
  images.layout,
  caption: [Distribución de las teclas],
)

La elección de MCU ha sido el RP2040 @rp2040, ya que nos brinda la oportunidad de usar tanto QMK como ZMK. Además, para acelerar el prototipado y evitar posibles fallos de diseño, se ha optado por el RP-Pico #footnote[Se usará un clon con USB-C porque el producto oficial tiene un conector USB-Micro], esta placa incorpora MCU y el hardware para que pueda funcionar (conector USB, memoria flash, cristal, ...).

También implica que si se daña alguno de sus componentes, podemos cambiar esta placa fácilmente; incluso con un RP-Pico *W*, que tiene un _pinout_ compatible e incorpora un segundo MCU (CYW43439) con conectividad WiFi y BLE
#figure(
  images.rp_pico,
  caption: [Placa Raspberry Pi Pico],
)

El objetivo será usar la menor cantidad de GPIO posible para poder exponer los restantes junto a líneas de alimentación (GND, 5V, 3V3), permitiendo conectar más elementos a la placa. Algunas ideas podrían ser un pequeño altavoz, una tarjeta SD para almacenar información o un puente UART-USB para depurar el código imprimiendo mensajes.

#h[Cableado de las teclas][
  #block(breakable: false)[
    La opción más sencilla sería soldar directamente a los GPIO. Para ello, existen dos opciones:
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
  ]

  Sin embargo, este enfoque presenta un problema; pronto necesitaremos un chip con muchos GPIO -o hacer un teclado con pocas teclas- debido a la cantidad limitada de estos.

  Para solucionar este inconveniente, se puede usar una matriz, con un pin para cada fila y columna. Las filas se activan una a una en bucle y se leen todas las columnas, obteniendo así el estado de cada intersección (o viceversa).
  #figure(
    images.matrix,
    caption: [Cableado en matriz],
  )

  Esta técnica tiene un problema conocido como _ghosting_, con el que se puede detectar como pulsada una tecla que realmente no lo está.
  #figure(
    images.ghosting,
    caption: [Ghosting en una matriz],
  )

  En la imagen vemos como la tecla en *(COL0, ROW1)* se va a leer como pulsada debido a la corriente circulando a traves de varios interruptores que están cerrados. La solución consiste en añadir diodos para bloquear este flujo pero permitiendo detectar las pulsaciones.
  #figure(
    images.anti_ghosting,
    caption: [Matriz anti-ghosting],
  )

  En nuestro diseño empleamos otra idea, inspirada en el _ghoul_ @ghoul, que consiste en usar registros de desplazamiento PISO SN74HC589ADR2G conectados en serie. Este circuito utiliza unos pocos pines controlar dichos chips y un único pin para leer el estado de las teclas. Lo mejor del diseño es que se pueden seguir añadiendo teclas sin aumentar la cantidad de GPIO necesarios.
]

#h[Pantallas][
  Muchos teclados usan OLED de dos colores (SH1106/SSD1306), que tienen una resolución bastante reducida: 128x32 o 128x64 píxeles, alrededor de una pulgada de diagonal.

  En nuestro diseño, vamos a inclusir pantallas más potentes, donde mostraremos información y que aportarán mayor utilidad. Se han elegido la ILI9163 (más pequeña y cuadrada) e ILI9341 (mayor tamaño, rectangular) porque son fáciles de encontrar en el mercado y QMK tiene soporte para ellas. Además, el módulo elegido para la segunda posee un sensor resistivo XPT2046 que nos permite usarla como pantalla táctil.

  También usaremos una IL91874 dado que la tenía por casa de experimentos anteriores. Es un componente antiguo y no tan sencillo de encontrar, pero su tecnología (tinta electrónica) es interesante porque solo consume energía al cambiar sus contenidos y sería perfecta para hacer un teclado inalámbrico, por su reducido gasto de batería.

  Todas estas pantallas se controlan mediante el protocolo SPI, con pines adicionales para otras señales de control: DC, CS y RST. Al compartir mismo bus, los GPIO necesarios son:
  - 3 compartidos para SPI (SCK, MISO, MOSI)
  - DC se puede conectar en paralelo, no mandamos información a varias pantallas simultáneamente
  - Las señales CS y RST son individuales para cada pantalla
  - El módulo ILI9341 necesita otra señal RST para el sensor táctil

  Dado que esto consumiría muchos GPIO, usaremos registros de desplazamiento sipo (SN74HC595) cuyas salidas serán las señales de control de las pantallas. Estos registros se conectarán al mismo bus que los registros que escanean las teclas y solo requieren una conexión extra (su señal RST).

  Para los datos enviados a las pantallas necesitaremos un segundo bus SPI, lo cual no supone mayor problema, ya que el RP2040 dispone de dos controladores en hardware para este protocolo.

  Al igual que con el escaneo de teclas, este diseño permite añadir más pantallas (o cualquier otra señal de salida) sin necesidad de emplear más pines del MCU; simplemente se conectan más registros en serie.
]

#h[Esquema][
  Habiendo elegido los componentes y hecho pruebas con la mayoría de ellos en protoboard, procedemos a abrir KiCad @kicad para plasmar nuestro circuito.

  Ambas mitades se han diseñado en un único archivo, vamos a ver imágenes del lado izquierdo, por eso habrá un sufijo "\_L" (_left_) en muchas de la etiquetas.

  Lo primero será acoplar nuestro RP-Pico a la propia PCB, para esto, conectamos la alimentación (5V, 3V3 y GND) y los GPIO que controlan los diversos periféricos.
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
        caption: [Resto de GPIO expuestos],
        numbering: none,
        outlined: false,
      ),
    ),
    caption: [Conexiones del RP-Pico],
  )

  La lectura de teclas se realiza configurando un PISO para cada una de las 5 filas, se conectan en serie entre sí, y finalmente al pin MISO del controlador. Las conexiones son las siguientes:
  - Las entradas que no están conectadas a una tecla se llevan a tierra para que se lean como "0" de forma predecible.
  - Las teclas se conectan a 3V3 en un extremo y a la entrada del registro (con una resistencia _pulldown_ en medio) en el otro; leyendo "1" cuando están pulsadas y "0" cuando no lo están.
  - La salida en serie de cada registro de desplazamiento se conecta a la entrada en serie del siguiente.
    - En el último, se conecta al pin MISO de la MCU para leer el estado.
  - Las señales CS, _latch_ y SCK son comunes a todos los integrados.
  - La señal _latch_ resulta ser la inversa de CS por lo que no necesita un pin dedicado
  #figure(
    images.piso,
    caption: [SN74HC589ADR2G para una fila],
  )

  Los SIPO que gestionan las pantallas solo necesitan las señales estándar de SPI (SCK, MOSI, CS), se conectan en serie y expondremos los pines sobrantes
  #figure(
    images.sipo,
    caption: [SN74HC595 para señales de control],
  )

  Las pantallas incorporan conectores de 2.54mm (los mismos usados en protoboards), solo tendremos que poner un conector hembra en nuestra PCB con las señales correspondientes.

  #block(breakable: false)[
    Se añadieron LED RGB (WS2812) bajo cada tecla, estos chip también se conectan en serie. Cada uno usa 4 pines: 5V, GND, entrada y salida de la señal de datos.
    #figure(
      images.leds,
      caption: [Cadena de LEDs RGB],
    )
  ]
]

#h[PCB][
  Con el diagrama hecho, pasamos a darle forma a la placa. Puesto que el diseño es relativamente sencillo, sólo usamos 2 capas, simplificando el modelado y reduciendo los costes de fabricación. Nos apoyaremos en la librería marbastlib @marbastlib, que contiene huellas y modelos 3D de multitud de componentes comúnmente utilizados en teclados.

  Aunque no se han mostrado en los esquemas, cada circuito integrado irá acompañado de un condensador para estabilizar su alimentación, los veremos cerca de ellos entre 3V3 y GND.

  He optado por dejar gran parte de los componentes a la vista en la parte superior; el motivo principal de esta decisión es estético, pero también permite que la cara trasera quede prácticamente lisa, con lo que es mucho más sencillo poder fabricar una caja donde montar el teclado.

  #block(breakable: false)[
    Lo primero que ponemos es la RP-Pico, como se ve en la imagen, la cara frontal va a ser un plano de tierra, mientras que la trasera 3V3. Esto facilita la conexión de los componentes a lo largo de la placa
    #figure(
      images.rp_pico_pcb,
      caption: [RP-Pico y conectores],
    )
  ]

  #block(breakable: false)[
    Después, colocamos las teclas, que son el elemento principal que define y limita la posición de las piezas. Siguiendo las medidas especificadas por el fabricante, las separaremos 19.05mm. Por cada una tenemos también en la cara opuesta de la PCB un LED y el recorte por donde asomará
    #figure(
      images.keys_pcb,
      caption: [Teclas y sus LEDs],
    )
  ]

  #block(breakable: false)[
    A continuación, conectamos todas las teclas a los registros sipo que las leen, cada una con su resistencia pull-down. Asimismo, conectamos los registros al bus SPI y entre ellos. Aquí podemos ver como queda el cableado de 2 filas
    #figure(
      images.piso_pcb,
      caption: [Lectura de las teclas],
    )
  ]

  #block(breakable: false)[
    Como ya comentamos previamente, para la señal _latch_ tenemos que invertir CS, esto lo logramos con un MOSFET
    #figure(
      images.inverter,
      caption: [MOSFET como inversor],
    )
  ]

  #block(breakable: false)[
    Para comunicar las MCU de ambas mitades podríamos usar cualquier cable, se ha optado por la elección más habitual para este fin: un jack de 3.5mm. Simplemente conectamos el GPIO encargado de enviar y recibir datos, 5V y GND (que alimentarán la segunda placa)
    #figure(
      images.jack,
      caption: [Conexión entre mitades],
    )
  ]

  Juntamos las placas para que reducir el área que ocupan. Las conectamos con pequeños "puentes" con _mousebits_ para mandarlas a fabricar como una pieza, más ecónomico que hacerlas por separado.
  #figure(
    images.pcbs,
    caption: [Diseño terminado],
  )

  Con el diseño terminado, generamos los arhivos gerber para su fabricación y los mandamos a fábrica. Unas semanas después llega a casa el resultado, y sólo resta soldar los componentes para tener un teclado funcional.
  #figure(
    images.pcb,
    caption: [Placa fabricada],
  )
]
