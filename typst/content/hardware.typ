#import "@preview/big-todo:0.2.0": todo

#import "@elpekenin/tfm:0.1.0": config, images

La meta principal al diseñar la PCB ha sido utilizar la menor cantidad de pines posible, dejando expuestos todos los pines restantes y las líneas de alimentación (GND, 5V, 3.3V) para poder añadir más elementos, como un buzzer o un motor de vibración para retroalimentación, o incluso servir de placa de pruebas para desarrollar drivers. Esta funcionalidad también ha resultado útil para depurar errores, enviando mensajes mediante UART por un par de pines.

Respecto a la disposición de las teclas, he optado por un diseño ortolineal y split, eliminando varias teclas del lateral derecho que en un teclado convencional rara vez se usan. De esta forma, se obtiene una distribución de teclas simétrica, más sencilla de diseñar. La implementación actual del firmware es QWERTY para aprovechar la memoria muscular, pero podría modificarse fácilmente mediante cambios en el firmware.

#figure(
  images.layout,
  caption: [Diseño aproximado del teclado],
)

== Cableado de las teclas
La opción más sencilla que se puede considerar para conectar varios interruptores al microcontrolador es soldarlos directamente a los pines de entrada/salida (GPIO). Para ello, existen dos opciones:

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

Sin embargo, este enfoque presenta un problema: pronto necesitaremos un chip con muchos pines de entrada/salida, o bien hacer un teclado con pocas teclas debido a la cantidad limitada de GPIOs.

Para solucionar este inconveniente, podemos cablear los botones mediante una matriz, usando un pin para cada fila y columna de teclas. Se utiliza una dimensión como salida y otra como entrada, realizando un bucle que aplica voltaje en cada una de las filas y comprueba si las columnas reciben una entrada (tecla pulsada cerrando el circuito).

#figure(
  images.matrix,
  caption: [Cableado en matriz],
)

Este diseño también tiene sus inconvenientes, siendo el más notorio el conocido como "efecto _ghosting_", en el que se puede detectar como pulsada una tecla que realmente no lo está.

#figure(
  images.ghosting,
  caption: [Ghosting en una matriz],
)

En este ejemplo, la tecla *(1, 1)* se detecta como pulsada de manera correcta, pero al pulsar también las teclas *(0, 1)* y *(0, 0)*, se cierra el circuito y se genera que en la columna 1 llegue voltaje a la entrada, lo que será interpretado como que la tecla *(1, 0)* ha sido pulsada, ya que estamos en la iteración de la fila 0.

Este problema se soluciona fácilmente añadiendo diodos que bloqueen esta retroalimentación, permitiendo detectar *(1, 1)* pero evitando la pulsación falsa de *(1, 0)* del caso anterior.

#figure(
  images.anti_ghosting,
  caption: [Matriz anti-ghosting],
)

La opción que vamos a emplear, inspirada en el _ghoul_ @ghoul, consiste en el uso de registros de desplazamiento PISO (SN74HC589ADR2G) conectados en serie. De esta forma, se utiliza un único pin para leer el estado de las teclas y unos pocos pines adicionales para controlar dichos chips. Lo mejor de este diseño es que la cantidad de pines es constante, por lo que se pueden añadir más teclas sin necesidad de más GPIO. Las conexiones quedan así:
- Las entradas que no están conectadas a una tecla se llevan a tierra para que se lean como '0'.
- Las teclas se conectan a VCC en un extremo y a la entrada (con una resistencia _pulldown_) en el otro. Así, se lee '1' cuando están pulsadas y '0' cuando no lo están.
- La salida serie de cada registro de desplazamiento se conecta a la entrada serie del siguiente.
  - El último se conecta a MISO (entrada SPI del microcontrolador).
- Las señales CS, Latch y CLK son comunes a todos los integrados.
- Como la señal Latch resulta ser la inversa de CS, en vez de usar otro pin, se invierte el voltaje con un MOSFET.

#figure(
  images.inverter,
  caption: [MOSFET como inversor],
)

#figure(
  images.piso,
  caption: [SN74HC589ADR2G para una fila],
)

== Sensor táctil
En tiempos recientes, es cada vez más común encontrar diseños que incluyen diferentes dispositivos como joysticks o sensores de ratón para poder mover el cursor por la pantalla sin necesidad de desplazar la mano hasta el ratón. Un ejemplo de esta idea es el Dactyl[@img:dactyl].

En nuestro caso, vamos a aprovechar el uso de pantallas y hacer que una de ellas sea táctil. Con pulsaciones o deslizamientos sobre ella, podremos mover el cursor o controlar ajustes en el teclado (por ejemplo, implementando "botones" en la pantalla).

== Pantallas
Otra tendencia en los diseños es la incorporación de displays; generalmente se usan OLED de dos colores (SH1106/SSD1306), que tienen una resolución bastante reducida (128x32 o 128x64 píxeles y alrededor de una pulgada de diagonal).

En nuestro diseño, incorporamos pantallas más potentes, donde mostraremos información y que aportarán mayor utilidad. En la mitad izquierda habrá una pantalla de tinta electrónica (IL91874), en la que se mostrará información estática (como las funcionalidades disponibles o la fecha de compilación); en el lado derecho habrá dos pantallas LCD (ILI9163 e ILI9341) para información dinámica.

Todas estas pantallas se controlan mediante el protocolo SPI, con pines adicionales para otras señales de control (DC, CS, RST). Al estar en el mismo bus, los pines necesarios son:
- 3 para SPI (SCK, MISO, MOSI)
- DC se puede conectar en paralelo, ya que no se puede mandar información a varias pantallas simultáneamente
- Las señales CS y RST son individuales para cada pantalla
- Adicionalmente, el módulo ILI9341 necesita otra señal CS, ya que incorpora un sensor táctil (XPT2046)

Dado que esto consumiría muchos pines, usaremos registros de desplazamiento SIPO (SN74HC595), cuyas salidas serán las señales de control de las pantallas. Estos registros se conectarán al mismo bus que los PISO que escanean las teclas y solo requieren una conexión extra (su señal CS). Para los datos enviados a las pantallas necesitaremos un segundo bus SPI, lo cual no supone mayor problema, ya que el RP2040 dispone de dos controladores hardware para dicho protocolo.

De forma análoga al escaneo de teclas, con este diseño podríamos añadir más pantallas (o cualquier otra señal de salida) sin necesidad de emplear más pines del MCU; simplemente se conectan más registros en serie.

#if (config.render_todos) {
  todo("Adjuntar imágenes del diseño", inline: true)
}
