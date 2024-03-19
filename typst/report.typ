// 3rd party
#import "@preview/acrostiche:0.3.1": acr, init-acronyms, print-index
#import "@preview/sourcerer:0.2.1"

// Helpers
#import "typ/funcs.typ": cli, snippet, page_footer

// Show
#show heading.where(level: 1): it => { pagebreak() + it }
#show bibliography: set heading(numbering: "1.")

// Set
#set heading(numbering: "1.")
#set page(footer: page_footer)
#set par(justify: true)
#set text(font: "Open Sans", lang: "es", ligatures: true, size: 12pt, slashed-zero: true)

// Misc
#init-acronyms((
    "HID": ("Human Interface Device"),
    "I2C": ("Inter Integrated Circuit"),
    "MCU": ("Micro Controller Unit"),
    "PCB": ("Printed Circuit Board"),
    "SPI": ("Serial Peripheral Interface"),
    "USB": ("Universal Serial Bus"),
))


// ***** Content start *****

#page[
    #align(center + horizon)[
        #image("images/UPCT-front.jpg", width: 70%)
    ]

    #align(center)[
        #text(size: 25pt, weight: "bold")[Revisitando el diseño del teclado]
    ]

    #align(bottom + right)[
        #text(weight: "bold", size: 13pt)[
            Autor: Pablo Martínez Bernal

            Director: José Alfonso Vera Repullo

            Máster Universitario en Ingeniería de Telecomunicación

            #datetime.today().display("[month repr:short]-[year]")
        ]
    ]

    // 0 to not render page number here
    #counter(page).update(0)
]

#page[
    // 0 to not render page number here
    #counter(page).update(0)
]

// File content overview
#outline(
    fill: block(width: 100% - 1.5em)[
        #repeat("  .  ")
    ],
    indent: auto,
)

#print-index(
    outlined: true,
    sorted: "up",
    title: [Listado de acrónimos],
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

= Resumen
En la actualidad es cada vez más la gente que pasa varias horas al día delante de un ordenador, nuestra principal herramienta para controlarlos es el teclado y, sin embargo, su diseño apenas ha avanzado desde su aparición. Este diseño arcaico supone problemas de salud, falta de accesibilidad y reduce la productividad.

Por esto, en el presente documento se estudian los avances y variaciones que ha realizado la comunidad de aficionados en los últimos años y se detalla el diseño y construcción de un teclado más acorde al mundo actual que solventa los problemas recién citados, así como permitiendo una gran capacidad de personalización gracias a su diseño hardware que permite cambiar componentes fácilmente y el desarrollo de un firmware y software _open source_ fácilmente editables.

<chapter:contents>

= Introducción <introducción>
#include "typ/intro.typ"

== Requisitos previos <requisitos-previos>
A lo largo del documento se asume cierto conocimiento de programación, así como tener algunos programas instalados. La mayoría son conocidos, pero aquí tenemos una lista con algunos menos populares y enlaces a sus respectivas páginas oficiales/documentación
- Rust @rust. Lenguaje de programación
- typst @typst. Lenguaje de marcado, parecido a LaTex, con el que se ha compuesto este documento.
- WSL @wsl. Herramienta de Windows, similar a una máquina virtual, para tener un entorno Linux

= Estado del arte <estado-del-arte>
#include "typ/state.typ"

= Desarrollo
#include "typ/development.typ"

== Hardware <hardware>

=== Cableado de las teclas <sec:scanning>

==== Conexión directa <conexión-directa>
La opción más sencilla que se nos puede ocurrir para conectar diversos interruptores a nuestro microcontrolador es soldarlos directamente a los pines de entrada/salida (GPIO). Para hacer esto tenemos 2 opciones:

#figure(
    grid(
        columns: (auto, auto),
        gutter: 1em,
        figure(
            image("images/pull_down.png"),
            caption: [Pull-Down],
            numbering: none,
            outlined: false,
        ),
        figure(
            image("images/pull_up.png"),
            caption: [Pull-Up],
            numbering: none,
            outlined: false,
        )
    ),
    caption: [Cableado directo]
)

Si hacemos esto, sin embargo, tendremos un problema pronto porque necesitaremos un chip con muchos pines de entrada/salida, o hacer un teclado con pocas teclas porque la cantidad de GPIOs es reducida.

==== Matriz <matriz>
Para solventar este problema, podemos cablear los botones mediante una matriz, usando un pin para cada fila y columna de teclas. Usamos una dimensión como salida y otra como entrada, haciendo un bucle que aplique voltaje en cada una de las filas y compruebe si las columnas reciben una entrada (tecla pulsada cerrando el circuito).
*Nota*: También se podría iterar en la otra dimensión

#figure(
    image("images/matrix.png", width: 35%),
    caption: [Cableado en matriz]
)

Este diseño también tiene sus problemas, el más notorio es el conocido como "efecto _ghosting_" en el que podemos detectar como pulsada una tecla que no lo está.

