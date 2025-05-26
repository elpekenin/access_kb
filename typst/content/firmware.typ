#import "@preview/big-todo:0.2.0": todo

#import "@elpekenin/tfm:0.1.0": cli, snippet, config

Gracias al uso de un framework que proporciona las bases para crear el firmware del teclado, solo tendremos que desarrollar los aspectos específicos para nuestro hardware, detallados en esta sección. Para que este código adicional se incluya en el binario final, será necesario modificar la configuración de `make` para indicar la ubicación de nuestros archivos o añadir opciones extra al compilador, lo cual se explicará en las siguientes secciones.

=== Instalación
Para poder usar QMK, instalamos su CLI y la utilizamos para clonar el repositorio de código fuente e instalar los compiladores y herramientas necesarios. Después, también podemos comprobar la instalación:
#cli(
  ```bash
  $ pip install --user qmk
  $ qmk setup
  $ qmk doctor
  ```,
  caption: [Instalación de QMK],
)

Opcionalmente, podemos configurar el número de hilos que usará el compilador para acelerar el desarrollo:
#cli(
  ```bash
  $ qmk config compile.parallel=20
  ```,
  caption: [Personalizar QMK],
)

=== Multi-bus SPI
Dado que QMK no proporciona soporte para configurar varios buses SPI al mismo tiempo, es necesario adaptar el código para ello. Se realiza una copia del archivo `platforms/chibios/drivers/spi_master.c`, modificando las variables existentes para convertirlas en arrays, de forma que puedan almacenar el estado de los distintos buses en vez de una sola configuración.

Para compilar este nuevo archivo, añadimos `SRC += spi_custom.c` a `rules.mk` y creamos una cabecera `spi_custom.h` para que los módulos que utilicen esta funcionalidad conozcan las funciones disponibles en nuestro código.

=== Escaneo de teclas
Como hemos visto anteriormente, usaremos registros de desplazamiento para la lectura de teclas. Dado que QMK no provee soporte para esta configuración, escribimos un poco de código para configurar el bus SPI y usarlo para escanear las teclas.
#snippet(
  ```c
  #include "spi_custom.h"

  // inicialización
  void matrix_init_custom(void) {
    gpio_set_pin_output(CS_PIN);
    gpio_write_pin_high(CS_PIN);
    spi_custom_init(SPI_BUS_ID);
  }

  // buffer donde trabajamos
  static matrix_row_t scan[N_REGISTERS] = {0};

  // escaneo
  bool matrix_scan_custom(matrix_row_t *output) {
    // inicia una transmisión SPI
    if (!spi_custom_start(CS_PIN, false, SPI_MODE, SPI_DIVISOR, SPI_BUS_ID)) {
        return false;
    }

    // leer de los registros
    spi_custom_receive((uint8_t *)scan, N_REGISTERS, SPI_BUS_ID);
    spi_custom_stop(SPI_BUS_ID);

    // comparamos si alguna tecla ha cambiado
    bool changed = memcmp(scan, output, N_REGISTERS) != 0;
    if (changed) {
        // de ser así, escribimos el valor en el puntero recibido
        memcpy(output, scan, N_REGISTERS);
    }

    // al indicar si ha cambiado alguna tecla, QMK sabrá si debe ejecutar
    // el código encargado de encontrar y procesar el cambio
    return changed;
  }
  ```,
  caption: [Lectura de teclas],
)

=== Pantallas
QMK tiene una API estandarizada (Quantum Painter @qp) para primitivas de dibujo en interfaces gráficas. Esto facilita mucho el desarrollo, ya que disponemos de un conjunto de funciones para dibujar puntos, líneas, ... además de drivers para multitud de dispositivos.

Las pantallas ILI9163 e ILI9341 ya están soportadas, por lo que no tenemos que hacer nada para poder usarlas; sin embargo, para la IL91874 tendremos que escribir el driver para que QMK pueda comunicarse con ella y controlarla.

Dado que esta es una pantalla donde cada píxel puede ser blanco, negro o rojo, el formato de datos que utiliza internamente es de 2 bits por píxel. Así, si el bit que controla el color está a "1", tendremos un píxel rojo y, en caso contrario, el color vendrá determinado por el segundo bit, siendo "0" blanco y "1" negro. Además, a diferencia de otros dispositivos más modernos, esta pantalla no permite refrescar regiones aisladas, sino que requiere enviar el contenido completo cada vez que vayamos a dibujar.

