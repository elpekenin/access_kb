#import "@elpekenin/tfm:0.1.0": snippet

La idea principal del control de la pantalla es que el teclado no dibuje en ella nada, de forma que la controlemos exclusivamente desde el programa desarrollado. De esta manera, lo que haremos será leer el estado del sensor táctil cada 200ms y, en caso de estar pulsado, enviaremos las coordenadas de dicha pulsación. Adicionalmente enviamos otro mensaje como si se hubiera pulsado la posicion (0, 0), donde no hay ninguna lógica, para que se "limpie" la pantalla al acabar una pulsación.

El código relevante es:

Al igual que hemos dicho antes con añadir o no añadir un archivo a nuestro código según la configuración, debemos hacer lo mismo con bloques de código, usando de forma que no intentemos usar la pantalla táctil o enviar un mensaje XAP si estas características no se han activado.

== Software
Para poder intercambiar información con el teclado de forma que podamos configurarlo o enviarle información en vez de simplemente escuchar las teclas que se han pulsado, vamos a desarrollar un programa en Tauri (librería escrita en Rust) ya que permite usar el mismo código en multitud de sistemas operativos gracias a que funciona internamente con un servidor HTML.

=== Instalación
Para instalar Rust podemos usar pacman , sin embargo para desarrollar código es preferible usar un script que nos proporciona la comunidad del lenguaje, y que permite cambiar fácilmente la versión del lenguaje con la que compilamos, tan sólo necsitamos ejecutar

La comunicación con el teclado se realiza usando la librería hidapi @hidapi, a la que accedemos desde Rust gracias al wrapper @hidapi-rs que implementa una "pasarela" a la librería en C. Para instalarla tan solo necesitamos añadirla al archivo de nuestro proyecto

Primero instalamos NodeJS con , después clonamos el repositorio @qmk_xap, usado de base (y en el que he colaborado) para desarrollar el software. Para instalar las dependencias de JavaScript ejecutamos . Ahora ya podemos correr para lanzar nuestro programa.

He tenido que desactivar la opción wgl(libreria de Windows para OpenGL) en VcXsrv para que funcionase

=== Desarrollo

==== Escuchar nuestros mensajes
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

Toda la lógica de usuario (personalizable), se puede encontrar en el archivo @user-rs

==== Correr aplicación en segundo plano
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

==== Indicador de conexión
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


==== Arrancar HomeAssistant
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
