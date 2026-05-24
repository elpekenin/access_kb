#import "@preview/big-todo:0.2.0": todo

#import "@elpekenin/tfm:0.1.0": cli, h, snippet, vars

Gracias al uso de un framework que proporciona las bases para crear el firmware del teclado, solo tendremos que desarrollar los aspectos específicos para nuestro hardware, detallados en esta sección. Para que este código adicional se incluya en el binario final, será necesario modificar la configuración de `make` para indicar la ubicación de nuestros archivos o añadir opciones extra al compilador, lo cual se explicará en las siguientes secciones.

#h[Instalación][
  Para poder usar QMK, instalamos su @cli y la utilizamos para clonar el repositorio de código fuente e instalar los compiladores y herramientas necesarios. Después, también podemos comprobar la instalación:
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
]

#h[Multi-bus @spi][
  Dado que QMK no proporciona soporte para configurar varios buses @spi al mismo tiempo, es necesario adaptar el código para ello. Se realiza una copia del archivo `platforms/chibios/drivers/spi_master.c`, modificando las variables existentes para convertirlas en arrays, de forma que puedan almacenar el estado de los distintos buses en vez de una sola configuración.

  Para compilar este nuevo archivo, añadimos `SRC += spi_custom.c` a `rules.mk` y creamos una cabecera `spi_custom.h` para que los módulos que utilicen esta funcionalidad conozcan las funciones disponibles en nuestro código.
]

#h[Escaneo de teclas][
  Como hemos visto anteriormente, usaremos registros de desplazamiento para la lectura de teclas. Dado que QMK no provee soporte para esta configuración, escribimos un poco de código para configurar el bus @spi y usarlo para escanear las teclas.
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
]

#h[Pantallas][
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

    // ... en otra parte del código
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
]

#h[Sensor pantalla táctil][
  Necesitaremos un driver para comunicarnos con el chip (XPT2046) y obtener la posición en la que se ha pulsado la pantalla. QMK no tiene funcionalidades similares, por lo que he tenido que implementarlo desde cero. La arquitectura del código es igual que para las pantallas, en un par de `struct`s almacenamos las variables y funciones necesarias para mandar/leer mensajes al dispositivo. Al arrancar el teclado, ejecutamos la inicialización del sensor y conforme haga falta iremos leyendo su información.

  Una característica muy útil de este sensor es que tiene una señal de salida (IRQ) que nos indica si la pantalla está pulsada, con lo que podemos evitar la mensajería para obtener la posición de un toque mientras no está en uso.
  #snippet(
    ```c
    touch_report_t get_spi_touch_report(touch_device_t device, bool check_irq) {
      touch_driver_t          *driver       = (touch_driver_t *)device;
      spi_touch_comms_config_t comms_config = driver->spi_config;

      // esta variable es `static` para almacenar la ubicación de la pulsación entre ejecuciones
      // con esto, al soltar la pantalla, también sabremos la posición donde se había pulsado
      static touch_report_t report = {
          .x       = 0,
          .y       = 0,
          .pressed = false,
      };

      // normalmente, si el pin IRQ no marca "pantalla pulsada", no hay nada que hacer salvo reportar esa información
      // el parámetro `check_irq` permite forzar una lectura, en caso de que no se haya conectado
      if (check_irq && comms_config.irq_pin != NO_PIN && readPin(comms_config.irq_pin)) {
          goto none;
      }

      // si falla la comunicación no podemos seguir
      if (!touch_spi_start(comms_config)) {
          goto none;
      }

      report.pressed = true;

      // pequeña espera para reducir efecto del ruido
      wait_ms(20);

      // leemos valor del sensor
      int16_t x = read_coord(driver->spi_config.x_cmd, comms_config);
      int16_t y = read_coord(driver->spi_config.y_cmd, comms_config);

      // calculamos la posición real valor según la calibración del sensor
      report_from(x, y, driver, &report);

      touch_spi_stop(comms_config);

      return report;

    none:
      report.pressed = false;
      return report;
    }
    ```,
    caption: [Lectura de coordenadas del XPT2046],
  )

  Otro factor importante es que es posible cambiar la configuración del sensor a la vez que lo leemos. Debido a esto, no podemos usar la función `spi_read()` para obtener el dato del sensor porque internamente envía un mensaje con todos los bits a 1 y lo desconfigura. En su lugar, escribimos un `0` para mantener el sensor en servicio y el valor que nos llega la vez es nuestra lectura.
  #snippet(
    ```c
    static int16_t read_coord(uint8_t cmd, spi_touch_comms_config_t comms_config) {
      // estas 2 líneas son equivalente a `gpio_write_high(cs_pin)` para indicar al sensor que vamos a comunicarnos con él
      // sin embargo, necesitamos este código personalizado para nuestro diseño con registros de desplazamiento
      set_sipo_pin(comms_config.chip_select_pin, false);
      send_sipo_state();

      // mandamos el mensaje para pedir la coordenada
      spi_custom_write(cmd, TOUCH_SPI_DRIVER_ID);

      // el valor del sensor son 16bits, tenemos que leer 2 veces (1 byte cada una) para obtenerlos y luego los combinamos
      // como hemos dicho antes, cada lectura se realiza con una escritura de `0`
      int16_t coord = ((spi_custom_write(0, TOUCH_SPI_DRIVER_ID) << 8) | spi_custom_write(0, TOUCH_SPI_DRIVER_ID)) >> 3;

      // indicamos al sensor que hemos terminado con él
      set_sipo_pin(comms_config.chip_select_pin, true);

      return coord;
    }
    ```,
    caption: [Detalle lectura sensor],
  )
]