#figure(
    image("images/ghosting.png", width: 35%),
    caption: [Ghosting en una matriz]
)

En este ejemplo, la tecla *(1, 1)* se detecta como pulsada de manera correcta pero, al pulsar también las teclas *(0, 1)* y *(0, 0)*, estamos cerrando el circuito y generando que en la columna 1 llegue voltaje a la entrada, que será interpretado como que la tecla *(1, 0)* ha sido pulsada puesto que estamos en la iteración de la fila 0.

Este problema se solventa de forma sencilla, añadiendo unos diodos que bloqueen esta retroalimentación permitiendo detectar *(1, 1)* pero sin la pulsación falsa de *(1, 0)* del caso anterior.

#figure(
    image("images/anti_ghosting.png", width: 45%),
    caption: [Matriz anti-ghosting]
)

Aunque no es muy grave, para matrices muy desiguales, por ejemplo 5x20 teclas, no estamos usando eficazmente los pines, ya que para esas 100 teclas estamos empleando 25 pines mientras que una configuración 10x10 (20 pines) sería suficiente. En este caso, podriamos hacer una distribución de teclas en forma rectangular, pero luego cablearlas como dicha matrix cuadrada, sin embargo el diseño sería bastante más confuso.

Por último hay que tener en cuenta que, aunque el uso de los pines sea más óptimo, seguimos necesitando una cantidad de pines cada vez mayor conforme queramos añadir más teclas, aunque esto no debería ser un factor limitante en la mayoría de casos ya que este límite seguiría permitiendo una cantidad bastante elevada de teclas.

==== Lectura en serie <lectura-en-serie>
La opción que vamos a usar, inspirada en el _ghoul_#cite(<ghoul>), consiste en el uso de registros de desplazamiento conectados en una _daisy chain_, de esta forma vamos a emplear un único pin para leer todas las teclas en una señal serie, y otros pocos pines (unos 3 o 4) para controlar estos chips mediante SPI#cite(<spi>) o I2C#cite(<i2c>). De esta forma, podemos escanear potencialmente cualquier cantidad de teclas sin aumentar el números de pines necesarios, simplemente añadimos más registros a la cadena (aunque el escaneo se iría haciendo más y más lento)

=== Pantallas <pantallas>
En los últimos años es cada vez más común ver teclados que incorporan pequeñas pantallas, sin embargo, no son muy útiles ya que en la gran mayoría de casos se trata de SH1106 o SSD1306, que son dispositivos OLED de 2 colores y con una resolución bastante reducida, de 128x32 o 128x64 píxeles, en torno a la pulgada de diagonal. En nuestro teclado vamos a usar pantallas más potentes para poder mostrar información útil en vez de pequeños dibujos en estas pantallas más comunes.

=== Sensor táctil <sensor-táctil>
También es relativamente común encontrar diseños que incluyen diferentes sensores (por ejemplo joysticks analógicos o PMW3360) para mover el sensor por la pantalla del ordenador sin tener que mover la mano hasta el ratón. Tal como podemos ver en el Dactyl[@img:dactyl]

En nuestro caso, puesto que vamos a añadir una pantalla, podemos aprovechar y usar una táctil, de forma que nos sirva para mover el cursor pero tambíen para tener una pequeña interfaz de usuario en el teclado.

== Firmware <firmware>

=== Funcionalidad <funcionalidad>
La mejor parte de usar una librería tan extendida como lo es QMK es que tenemos muchas facilidades a la hora de escribir el codigo ya que buena parte del trabajo está hecho ya.

Esto incluye:
- Gestión sobre #acr("USB") para reportar distintos endpoints (teclado, ratón, multimedia)
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
    image("images/layout.png", width: 100%),
    caption: [Diseño aproximado del teclado]
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
    image("images/inverter.png", width: 30%),
    caption: [MOSFET como inversor]
)

#figure(
    image("images/piso.png", width: 80%),
    caption: [SN74HC589ADR2G para una fila]
)

==== Cableado de las pantallas <cableado-de-las-pantallas>
Vamos a usar una pantalla en el lado izquierdo (IL91874) y dos en el derecho (ILI9163 e ILI9341), estos dispositivos necesitan de conexión SPI así como pines extra (DC, CS, RST) para ser controlados. Dado que las vamos a conectar al mismo bus, solo necesitamos 3 pines para SPI y puesto que no podemos mandar información a dos pantallas simultáneamente, también pueden usar una línea DC común. Sin embargo las señales de DC y RST deben ser individuales y además la ILI9341 necesita dos señales DC, una para la pantalla y otra para el sensor táctil. Por tanto, también usaremos registros de desplazamiento, en este caso de entrada serie con salidas en paralelo (SN74HC595) y dichas salidas se conectaran a las entradas de control de las pantallas. Como ya teníamos un bus SPI configurado, usar estos registros tan solo supone el uso de otro pin (CS). Sin embargo, como necesitamos cambiar estas señales mientras estamos hablando con las pantallas, no podemos tener las dos cosas en el mismo bus, por lo que usaremos un bus para los registros (escanear teclas y señales de control) y otro bus para las pantallas. Lo mas importante de este diseño es que encadenando suficientes registros podríamos controlar tantas señales de control para diversos dispositivos como queramos, manteniendo mínima la utilización de pines del MCU.

