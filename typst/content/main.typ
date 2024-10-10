#import "@preview/codly:1.0.0": codly-range
#import "@preview/glossarium:0.4.2": gls, print-glossary

#import "../glossary.typ": entry-list
#import "utils.typ": cli, snippet

#include "parts/front_page.typ"

/// White page
#page[
  // 0 to not render page number here
  #counter(page).update(0)
]

/// Document overview (sections, glossary, figures, etc)
#outline(
  fill: block(width: 100% - 1.5em)[
    #repeat("  .  ")
  ],
  indent: auto,
)

#heading("Listado de acronimos", outlined: false, numbering: none)
#print-glossary( // TODO: avoid centered text
  entry-list,
  show-all: true,
  disable-back-references: true,
)

#outline(
  target: figure.where(kind: image),
  title: [Listado de imágenes],
)

#outline(
  target: figure.where(kind: "snippet"),
  title: [Listado de código],
)

#outline(
  target: figure.where(kind: "cmd"),
  title: [Listado de comandos],
)

/// Document begins, finally
= Resumen
#include "parts/summary.typ"

= Introducción <introducción>
#include "parts/introduction.typ"

== Requisitos previos <requisitos-previos>
A lo largo del documento se asume cierto conocimiento de programación, así como tener algunos programas instalados. La mayoría son conocidos, pero aquí tenemos una lista con algunos menos populares y enlaces a sus respectivas páginas oficiales/documentación
- Rust @rust. Lenguaje de programación
- typst @typst. Lenguaje de marcado, parecido a LaTex, con el que se ha compuesto este documento.
- WSL @wsl. Herramienta de Windows, similar a una máquina virtual, para tener un entorno Linux

= Estado del arte <estado-del-arte>
#include "parts/state_of_the_art.typ"

= Desarrollo
#include "parts/development.typ"

== Hardware <hardware>
#include "parts/hardware/main.typ"

== Firmware <firmware>

=== Funcionalidad <funcionalidad>
La mejor parte de usar una librería tan extendida como lo es QMK es que tenemos muchas facilidades a la hora de escribir el codigo ya que buena parte del trabajo está hecho ya.

Esto incluye:
- Gestión sobre #gls("usb") para reportar distintos endpoints (teclado, ratón, multimedia)
- Drivers para diversos periféricos tales como las pantallas ya comentadas, piezoelectricos para tener feedback sonoro o solenoides para vibración al pulsar las teclas, por nombrar algunos...
- Abstracciones para poder usar la misma API en diversos microcontroladores que internamente utilizan código y hardware muy diferente
- Documentación extensa y detallada
- Servidor ofcial en Discord donde podemos encontrar ayuda

=== Escaneo de teclas <escaneo-de-teclas>
Como se ha comentado en el apartado anterior, vamos a usar registros de desplazamiento, sin embargo QMK tiene soporte para cableado directo, en matrices y usando algunos otros circuitos, como _IO expanders_. Sin embargo este no es el caso para los registros por lo que tendemos que escribir un poco de código para leer su información.

=== Pantallas <pantallas-1>
QMK tiene una API estandarizada (Quantum Painter#cite(<qp>)) para primitivas de dibujo en interfaces gráficas. Aún es "joven" y no soporta demasiadas pantallas, pero hace gran parte del trabajo por nosotros.

=== Pantalla táctil
<pantalla-táctil>
QMK también tiene una capa de abstracción#cite(<pointing>) para diversos sensores que permiten mover el cursor, sin embargo todos se basan en medir desplazamientos y no una coordenada como es el caso de la pantalla táctil, por tanto en este caso nos vemos obligados a diseñar una arquitectura de software nueva, para poder trabajar con este otro tipo
de datos de entrada.

= Desarrollo <desarrollo>

== Hardware <hardware-1>

=== Objetivos <objetivos>
Las metas principales a la hora de diseñar el teclado han sido el usar la menor cantidad de pines posible para las tareas "básicas" y el exponer todos los pines restantes así como varias tomas de alimentación. De esta forma, el teclado puede servir como placa de pruebas donde desarrollar drivers para otro hardware y además es modular, ya que nos permite añadir más periféricos (por ejemplo, para sonido) en el futuro sin necesidad de tener que fabricar una nueva PCB.

=== Distribución <distribución>
He optado por una disposición ortolineal, split y (por ahora) QWERTY, reduciendo un par de columnas en el lateral derecho respecto al tamaño habitual, ya que varias de esas teclas raramente se usan, y al tener una forma simétrica es más sencillo de diseñar.

#figure(
  image("../images/layout.png", width: 100%),
  caption: [Diseño aproximado del teclado],
)

