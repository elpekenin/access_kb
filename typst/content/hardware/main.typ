#import "@elpekenin/tfm:0.1.0": images

La meta principal al diseñar la PCB ha sido usar la menor cantidad de pines posible. Dejando expuesto todos los pines restantes y alimentación (GND, 5V, 3.3V) para poder añadir más elementos (por ejemplo, un buzzer o motor de vibración para feedback) o incluso servir de placa de pruebas para desarrollar drivers. Esta funcionalidad también ha sido útil para realizar depuración de errores, enviando mensajes mediante UART por un par de pines.

Respecto a la disposición de las teclas, he optado por ortolineal y split, eliminando varias teclas del lateral derecho que en un teclado convencional raramente se usan, de esta forma obtenemos una distribución de teclas simétrica, que es más sencilla de diseñar. La implementación actual del firmware es QWERTY para aprovechar la memoria muscular, pero podría editarse fácilmente mediante cambios al firmware

#figure(
  images.layout,
  caption: [Diseño aproximado del teclado],
)


== Cableado de las teclas
#include "scanning.typ"

== Pantallas
#include "screens.typ"

== Sensor táctil
#include "touch.typ"