== Firmware <firmware-1>

=== Instalación y configuración de QMK <instalación-y-configuración-de-qmk>
Para usar QMK#cite(<qmk>) instalamos su CLI ejecutando ya que se trata de una librería escrita en Python y está disponible en el repositorio de paquetes de Python(pip). Tras esto, descargamos el código fuente con , a este comando le podemos pasar como parámetro nuestro fork del repositorio (en mi caso ), para poder usar git, ya que no tendremos permisos en el repositorio oficial. Este comando también nos instalará los compiladores necesarios y comprobará las udev de nuestro sistema Linux, para que podamos trabajar con los dipositivos sin problema, en caso de que no estén bien configuradas podemos usar copiar el archivo en . Finalmente podemos ejecutar para comprobar el estado de QMK. Para poder hacer debug con he necesitado añadir una línea a dicho archivo:

Opcionalmente, podríamos instalar LVGL#cite(<lvgl>) funciones gráficas más complejas en la pantalla LCD, en vez de usar el driver de QMK. Esto se hace con (desde el directorio base de QMK). Seguidamente, usamos el código de jpe#cite(<lvgl-jpe>) para usar esta librería.

De momento no he implementado esta librería puesto que añadiría bastante complejidad al código y tampoco vamos a hacer ninguna interfaz especialmente llamativa.

==== Errores de compilación <errores-de-compilación>
Es posible que al intentar compilar obtengamos un error parecido a, este error se debe a un cambio en *gcc 12*. Seguramente estará solucionado en un par de meses con una actualización en QMK, pero de no ser así podemos revertir a *gcc 11* para que el mismo código se pueda compilar.

Pero también podemos arreglarlo manualmente con:
#cli(
    ```bash
    $ sudo pacman --needed -U https://archive.archlinux.org/packages/a/arm-none-eabi-gcc/arm-none-eabi-gcc-11.3.0-1-x86_64.pkg.tar.zst
    ```,
    caption: [Downgrade de GCC]
)

Una vez configurado QMK, creamos los archivos básicos para el firmware de nuestro teclado haciendo e introduciendo los datos necesarios, sin embargo esto crea una carpeta directamente en la ruta , en mi caso he creado una carpeta nueva bajo este directorio con mi nick (elpekenin) como nombre, y he movido la carpeta del teclado ahí dentro, de forma que si en un futuro diseño otro teclado se guarde en esta misma carpeta.

Para acelerar el proceso de compilado podemos guardar nuestra configuración (teclado y keymap) y el número de hilos que usará el compilador

#cli(
    ```bash
    $ qmk config user.keyboard=elpekenin/access
    $ qmk config user.keymap=default
    $ qmk config compile.parallel=20
    ```,
    caption: [Personalizar QMK]
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
    caption: [Definición de mensaje de usuario]
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
    caption: [Definición de nuevo evento]
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
    caption: [Emitir evento al recibir mensaje]
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
    caption: [Manejo del evento]
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
    caption: [Creación del _systray_]
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
    caption: [Añadir _systray_ al programa, con su logica]
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
    caption: [Interceptar señal de cierre de ventana]
)

==== Indicador de conexión <indicador-de-conexión>
Vamos a añadir también un indicador en la pantalla, de forma que Tauri nos haga saber cuando se conecta o desconecta del teclado, evitando que intentemos usar la pantalla cuando no va a funcionar. Editamos el eventlooop y definimos las nuevas funciones

#snippet(
    ```Diff
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
    caption: [Hooks de usuario]
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
    caption: [Hook de inicio]
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
    caption: [Compilar MicroPython para Linux]
)

#cli(
    ```bash
    $ cd build-standard
    $ ./micropython
    MicroPython 13dceaa4e on 2022-08-24; linux [GCC 12.2.0] version Use Ctrl-D to exit, Ctrl-E for paste mode
    >>>
    ```,
    caption: [Ejecutar MicroPython en Linux]
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
    caption: [Compilar MicroPython para RP2040]
)

Y ya podremos flashear este binario en nuestra placa de desarrollo


#bibliography(
    "bibliography.yml",
    title: "Bibliografía",
)

= Anexo B. Código fuente del informe (typst).

Aquí están adjuntas las primeras líneas del código fuente con el que he generado este documento. 

#text(size: 8pt)[
    #sourcerer.code(
        raw(
            read("report.typ"),
            lang: "typst"
        ),
        fill: rgb("#3F3F3F"),
        numbering: false,
    )
]