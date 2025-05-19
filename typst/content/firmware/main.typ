#import "@elpekenin/tfm:0.1.0": cli, snippet

=== Funcionalidad
La mejor parte de usar una librería tan extendida como lo es QMK es que tenemos muchas facilidades a la hora de escribir el codigo ya que buena parte del trabajo está hecho ya.

Esto incluye:
- Gestión sobre USB para reportar distintos endpoints (teclado, ratón, multimedia)
- Drivers para diversos periféricos tales como las pantallas ya comentadas, piezoelectricos para tener feedback sonoro o solenoides para vibración al pulsar las teclas, por nombrar algunos...
- Abstracciones para poder usar la misma API en diversos microcontroladores que internamente utilizan código y hardware muy diferente
- Documentación extensa y detallada
- Servidor ofcial en Discord donde podemos encontrar ayuda

=== Escaneo de teclas
Como se ha comentado en el apartado anterior, vamos a usar registros de desplazamiento, sin embargo QMK tiene soporte para cableado directo, en matrices y usando algunos otros circuitos, como _IO expanders_. Sin embargo este no es el caso para los registros por lo que tendemos que escribir un poco de código para leer su información.

=== Pantallas
QMK tiene una API estandarizada (Quantum Painter @qp) para primitivas de dibujo en interfaces gráficas. Aún es "joven" y no soporta demasiadas pantallas, pero hace gran parte del trabajo por nosotros.

=== Pantalla táctil
QMK también tiene una capa de abstracción @pointing para diversos sensores que permiten mover el cursor, sin embargo todos se basan en medir desplazamientos y no una coordenada como es el caso de la pantalla táctil, por tanto en este caso nos vemos obligados a diseñar una arquitectura de software nueva, para poder trabajar con este otro tipo
de datos de entrada.

/** -- TODO: Keep revising/re-writing -- */

=== Instalación y configuración de QMK
Para usar QMK instalamos su CLI ejecutando `pip install --user qmk`. Después, usamos `qmk setup` para descargar las herramientas necesarias y el código fuente, a este comando le podemos pasar como parámetro nuestro fork del repositorio (en mi caso `elpekenin/qmk_firmware`), para poder usar git, ya que no tendremos permisos en el repositorio oficial. Este comando también nos instalará los compiladores necesarios y comprobará las udev de nuestro sistema Linux, para que podamos trabajar con los dipositivos sin problema, en caso de que no estén bien configuradas podemos usar copiar el archivo en . Finalmente podemos ejecutar para comprobar el estado de QMK. Para poder hacer debug con he necesitado añadir una línea a dicho archivo:

Para acelerar el proceso de compilado podemos guardar nuestra configuración (teclado y keymap) y el número de hilos que usará el compilador

#cli(
  ```bash
  $ qmk config user.keyboard=elpekenin/access
  $ qmk config user.keymap=default
  $ qmk config compile.parallel=20
  ```,
  caption: [Personalizar QMK],
)

=== Driver para pantalla ili9486
Para hacer pruebas antes de diseñar y fabricar la PCB, he usado un módulo que integra una pantalla, sensor táctil y lector de tarjeta SD. Sin emabargo, QMK no tiene soporte para este modelo concreto de pantalla. Por tanto, usando como base el código @ili9488 para otro dispositivo de la misma familia, y con ayuda de los usuarios \@sigprof y \@tzarc he desarrollado un driver @ili9486-pr para la pantalla usada.

Los cambios se reducen a dos cosas:
- Cambiar la fase de inicialización de la pantalla, que configura valores como el formato en el que se envían los píxeles a mostrar
- Puesto que la pantalla contiene un circuito que convierte la línea SPI en una señal paralela de 16 bits que se envía a la pantalla, mediante registros de desplazamiento, tendremos problemas al enviar información de un tamaño que no sea múltiplo de 16, por tanto hacemos un par de cambios al código "normal".

=== Dibujar por USB
Lo siguiente que necesitamos resolver es controlar la pantalla desde el ordenador, de forma que podamos dibujar en ella desde nuestro software, para esto usaremos la funcionalidad XAP @xap (aún en desarrollo), que nos proporciona un nuevo endpoint HID @hid sobre el bus USB y un protocolo @xap-specs que podemos extender. Ahora podemos definir nuestros mensajes simplemente creando un archivo @xap-custom-msg Una vez creados los mensajes, añadimos el código @qp-xap-handlers que se ejecuta cuando los recibamos

=== Mejora algoritmo elipses
Durante las pruebas con el script anterior, vi que el algoritmo para dibujar elipses funcionaba regular, por lo que hice un _refactor_ del mismo. Es decir, mantenemos la misma "forma" (atributos que se reciben y valor devuelto), pero la implementación @ellipse-pr es compleamente diferente. Sigue sin ser perfecto, pero parece funcionar mejor.

=== Driver para sensor XPT2046
Al igual que hemos necesitado un driver (código que nos permite comunicarnos con el hardware) para dibujar en la pantalla, necesitamos otro para poder leer la posición en la que esta ha sido pulsada. Por desgracia, QMK no tiene ninguna característica parecida, por lo que usaremos el código @touch-driver que he escrito para ello desde 0.

Este hardware, al igual que la pantalla, también tiene una particularidad, y es que la función proporcionada por QMK, realmente es un intercambio de información en vez de solo lectura, esto es debido a que SPI es un protocolo síncrono. El problema aparece porque QMK envía todos los bits a 1 -dado que algunos microcontroladores hacen esto por hardware y no se puede cambiar- para poder recibir información, *sin embargo*, el XPT2046 (a diferencia de la mayoria de chips) utiliza esta información que recibe mientras que nos reporta su información para cambiar su configuración. De forma, que en caso de que los 2 últimos bits que enviamos no tengan el valor adecuado, no podremos usar el sensor. Lo bueno es que la solución es tan simple como usar para enviar un byte vacío y recibir la respuesta dejando una configuración conveniente.

=== Generar imágenes
Dado que vamos a usar varios iconos para hacer la interfaz gráfica en la pantalla, he cogido la colección de iconos Material Design Icons @templarian-mdi y he convertido todos sus iconos al formato que utiliza QMK para representar imágenes. Las imágenes originales, scripts usados para la conversión y los archivos en formato QGF@qgf se pueden encontrar en este repositorio @mdi-qgf

=== Añadir nuestro código
Para poder usar nuestros nuevos archivos, no es suficiente con añadir un sino que también debemos indicarle a las herramientas de compilado que "busquen" archivos en la carpeta donde tenemos el código, en este caso la carpeta base del teclado, asi como subcarpeta . También es necesario, no tengo muy claro por qué, añadir los archivos de las imagenes QGF o no podremos usarlos. Este archivo se complica un poco porque no debemos incluir algunos archivos si no están algunas opciones habilitadas, por ejemplo, no debemos añadir las imágenes si no tenemos la opción de usar la pantalla habilitada. Esto lo hacemos con el archivo @rules-mk