#h[Comunicación con ordenador][
  Para intercambiar mensajes entre dispositivos utilizaremos @xap (⚠ aún en desarrollo). Este protocolo, definido por QMK, funciona encima de @hid y permite el intercambio de información. Lo más importante es que evita problemas con los drivers del sistema operativo porque usa un endpoint adicional; es decir, utiliza un flujo de datos *independiente* de la comunicación convencional del teclado para reportar el estado de las teclas

  Los mensajes que se reciben en el teclado, para lanzar acciones o leer información se definen en el archivo `xap.hjson` #footnote[Hjson @hjson es un super-set de JSON más fácil de usar]. Por ejemplo, un mensaje para que el programa pueda descubrir el tamaño de una pantalla:
  #snippet(
    ```json
    // identificador del mensaje
    "0x0D": {
      // información varia como un nombre y descripción
      "type": "command",
      "name": "get_geometry",
      "define": "GET_GEOMETRY",
      "description": "Expose `qp_get_geometry`",
      // definimos el contenido del mensaje enviado por el ordenador
      // desglosando los parametros que lo componen
      "request_type": "struct",
      "request_struct_length": 10,
      "request_struct_members": [
          {
            // nombre (identificador) de la pantalla en la que se quiere operar, máximo 9 letras
            "type": "u8[9]",
            "name": "device_name",
          },
          {
            // siempre será '\0` - convenio en C delimitar el fin de un texto
            "type": "u8",
            "name": "dev_terminator",
          },
      ],
      // función que se ejecuta en el teclado al recibir este mensaje
      "return_execute": "qp_get_geometry",
    }
    ```,
    caption: [
      Definición de mensaje XAP enviado por el ordenador
      #footnote[Equivalente en JSON, para tener marcado de sintaxis en el informe]
    ],
  )

  Lógica para responder a un mensaje
  #snippet(
    ```c
    bool xap_execute_qp_get_geometry(
      xap_token_t token,
      // como podemos ver, QMK genera un `struct` a partir de nuestras definiciones
      // recibimos la información lista para usar, en vez de leer byte a byte el mensaje
      xap_route_user_quantum_painter_get_geometry_arg_t *arg
    ) {
      // función común a todos los mensajes, hace 2 cosas:
      //   - manda mensaje ACK al ordenador
      //   - almacena el tiempo actual (para saber cuándo se recibió el último mensaje)
      xap_preprocess(token);

      // preparamos las variables donde se almacenarán los datos
      painter_rotation_t rotation;
      uint16_t           width;
      uint16_t           height;
      uint16_t           offset_x;
      uint16_t           offset_y;

      // nuestro código que permite identificar pantallas con un nombre que configuramos
      const painter_device_t device = get_device_by_name((const char *)arg->device_name);
      if (device != NULL) {
        // pedimos a QMK la información de la pantalla
        qp_get_geometry(device, &width, &height, &rotation, &offset_x, &offset_y);

        // la enviamos al ordenador, los u16 representados en little-endian
        uint8_t ret[9] = {
          lsb(width), msb(width),
          lsb(height), msb(height),
          rotation,
          lsb(offset_x), msb(offset_x),
          lsb(offset_y), msb(offset_y)
        };
        xap_send(token, XAP_RESPONSE_FLAG_SUCCESS, (const void *)ret, sizeof(ret));
      }

      return true;
    }
    ```,
    caption: [Manejo de un mensaje XAP],
  )

  Además, podemos enviar mensajes en cualquier momento usando la función `xap_broadcast_user`. Este es el mecanismo que utilizamos para informar del estado de la pantalla táctil, en vez de que el programa esté constantemente preguntando, el teclado se encarga de enviar un mensaje cada vez que haya un cambio
  #snippet(
    ```c
    // enviar un identificador del sensor permite diseños multi-sensor en un futuro

    void xap_screen_pressed(uint8_t screen_id, touch_report_t report) {
      const screen_pressed_msg_t msg = {
        .msg_id    = SCREEN_PRESSED,
        .screen_id = screen_id,
        .x         = report.x,
        .y         = report.y,
      };

      xap_broadcast_user(&msg, sizeof(msg));
    }

    void xap_screen_released(uint8_t screen_id) {
      const screen_released_msg_t msg = {
        .msg_id    = SCREEN_RELEASED,
        .screen_id = screen_id,
      };

      xap_broadcast_user(&msg, sizeof(msg));
    }
    ```,
    caption: [Envío de mensaje XAP desde teclado],
  )
]

#todo[UI code]
