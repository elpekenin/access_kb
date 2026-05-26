#import "@preview/codly:1.3.0": codly

#import "@elpekenin/tfm:0.1.0": cli, h, snippet, vars

Gracias al uso de un framework que proporciona las bases para crear el firmware del teclado, solo tendremos que desarrollar los aspectos específicos para nuestro hardware, detallados en esta sección.

#h[Instalación][
  Para poder usar QMK, instalamos su CLI y la utilizamos para clonar el repositorio de código fuente e instalar los compiladores y herramientas necesarios. Después, también podemos comprobar la instalación:
  #cli(
    ```bash
    $ pip install --user qmk
    $ qmk setup
    $ qmk doctor
    ```,
    caption: [Instalación de QMK],
  )

  Opcionalmente, podemos configurar el número de hilos que usará el compilador, acelerando el bucle de desarrollo:
  #cli(
    ```bash
    $ qmk config compile.parallel=20
    ```,
    caption: [Personalizar compilación QMK],
  )
]

#h[Multi-bus SPI][
  Dado que QMK no proporciona soporte para configurar varios buses SPI al mismo tiempo, es necesario adaptar el código para ello. Duplicamos el archivo `platforms/chibios/drivers/spi_master.c`, para convertir sus variables en arrays y poder almacenar el estado de los distintos buses, en vez de una única configuración.

  Para compilar este nuevo archivo, añadimos `SRC += spi_custom.c` a `rules.mk` y creamos una cabecera `spi_custom.h` para que los archivos que utilicen esta funcionalidad conozcan las funciones disponibles en nuestro código.
]

#h[Escaneo de teclas][
  Dado que QMK no provee soporte para leer teclas conectadas a registros de desplazamiento, escribimos un poco de código para configurar el bus SPI y usarlo para escanear las teclas.
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
  La API estandarizada Quantum Painter @qp proporciona funciones para primitivas gráficas (puntos, líneas, etc.) y drivers para múltiples dispositivos, simplificando el desarrollo significativamente.

  Las pantallas ILI9163 e ILI9341 ya están soportadas. Para la IL91874, desarrollamos un driver personalizado.

  Dado que cada píxel puede ser blanco, negro o rojo, el formato de datos que se utiliza es de 2 bits por píxel. Así, si el bit que controla el color está a "1", tendremos un píxel rojo y, en caso contrario, el color vendrá determinado por el segundo bit, siendo "0" blanco y "1" negro. Además, a diferencia de otros dispositivos más modernos, esta pantalla no permite refrescar regiones aisladas, sino que requiere enviar el contenido completo cada vez que vayamos a dibujar.

  Debido a estas características, el código para interactuar con esta pantalla consiste en dos framebuffers donde almacenamos por separado el estado del bit rojo y el bit blanco/negro. Por suerte, QMK ya dispone de utilidades que permiten interactuar con un buffer usando la misma API de dibujo. Por tanto, nuestro código será una capa de utilidad que emplea dos de estas "pantallas virtuales".
  #snippet(
    ```c
    // calcula la distancia entre 2 colores HSV usando Pitágoras (sin potencias para acelerar el cálculo)
    static inline uint16_t hsv_distance(HSV a, HSV b) {
        return abs(a.h - b.h) + abs(a.s - b.s) + abs(a.v - b.v);
    }

    // ... en otra parte del código
    for (int16_t i = 0; i < palette_size; ++i) {
      HSV hsv = (HSV){palette[i].hsv888.h, palette[i].hsv888.s, palette[i].hsv888.v};

      // calculamos la distancia hacia las 3 posibles salidas
      uint16_t white = hsv_distance(hsv, (HSV){HSV_WHITE});
      uint16_t black = hsv_distance(hsv, (HSV){HSV_BLACK});
      // el código está preparado para manejar otros colores
      // por ejemplo: pantallas blanco/negro/amarillo, aquí leemos la configuración de la pantalla
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

  #pagebreak()
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
    size: 8pt,
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
    caption: [Traspaso a los buffers de cada color],
    size: 9pt,
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
  Necesitaremos un driver para comunicarnos con el chip (XPT2046) y obtener la posición en la que se ha pulsado la pantalla. QMK no tiene funcionalidades similares, por lo que he tenido que implementarlo desde cero. La arquitectura del código es igual que para las pantallas, en un par de `struct`s almacenamos las variables y funciones necesarias para mandar/leer mensajes al dispositivo.

  Al arrancar el teclado, ejecutamos la inicialización del sensor y lo iremos leyendo cuando de señal de salida *IRQ* nos marque que la pantalla está pulsada, evitando transmitir mensajes mientras no esté en uso.
  #snippet(
    ```c
    // el parámetro `check_irq` permite forzar una lectura, ej: si IRQ no se ha conectado
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
    size: 9pt,
  )

  #block(breakable: false)[
    Una cosa importante es que el sensor se puede configurar a la vez que se lee. Por ello no podemos usar la función `spi_read()`, que envía un mensaje con todos los bits a 1 y desconfigura el sensor. En su lugar, escribimos un `0` para mantener el sensor en servicio y el valor que nos llega la vez es nuestra lectura.
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
      size: 10pt,
    )
  ]
]

