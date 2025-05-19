#import "@elpekenin/tfm:0.1.0": images

La opción más sencilla que se nos puede ocurrir para conectar diversos interruptores a nuestro microcontrolador es soldarlos directamente a los pines de entrada/salida (GPIO). Para hacer esto tenemos 2 opciones:

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

Si hacemos esto, sin embargo, tendremos un problema pronto porque necesitaremos un chip con muchos pines de entrada/salida, o hacer un teclado con pocas teclas porque la cantidad de GPIOs es reducida.

Para solventar este problema, podemos cablear los botones mediante una matriz, usando un pin para cada fila y columna de teclas. Usamos una dimensión como salida y otra como entrada, haciendo un bucle que aplique voltaje en cada una de las filas y compruebe si las columnas reciben una entrada (tecla pulsada cerrando el circuito).

#figure(
  images.matrix,
  caption: [Cableado en matriz],
)

Este diseño también tiene sus problemas, el más notorio es el conocido como "efecto _ghosting_" en el que podemos detectar como pulsada una tecla que no lo está.

#figure(
  images.ghosting,
  caption: [Ghosting en una matriz],
)

En este ejemplo, la tecla *(1, 1)* se detecta como pulsada de manera correcta pero, al pulsar también las teclas *(0, 1)* y *(0, 0)*, estamos cerrando el circuito y generando que en la columna 1 llegue voltaje a la entrada, que será interpretado como que la tecla *(1, 0)* ha sido pulsada puesto que estamos en la iteración de la fila 0.

Este problema se solventa de forma sencilla, añadiendo unos diodos que bloqueen esta retroalimentación permitiendo detectar *(1, 1)* pero sin la pulsación falsa de *(1, 0)* del caso anterior.

#figure(
  images.anti_ghosting,
  caption: [Matriz anti-ghosting],
)

La opción que vamos a usar, inspirada en el _ghoul_ @ghoul, consiste en el uso de registros de desplazamiento PISO (SN74HC589ADR2G) conectados en serie, de esta forma vamos a emplear un único pin para leer el estado de las teclas, y otros pocos pines para controlar dichos chips. Lo mejor de este diseño es que la cantidad de pines es constante, por lo que se podrían añadir más teclas sin necesidad de mas GPIO. Las conexiónes nos quedan
- Las entradas que no están conectadas a una tecla las llevamos a tierra para que se lean como '0'
- Las teclas se conectan a VCC en un extremo y a la entrada (con una resistencia _pulldown_) en el otro. De esta forma leeremos '1' cuando estén pulsadas y '0' cuando no lo estén
- La salida serie de cada registro de desplazamiento se conecta a la entrada serie del siguiente
  - El último se conecta a MISO (entrada SPI del microcontrolador)
- Las señales CS, Latch y CLK son comunes a todos los integrados
- Como la señal Latch resulta ser la inversa de CS, en vez de usar otro pin, invertimos el voltaje con un MOSFET

#figure(
  images.inverter,
  caption: [MOSFET como inversor],
)

#figure(
  images.piso,
  caption: [SN74HC589ADR2G para una fila],
)