=== Desarrollo <desarrollo-1>

==== Teclas <teclas>
Como ya vimos en la sección 3.2.1 @sec:scanning, el escaneo de teclas se hará mediante registros de desplazamiento de entrada en paralelo y salida en serie, para un mekor tiempo de respuesta vamos a usar componentes SPI en vez de I2C (protocolo más rápido). Podríamos haber usado SN74HC165 que son algo más baratos y fáciles de encontrar, pero vamos a usar SN74HC589ADR2G ya que tienen el mismo funcionamiento pero presentan 7 de las 8 entradas en el mismo lateral del circuito integrado (en vez de 4 en cada lado), por lo que son mucho más sencillos de enrutar. Además, aunque cada mitad del teclado tiene un total de 29 teclas y, por tanto, 4 registros (32 entradas) nos hubieran bastado; vamos a usar un registro por cada fila (5), ya que evitamos tener que enrutar una de las teclas hacía el otro lado del registro y tener algunas conexiones "saltando" de una fila a otra.

Realizamos las conexiones de la siguiente manera:
- Las entradas que no usamos las conectaremos a tierra para que se lean como '0'. Las teclas se conectan a VCC y (mediante "pulldown") a una entrada. De esta forma leeremos '1' cuando estén pulsadas y '0' cuando no lo estén
- La salida serie de cada integrado se conecta a la entrada serie del siguiente. El último se conecta a MISO (entrada SPI del microcontrolador)
- Las señales CS, Latch y CLK son comunes a todos los integrados
- Como la señal Latch resulta ser la inversa de CS, en vez de usar otro pin, invertimos el voltaje con un MOSFET

#figure(
  image("../images/inverter.png", width: 30%),
  caption: [MOSFET como inversor],
)

#figure(
  image("../images/piso.png", width: 80%),
  caption: [SN74HC589ADR2G para una fila],
)

==== Cableado de las pantallas <cableado-de-las-pantallas>
Vamos a usar una pantalla en el lado izquierdo (IL91874) y dos en el derecho (ILI9163 e ILI9341), estos dispositivos necesitan de conexión SPI así como pines extra (DC, CS, RST) para ser controlados. Dado que las vamos a conectar al mismo bus, solo necesitamos 3 pines para SPI y puesto que no podemos mandar información a dos pantallas simultáneamente, también pueden usar una línea DC común. Sin embargo las señales de DC y RST deben ser individuales y además la ILI9341 necesita dos señales DC, una para la pantalla y otra para el sensor táctil. Por tanto, también usaremos registros de desplazamiento, en este caso de entrada serie con salidas en paralelo (SN74HC595) y dichas salidas se conectaran a las entradas de control de las pantallas. Como ya teníamos un bus SPI configurado, usar estos registros tan solo supone el uso de otro pin (CS). Sin embargo, como necesitamos cambiar estas señales mientras estamos hablando con las pantallas, no podemos tener las dos cosas en el mismo bus, por lo que usaremos un bus para los registros (escanear teclas y señales de control) y otro bus para las pantallas. Lo mas importante de este diseño es que encadenando suficientes registros podríamos controlar tantas señales de control para diversos dispositivos como queramos, manteniendo mínima la utilización de pines del MCU.

== Firmware <firmware-1>

