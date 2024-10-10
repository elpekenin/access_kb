#import "@preview/glossarium:0.4.2": gls

==== Conexión directa <conexión-directa>
La opción más sencilla que se nos puede ocurrir para conectar diversos interruptores a nuestro microcontrolador es soldarlos directamente a los pines de entrada/salida (GPIO). Para hacer esto tenemos 2 opciones:

#figure(
  grid(
    columns: (auto, auto),
    gutter: 1em,
    figure(
      image("../../../images/pull_down.png"),
      caption: [Pull-Down],
      numbering: none,
      outlined: false,
    ),
    figure(
      image("../../../images/pull_up.png"),
      caption: [Pull-Up],
      numbering: none,
      outlined: false,
    ),
  ),
  caption: [Cableado directo],
)

Si hacemos esto, sin embargo, tendremos un problema pronto porque necesitaremos un chip con muchos pines de entrada/salida, o hacer un teclado con pocas teclas porque la cantidad de GPIOs es reducida.

==== Matriz <matriz>
Para solventar este problema, podemos cablear los botones mediante una matriz, usando un pin para cada fila y columna de teclas. Usamos una dimensión como salida y otra como entrada, haciendo un bucle que aplique voltaje en cada una de las filas y compruebe si las columnas reciben una entrada (tecla pulsada cerrando el circuito).
*Nota*: También se podría iterar en la otra dimensión

#figure(
  image("../../../images/matrix.png", width: 35%),
  caption: [Cableado en matriz],
)

Este diseño también tiene sus problemas, el más notorio es el conocido como "efecto _ghosting_" en el que podemos detectar como pulsada una tecla que no lo está.

#figure(
  image("../../../images/ghosting.png", width: 35%),
  caption: [Ghosting en una matriz],
)

En este ejemplo, la tecla *(1, 1)* se detecta como pulsada de manera correcta pero, al pulsar también las teclas *(0, 1)* y *(0, 0)*, estamos cerrando el circuito y generando que en la columna 1 llegue voltaje a la entrada, que será interpretado como que la tecla *(1, 0)* ha sido pulsada puesto que estamos en la iteración de la fila 0.

Este problema se solventa de forma sencilla, añadiendo unos diodos que bloqueen esta retroalimentación permitiendo detectar *(1, 1)* pero sin la pulsación falsa de *(1, 0)* del caso anterior.

#figure(
  image("../../../images/anti_ghosting.png", width: 45%),
  caption: [Matriz anti-ghosting],
)

Aunque no es muy grave, para matrices muy desiguales, por ejemplo 5x20 teclas, no estamos usando eficazmente los pines, ya que para esas 100 teclas estamos empleando 25 pines mientras que una configuración 10x10 (20 pines) sería suficiente. En este caso, podriamos hacer una distribución de teclas en forma rectangular, pero luego cablearlas como dicha matrix cuadrada, sin embargo el diseño sería bastante más confuso.

Por último hay que tener en cuenta que, aunque el uso de los pines sea más óptimo, seguimos necesitando una cantidad de pines cada vez mayor conforme queramos añadir más teclas, aunque esto no debería ser un factor limitante en la mayoría de casos ya que este límite seguiría permitiendo una cantidad bastante elevada de teclas.

==== Lectura en serie <lectura-en-serie>
La opción que vamos a usar, inspirada en el _ghoul_#cite(<ghoul>), consiste en el uso de registros de desplazamiento conectados en una _daisy chain_, de esta forma vamos a emplear un único pin para leer todas las teclas en una señal serie, y otros pocos pines (unos 3 o 4) para controlar estos chips mediante #gls("spi") o #gls("i2c"). De esta forma, podemos escanear potencialmente cualquier cantidad de teclas sin aumentar el números de pines necesarios, simplemente añadimos más registros a la cadena (aunque el escaneo se iría haciendo más y más lento)