Debido a estas características, el código para interactuar con esta pantalla consiste en dos framebuffers donde almacenamos por separado el estado del bit rojo y el bit blanco/negro. Por suerte, QMK ya dispone de utilidades que permiten interactuar con un buffer usando la misma API de dibujo. Por tanto, nuestro código será otra capa de utilidad usando dos de estas "pantallas virtuales". Las partes más relevantes del código se adjuntan a continuación:
#snippet(
  ```c
  // calcula la distancia entre 2 colores HSV usando Pitágoras
  // sin potencias para acelerar el cálculo
  static inline uint16_t hsv_distance(HSV a, HSV b) {
      return abs(a.h - b.h) + abs(a.s - b.s) + abs(a.v - b.v);
  }

  // ... más adelante ...
  for (int16_t i = 0; i < palette_size; ++i) {
      HSV hsv = (HSV){palette[i].hsv888.h, palette[i].hsv888.s, palette[i].hsv888.v};

      // calculamos la distancia hacia las 3 posibles salidas
      //
      // el código está preparado para manejar otros colores
      // por ejemplo: pantallas blanco/negro/amarillo
      uint16_t white = hsv_distance(hsv, (HSV){HSV_WHITE});
      uint16_t black = hsv_distance(hsv, (HSV){HSV_BLACK});
      uint16_t color = hsv_distance(hsv, driver->color);

      // equivalente a blanco
      bool black = false;
      bool color = false;

      // según cuál es la menor de las distancias
      uint32_t min = QP_MIN(white, QP_MIN(black, color));
      if (min == black) {
          black = true;
      } else if (min == color) {
          color = true;
      }

      uint8_t converted = (black << 1) | (color << 0);

      // almacenamos en el buffer de salida los 2 bits de este color
      //
      // código preparado para invertir los bits
      // en pantallas que representen con otro formato
      palette[i].mono = converted ^ driver->invert_mask;
  }
  ```,
  caption: [Conversión del espacio de colores],
)

#snippet(
  ```c
  for (uint32_t i = 0; i < pixel_count; ++i) {
      uint32_t pixel_num = pixel_offset + i;

      // cada pixel son 2 bits, cada byte son 4 píxeles
      // calculamos el offset de este pixel
      uint32_t byte_offset  = pixel_num / 4;
      uint8_t  pixel_offset = 3 - (pixel_num % 4);

      // leemos el color previamente convertido
      bool black_bit = palette[palette_indices[i]].mono & 0b10;
      bool color_bit = palette[palette_indices[i]].mono & 0b01;

      // cada byte almacena: B0C0B1C1B2C2B3C3
      // donde: Bx es un bit de blanco/negro
      //        Cx es un bit de color
      //
      // calculamos los bitmask equivalentes
      uint8_t black_mask = 1 << (2 * pixel_offset + 1);
      uint8_t color_mask = 1 << (2 * pixel_offset + 0);

      // ponemos los bits a 0 o 1 según el color en la paleta
      if (black_bit) {
          target_buffer[byte_offset] |= black_mask;
      } else {
          target_buffer[byte_offset] &= ~black_mask;
      }

      if (color_bit) {
          target_buffer[byte_offset] |= color_mask;
      } else {
          target_buffer[byte_offset] &= ~color_mask;
      }
  }
  ```,
  caption: [Añadir píxeles al buffer de trabajo],
)

#snippet(
  ```c
  while (i < native_pixel_count) {
      // como mucho, extraemos 8 píxeles por iteración
      uint8_t  pixels_this_loop = QP_MIN(native_pixel_count - i, 8);
      uint32_t byte             = i / 4;

      // función que extrae información de los píxeles
      // separa color y blanco/negro, el buffer los tiene mezclados
      decode_masked_pixels(pixels, byte, &black_data, &color_data);

      // como habíamos adelantado, el trabajo se resume en "reenviar"
      // a cada buffer (surface), le mandamos la información de su color
      ret  = qp_pixdata(driver->black_surface, &black_data, pixels_this_loop);
      ret &= qp_pixdata(driver->color_surface, &color_data, pixels_this_loop);

      // si alguna operación falla, detenemos el bucle reportando error
      if (!ret) {
          qp_dprintf("qp_eink_panel_pixdata: something went wrong, quitting\n");
          return false;
      }

      // actualizamos estado del bucle
      i += pixels_this_loop;
  }
  ```,
  caption: [Traspaso de información a los buffers],
)

#snippet(
  ```c
  // enviamos los dos canales de color
  qp_comms_command(device, vtable->opcodes.send_black_data);
  qp_comms_send(device, black->buffer, n_bytes);

  qp_comms_command(device, vtable->opcodes.send_color_data);
  qp_comms_send(device, color->buffer, n_bytes);

  // indicamos a la pantalla que dibuje la información recibida
  qp_comms_command(device, vtable->opcodes.refresh);

  // bloqueamos la posibilidad de dibujar durante un tiempo
  // esto se debe a que el datasheet indica posibles daños
  qp_eink_update_can_flush(device);

  // limpiamos el estado interno de los 2 framebuffers
  qp_flush(driver->black_surface);
  qp_flush(driver->color_surface);
  ```,
  caption: [Envío a pantalla],
)

=== Pantalla táctil
Necesitaremos un driver para comunicarnos con el hardware (XPT2046) y obtener la posición en la que se ha pulsado la pantalla. QMK no tiene ninguna característica parecida, por lo que usaremos el código @touch-driver que he escrito para ello desde cero.

Cabe destacar una particularidad de este componente: a diferencia de la mayoría de chips, utiliza los mensajes que recibe mientras reporta su información para cambiar la configuración. Debido a esto, no podemos usar la función convencional de lectura SPI de QMK (que internamente envía todos los bits a 1), por lo que las lecturas en el código, en vez de hacerse con `uint8_t byte = spi_read()`, usan `uint8_t byte = spi_write(0)`, ya que este valor deja la configuración en un estado correcto.

#if (config.render_todos) {
  todo("Adjuntar y explicar código", inline: true)
}