=== Instalación y configuración de QMK <instalación-y-configuración-de-qmk>
Para usar QMK#cite(<qmk>) instalamos su CLI ejecutando ya que se trata de una librería escrita en Python y está disponible en el repositorio de paquetes de Python(pip). Tras esto, descargamos el código fuente con , a este comando le podemos pasar como parámetro nuestro fork del repositorio (en mi caso ), para poder usar git, ya que no tendremos permisos en el repositorio oficial. Este comando también nos instalará los compiladores necesarios y comprobará las udev de nuestro sistema Linux, para que podamos trabajar con los dipositivos sin problema, en caso de que no estén bien configuradas podemos usar copiar el archivo en . Finalmente podemos ejecutar para comprobar el estado de QMK. Para poder hacer debug con he necesitado añadir una línea a dicho archivo:

Opcionalmente, podríamos instalar LVGL#cite(<lvgl>) funciones gráficas más complejas en la pantalla LCD, en vez de usar el driver de QMK. Esto se hace con (desde el directorio base de QMK). Seguidamente, usamos el código de jpe#cite(<lvgl-jpe>) para usar esta librería.

De momento no he implementado esta librería puesto que añadiría bastante complejidad al código y tampoco vamos a hacer ninguna interfaz especialmente llamativa.

Para acelerar el proceso de compilado podemos guardar nuestra configuración (teclado y keymap) y el número de hilos que usará el compilador

#cli(
  ```bash
  $ qmk config user.keyboard=elpekenin/access
  $ qmk config user.keymap=default
  $ qmk config compile.parallel=20
  ```,
  caption: [Personalizar QMK],
)

=== Driver para pantalla ili9486 <driver-para-pantalla-ili9486>
Para hacer pruebas antes de diseñar y fabricar la PCB, he usado un módulo que integra una pantalla, sensor táctil y lector de tarjeta SD. Sin emabargo, QMK no tiene soporte para este modelo concreto de pantalla. Por tanto, usando como base el código#cite(<ili9488>) para otro dispositivo de la misma familia, y con ayuda de los usuarios \@sigprof y \@tzarc he desarrollado un driver#cite(<ili9486-pr>) para la pantalla usada.

Los cambios se reducen a dos cosas:
- Cambiar la fase de inicialización de la pantalla, que configura valores como el formato en el que se envían los píxeles a mostrar
- Puesto que la pantalla contiene un circuito que convierte la línea SPI en una señal paralela de 16 bits que se envía a la pantalla, mediante registros de desplazamiento, tendremos problemas al enviar información de un tamaño que no sea múltiplo de 16, por tanto hacemos un par de cambios al código "normal".

=== Dibujar por USB <section:dibujar-usb>
Lo siguiente que necesitamos resolver es controlar la pantalla desde el ordenador, de forma que podamos dibujar en ella desde nuestro software, para esto usaremos la funcionalidad XAP#cite(<xap>) (aún en desarrollo), que nos proporciona un nuevo endpoint HID#cite(<hid>) sobre el bus USB y un protocolo#cite(<xap-specs>) que podemos extender. Ahora podemos definir nuestros mensajes simplemente creando un archivo #cite(<xap-custom-msg>) Una vez creados los mensajes, añadimos el código#cite(<qp-xap-handlers>) que se ejecuta cuando los recibamos

=== Mejora algoritmo elipses <mejora-algoritmo-elipses>
Durante las pruebas con el script anterior, vi que el algoritmo para dibujar elipses funcionaba regular, por lo que hice un _refactor_ del mismo. Es decir, mantenemos la misma "forma" (atributos que se reciben y valor devuelto), pero la implementación#cite(<ellipse-pr>) es compleamente diferente. Sigue sin ser perfecto, pero parece funcionar mejor.

=== Driver para sensor XPT2046 <driver-para-sensor-xpt2046>
Al igual que hemos necesitado un driver (código que nos permite comunicarnos con el hardware) para dibujar en la pantalla, necesitamos otro para poder leer la posición en la que esta ha sido pulsada. Por desgracia, QMK no tiene ninguna característica parecida, por lo que usaremos el código#cite(<touch-driver>) que he escrito para ello desde 0.