#h[Comunicación con ordenador][
  Para intercambiar mensajes entre dispositivos utilizaremos XAP @xap. Este protocolo definido por QMK funciona encima de HID y permite el intercambio de información. Evita problemas con los drivers del sistema operativo usando un endpoint adicional; es decir, utiliza un flujo de datos *independiente* de la comunicación convencional del teclado para reportar el estado de las teclas

  #block(breakable: false)[
    Los mensajes que se reciben en el teclado para lanzar acciones o leer información se definen en el archivo `xap.hjson` #footnote[Hjson @hjson es un super-set de JSON más fácil de usar]. Por ejemplo, un mensaje para que el ordenador pueda descubrir el tamaño de una pantalla
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
        #footnote[Equivalente en JSON del contenido del archivo, para tener marcado de sintaxis en el informe]
      ],
      size: 9pt,
    ) <xap:hjson>
  ]

  Lógica para responder a un mensaje
  #snippet(
    ```c
    bool xap_execute_qp_get_geometry(
      xap_token_t token,
      xap_route_user_quantum_painter_get_geometry_arg_t *arg
    ) {
      // manda ACK al ordenador
      // almacena el tiempo actual (para saber cuándo se recibió el último mensaje)
      xap_preprocess(token);

      // preparamos las variables donde se almacenarán los datos
      painter_rotation_t rotation;
      uint16_t           width;
      uint16_t           height;
      uint16_t           offset_x;
      uint16_t           offset_y;

      // buscar pantalla según el nombre configurado
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

  El teclado es capaz de mandar mensajes de forma autónoma, usando la función `xap_broadcast_user`. Este mecanismo se empleará para informar del estado de la pantalla táctil: en vez de que el ordenador esté constantemente preguntando, el teclado se encarga de enviar un mensaje cada vez que hay un cambio
  #snippet(
    ```c
    // NOTA: enviar un identificador del sensor permite diseños multi-sensor
    void xap_screen_pressed(uint8_t screen_id, touch_report_t report) {
      const screen_pressed_msg_t msg = {
        .msg_id    = SCREEN_PRESSED,
        .screen_id = screen_id,
        .x         = report.x,
        .y         = report.y,
      };
      xap_broadcast_user(&msg, sizeof(msg));
    }
    ```,
    caption: [Envío de mensaje XAP desde teclado],
  ) <xap:broadcast>
]

#h[Interfaz de usuario][
  Para evitar problemas al dibujar en las pantallas, es importante dividir el espacio donde mostraremos cada elemento, si no, podemos acabar con elementos que han sido sobre-escritos por otros.

  Esto requiere de cálculos sencillos, pero cuando se quiere añadir o quitar algún elemento, toca rehacer a mano las sumas y multiplicaciones. Para solventar esta problemática, se implementó una pequeña librería que calcula las áreas a ocupar por cada componente a partir de las dimensiones de la pantalla y una configuración declarativa (p.ej: "la mitad derecha de pantalla van a ser 3 elementos igual de grandes ...")

  Esta configuración se basa en un árbol donde los nodos hoja almacenan cuándo dibujarlos de nuevo (representación dinámica) y la función a utilizar para dicha tarea, mientras que los nodos intermedios actúan como contenedores cuyo espacio se repartirá entre sus hijos.
  #snippet(
    ```c
    typedef struct _ui_node_t {
      // listado de hijos
      const ui_children_t children;
      // qué tamaño se desea tener. ej: "10% de mi padre"
      const ui_node_size_t node_size;
      // como dividir este nodo: de arriba a abajo, de derecha a izquierda, ...
      const ui_split_direction_t direction;

      // resultado de intentar resolver los requisitos: pendiente, ok, error
      ui_state_t state;
      // tamaño calculado
      ui_vector_t start; // (x, y)
      ui_vector_t size;  // (size_x, size_y)

      // validación/inicialización del nodo
      bool (*const init)(struct _ui_node_t *);

      // estado del dibujado
      ui_time_t   next_render;
      void *const args;
      ui_time_t (*const render)(const struct _ui_node_t *, painter_device_t);
    } ui_node_t;
    ```,
    caption: [Estructura de un nodo],
  )

  #block(breakable: false)[
    Así, podemos definir una interfaz cuyas dimensiones cambian para adaptarse a configuración que usemos, dado que algunos nodos que solo existen en algunas circunstancias. Nótese el nodo con `#if IS_ENABLED(...)`
    #snippet(
      ```c
      static ui_node_t left[] = {
        {
          // nodo tan alto como la imagen
          .node_size = UI_IMAGE(1),
          // dividir de izquierda a derecha
          .direction = UI_SPLIT_DIR_LEFT_RIGHT,
          // elementos contenidos
          .children  = UI_CHILDREN(first_row),
          // contiene una referencia a la imagen, para obtener su tamaño
          .args      = &gh_args,
        },
        {
          .node_size = UI_FONT(1),
          .init      = layer_init,
          .render    = layer_render,
          .args      = &layer_args,
        },
      #if IS_ENABLED(MEMORY)
        {
          .node_size = UI_FONT(1),
          .init      = flash_init,
          .render    = flash_render,
          .args      = &flash_args,
        },
      #endif
        {
          .node_size = UI_REMAINING(),
          .init      = computer_init,
          .render    = computer_render,
          .args      = &computer_args,
        }
      };

      static ui_node_t nodes[] = {
        {
          .node_size = UI_RELATIVE(50),
          .direction = UI_SPLIT_DIR_TOP_BOTTOM,
          .children  = UI_CHILDREN(left),
        },
        {
          .node_size = UI_REMAINING(),
          .direction = UI_SPLIT_DIR_TOP_BOTTOM,
          .children  = UI_CHILDREN(right),
        },
      };

      ui_node_t root = {
        .direction = UI_SPLIT_DIR_LEFT_RIGHT,
        .children  = UI_CHILDREN(nodes),
      };
      ```,
      caption: [Definición de una interfaz],
      size: 9pt,
    )
  ]

  Aquí podemos ver cómo se dibuja uno de estos nodos
  #snippet(
    ```c
    bool layer_init(ui_node_t *self) {
      layer_args_t *const args = self->args;

      // no se ha configurado la función -> error
      if (args->get_layer_name == NULL) {
        return false;
      }

      // inicializa estado interno
      args->last.layer = ~0;

      // si la fuente no entra en la altura que tiene asignada este nodo
      // se marcará (y por tanto, al árbol entero) como irresoluble
      return ui_font_fits(self);
    }

    ui_time_t layer_render(const ui_node_t *self, painter_device_t display) {
      layer_args_t *const args = self->args;

      const uint8_t layer = get_highest_layer(layer_state | default_layer_state);

      // mismo texto que ya está dibujado, evitar trabajo
      if (args->last.layer == layer) {
        goto exit;
      }

      const painter_font_handle_t font = qp_load_font_mem(args->font);
      if (font == NULL) {
        goto exit;
      }

      const char *const str = args->layer_name(layer);

      const uint16_t width = qp_textwidth(font, str);
      if (width == 0 || width > self->size.x) {
        goto err;
      }

      if (args->last.width > width) {
        bool ret = qp_rect(display, self->start.x + width, self->start.y, self->start.x + args->last.width, self->start.y + font->line_height, HSV_BLACK, true);
        if (!ret) {
          goto err;
        }
      }

      qp_drawtext(display, self->start.x, self->start.y, font, str);

      args->last = (last_layer_t){
        .layer = layer,
        .width = width,
      };

    err:
        qp_close_font(font);

    exit:
        return args->interval;
    }
    ```,
    caption: [Implementación de un nodo],
    size: 8pt,
  )
]