Este hardware, al igual que la pantalla, también tiene una particularidad, y es que la función proporcionada por QMK, realmente es un intercambio de información en vez de solo lectura, esto es debido a que SPI es un protocolo síncrono. El problema aparece porque QMK envía todos los bits a 1 -dado que algunos microcontroladores hacen esto por hardware y no se puede cambiar- para poder recibir información, *sin embargo*, el XPT2046 (a diferencia de la mayoria de chips) utiliza esta información que recibe mientras que nos reporta su información para cambiar su configuración. De forma, que en caso de que los 2 últimos bits que enviamos no tengan el valor adecuado, no podremos usar el sensor. Lo bueno es que la solución es tan simple como usar para enviar un byte vacío y recibir la respuesta dejando una configuración conveniente.

=== Generar imágenes <generar-imágenes>
Dado que vamos a usar varios iconos para hacer la interfaz gráfica en la pantalla, he cogido la colección de iconos Material Design Icons#cite(<templarian-mdi>) y he convertido todos sus iconos al formato que utiliza QMK para representar imágenes. Las imágenes originales, scripts usados para la conversión y los archivos en formato QGF#cite(<qgf>) se pueden encontrar en este repositorio#cite(<mdi-qgf>)

=== Añadir nuestro código <añadir-nuestro-código>
Para poder usar nuestros nuevos archivos, no es suficiente con añadir un sino que también debemos indicarle a las herramientas de compilado que "busquen" archivos en la carpeta donde tenemos el código, en este caso la carpeta base del teclado, asi como subcarpeta . También es necesario, no tengo muy claro por qué, añadir los archivos de las imagenes QGF o no podremos usarlos. Este archivo se complica un poco porque no debemos incluir algunos archivos si no están algunas opciones habilitadas, por ejemplo, no debemos añadir las imágenes si no tenemos la opción de usar la pantalla habilitada. Esto lo hacemos con el archivo #cite(<rules-mk>)

=== Comunicación con el software de PC <comunicación-con-el-software-de-pc>
La idea principal del control de la pantalla es que el teclado no dibuje en ella nada, de forma que la controlemos exclusivamente desde el programa desarrollado. De esta manera, lo que haremos será leer el estado del sensor táctil cada 200ms y, en caso de estar pulsado, enviaremos las coordenadas de dicha pulsación. Adicionalmente enviamos otro mensaje como si se hubiera pulsado la posicion (0, 0), donde no hay ninguna lógica, para que se "limpie" la pantalla al acabar una pulsación.

El código relevante es:

Al igual que hemos dicho antes con añadir o no añadir un archivo a nuestro código según la configuración, debemos hacer lo mismo con bloques de código, usando de forma que no intentemos usar la pantalla táctil o enviar un mensaje XAP si estas características no se han activado.

== Software <software>
Para poder intercambiar información con el teclado de forma que podamos configurarlo o enviarle información en vez de simplemente escuchar las teclas que se han pulsado, vamos a desarrollar un programa en Tauri#cite(<tauri>) (librería escrita en Rust) ya que permite usar el mismo código en multitud de sistemas operativos gracias a que funciona internamente con un servidor HTML.

=== Instalación <instalación>
Para instalar Rust podemos usar pacman , sin embargo para desarrollar código es preferible usar un script que nos proporciona la comunidad del lenguaje, y que permite cambiar fácilmente la versión del lenguaje con la que compilamos, tan sólo necsitamos ejecutar

La comunicación con el teclado se realiza usando la librería hidapi#cite(<hidapi>), a la que accedemos desde Rust gracias al wrapper#cite(<hidapi-rs>) que implementa una "pasarela" a la librería en C. Para instalarla tan solo necesitamos añadirla al archivo de nuestro proyecto

Primero instalamos NodeJS con , después clonamos el repositorio#cite(<qmk_xap>), usado de base (y en el que he colaborado) para desarrollar el software. Para instalar las dependencias de JavaScript ejecutamos . Ahora ya podemos correr para lanzar nuestro programa.

He tenido que desactivar la opción wgl(libreria de Windows para OpenGL) en VcXsrv para que funcionase

=== Desarrollo <desarrollo-2>

==== Escuchar nuestros mensajes <escuchar-nuestros-mensajes>
Lo primero que vamo a añadir es la capacidad de poder recibir los mensajes que nos llegan desde el teclado, para ello definimos un struct que indique el formato del mensaje, podemos ver algo de código que no es necesario entender, lo importante son los dos `u16`

#snippet(
  ```rust
  #[derive(BinRead, Debug)]
  pub struct ReceivedUserBroadcast {
      pub x: u16,
      pub y: u16,
  }

  impl XAPBroadcast for ReceivedUserBroadcast {}
  ```,
  caption: [Definición de mensaje de usuario],
)

Acto seguido, hacemos que el _eventloop_ de nuestra aplicación escuche los mensajes, ya que por defecto los ignora. Primero definimos un nuevo evento dentro del listado de tipos de evento

#snippet(
  ```rust
  pub(crate) enum XAPEvent {
      HandleUserBroadcast {
          broadcast: BroadcastRaw,
          id: Uuid,
      },
      // -- Otros eventos recortados --
  }
  ```,
  caption: [Definición de nuevo evento],
)

Añadimos el código necesario para publicar este nuevo tipo de evento al
recibir los mensajes correspondientes

#snippet(
  ```rust
  match broadcast.broadcast_type() {
      BroadcastType::User => {
          event_channel
              .send(XAPEvent::ReceivedUserBroadcast { id, broadcast })
              .expect("failed to send user broadcast event!");
      }
      // -- Otros tipos recortados --
  }
  ```,
  caption: [Emitir evento al recibir mensaje],
)

Y por último hacemos la relación entre este evento la lógica de usuario
que se debe ejecutar con ellos.

#snippet(
  ```rust
  match broadcast.broadcast_type() {
      // -- Otros eventos recortados --
      Ok(XAPEvent::ReceivedUserBroadcast{broadcast, id}) => {
          user::broadcast_callback(broadcast, id, &state);
      }
  }
  ```,
  caption: [Manejo del evento],
)

Toda la lógica de usuario (personalizable), se puede encontrar en el archivo #cite(<user-rs>)

==== Correr aplicación en segundo plano <correr-aplicación-en-segundo-plano>
Dado que todo el contenido de la pantalla, así como su funcionalidad al pulsarla dependen de nuestra aplicación, vamos a hacer todo lo posible para que se mantenga abierta. Para ello hacemos que al pulsar el boton de cerrar, la app se minimice en vez de terminar y le añadimos un _systray_ para tener un icono en la barra de tareas donde podamos volver a ponerla en pantalla

#snippet(
  ```rust
  let tray_menu = SystemTrayMenu::new()
      .add_item(CustomMenuItem::new("show".to_string(), "Show"))
      .add_item(CustomMenuItem::new("hide".to_string(), "Hide"))
      .add_native_item(SystemTrayMenuItem::Separator)
      .add_item(CustomMenuItem::new("quit".to_string(), "Quit"));
  ```,
  caption: [Creación del _systray_],
)

#snippet(
  ```rust
  .system_tray(system_tray)
  .on_system_tray_event(move |app, event|
      match event {
          // Al usar click izquierdo
          SystemTrayEvent::MenuItemClick { id, .. } => {
              // Segun boton usado
              match id.as_str() {
                  "hide" => app.get_window("main").unwrap().hide().unwrap(),
                  "quit" => {
                      user::on_close(_state.clone());
                      std::process::exit(0);
                  },
                  "show" => app.get_window("main").unwrap().show().unwrap(),
                  _ => {}
              }
          },
          _ => {} // Otros eventos, no hacemos nada
      }
  )
  ```,
  caption: [Añadir _systray_ al programa, con su logica],
)

#snippet(
  ```rust
  .on_window_event(
      |event| match event.event() {
          tauri::WindowEvent::CloseRequested { api, .. } => {
              event.window().hide().unwrap();
              api.prevent_close()
          },
          _ => {} // Otros eventos, no hacemos nada
      }
  )
  ```,
  caption: [Interceptar señal de cierre de ventana],
)

==== Indicador de conexión <indicador-de-conexión>
Vamos a añadir también un indicador en la pantalla, de forma que Tauri nos haga saber cuando se conecta o desconecta del teclado, evitando que intentemos usar la pantalla cuando no va a funcionar. Editamos el eventlooop y definimos las nuevas funciones

#snippet(
  ```diff
  match msg {
      Ok(XAPEvent::Exit) => {
          info!("received shutdown signal, exiting!");
  +       user::on_close(state);
          break 'event_loop;
      },
      Ok(XAPEvent::NewDevice(id)) => {
          if let Ok(device) = state.lock().get_device(&id){
              info!("detected new device - notifying frontend!");
  +           user::on_device_connection(device);
          }
      }

  ```,
  caption: [Hooks de usuario],
)


==== Arrancar HomeAssistant <arrancar-homeassistant>
Dado que nuestro programa se tiene que comunicar con la domótica de casa, vamos a asegurarnos de que esté ejecutándose, iniciándolo al abrir el programa.

Al arrancar la aplicación ejecutamos:
#snippet(
  ```rust
  pub(crate)fn on_init() {
      match std::process::Command::new("sh")
          .arg("-c")
          .arg(r#"sudo systemctl start docker
                  && cd $HOME/docker
                  && docker compose up -d"#)
          .output()
      {
          Ok(_) => error!("on_init went correctly"),
          Err(out) => error!("on_init failed due to: {out}")
      }
  }
  ```,
  caption: [Hook de inicio],
)

= Lineas futuras <lineas-futuras>

= Anexo I: Instalación de MicroPython <anexo-i-instalación-de-micropython>
Durante el desarrollo del proyecto, he probado _MicroPython_#cite(<micropython>) una implementación en C del intérprete de Python que se enfoca a su uso en microcontroladores. Finalmente he descartado usarlo debido a su menor rendimiento y la falta de muchas opciones que vienen hechas en QMK. Sin embargo creo que puede ser una buena alternativa para prácticas de electónica en la universidad, reemplazando a Arduino, puesto que Python es mucho más amigable que C o C++

== Preparar el compilador <preparar-el-compilador>
Tras clonar el source de MicroPython , hacemos para compilar el compilador cruzado de MicroPython que nos permitirá convertir el código fuente para ser ejecutado en diferentes arquitecturas.

== Compilar para Linux (Opcional) <compilar-para-linux-opcional>
Si queremos usar MicroPython en nuestro ordenador para hacer pruebas, en vez de CPython(que es la versión más común), usaremos el compilador que acabamos de construir para compilar el código fuente del intérprete y usarlo en nuestra máquina

#cli(
  ```bash
  $ cd ports/unix
  $ make submodules
  $ make
  ```,
  caption: [Compilar MicroPython para Linux],
)

#cli(
  ```bash
  $ cd build-standard
  $ ./micropython
  MicroPython 13dceaa4e on 2022-08-24; linux [GCC 12.2.0] version Use Ctrl-D to exit, Ctrl-E for paste mode
  >>>
  ```,
  caption: [Ejecutar MicroPython en Linux],
)

== Compilar para RP2040 <compilar-para-rp2040>
Primero instalamos un compilador necesario para la arquitectura del procesador, en mi caso (Arch Linux), el comando es y después añadimos la configuración necesaria para reportar y usar un endpoint HID, siguiendo (y adaptando) #cite(<tusb-rp2>)

Definimos en C el módulo #cite(<mod-usb-hid>);, será la macro encargada de añadir el módulo al firmware compilado. También debemos editar un archivo de configuración de compilación para que se añada el nuevo archivo

Para poder compilar la versión de RP2040 en Arch he necesitado instalar el paquete

Por último, compilamos con
#cli(
  ```bash
  $ cd ports/rp2
  $ make submodules
  $ make clean
  $ make
  ```,
  caption: [Compilar MicroPython para RP2040],
)

Y ya podremos flashear este binario en nuestra placa de desarrollo


#bibliography(
  "../bibliography.yml",
  title: "Bibliografía",
)

= Anexo B. Código fuente del informe (typst).

Aquí están adjuntas las primeras líneas del código fuente con el que he generado este documento.

#text(size: 8pt)[
  #codly-range(start: 1, end: 20);

  #raw(
    read("main.typ").split("\n").slice(0, 30).join("\n"),
    lang: "typst",
  )
]
