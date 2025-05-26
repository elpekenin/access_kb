#import "@preview/big-todo:0.2.0": todo

#import "@elpekenin/tfm:0.1.0": cli, snippet, config

La idea principal es que el ordenador actúe como el orquestador del sistema.
- Por un lado, recibirá eventos del teclado, como una pulsación en la pantalla táctil, para ejecutar acciones tales como encender una luz.
- Por otro, se podrán monitorizar valores como la temperatura o la previsión de precipitaciones y mostrarlos en pantalla.

Para comunicar el ordenador con el teclado utilizaremos @xap (⚠ aún en desarrollo). Este protocolo, definido por QMK, funciona sobre HID y permite el intercambio de información arbitraria. Lo más importante es que evita problemas con los drivers del sistema operativo utilizando un endpoint adicional; es decir, se dispone de un segundo flujo de datos independiente de la comunicación convencional de "el usuario ha pulsado esta tecla".

En el cliente, extenderemos la funcionalidad de @qmk_xap. Este programa utiliza el framework Tauri, cuya filosofía es reutilizar el mismo código en diferentes sistemas. Esto se logra modelando la interfaz gráfica como una web e integrando dicha página en el binario, junto con un navegador para visualizarla.

Siempre que sea posible, ejecutaremos la lógica en el frontend; de este modo, evitamos tener que recompilar el backend (proceso lento) y obtenemos un lenguaje más sencillo de usar (TypeScript en vez de Rust), lo que agiliza el desarrollo de nuevas funcionalidades.

=== Instalación
Para el _backend_ podríamos instalar Rust directamente, pero se recomienda usar `rustup`, ya que permite gestionar las instalaciones. Con este instalador obtendremos la última versión del compilador para nuestro sistema y arquitectura.

Para el frontend, instalamos Node (intérprete de JavaScript) y lo usamos para instalar el gestor de paquetes `yarn`. Una vez hecho esto, podremos instalar las librerías necesarias.
#cli(
  ```bash
  curl --proto "=https" --tlsv1.2 -sSf https://sh.rustup.rs | sh
  rustup toolchain install stable

  sudo apt update
  sudo apt install nodejs
  npm install --global yarn

  git clone https://github.com/qmk/qmk_xap
  cd qmk_xap
  yarn install

  yarn run dev
  ```,
  caption: [Instalación de cliente XAP],
)

=== Manejo de mensajes personalizados
Lo primero que debemos hacer es definir, mediante un `enum`, cada tipo de mensaje que se puede recibir, identificado con un `u8`. El contenido de cada uno de dichos mensajes se define con un `struct`.
#snippet(
  ```rust
  // uno de los distintos mensajes que se van a recibir
  #[derive(BinRead, Debug)]
  pub struct ScreenPressed {
      pub x: u16,
      pub y: u16,
  }

  //
  // ... los demás mensajes ...
  //

  // el tipo de mensaje recibido se indica en el primer byte de su contenido
  // esta información la indicamos con el atributo `br(magic)`
  // con el atributo `serde` seremos capaces de distinguir los mensajes en el frontend
  #[derive(BinRead, Debug, Clone)]
  #[serde(tag = "kind", content = "data")]
  pub enum UserBroadcast {
      #[br(magic = 0u8)]
      ScreenPressed(ScreenPressed),
      #[br(magic = 1u8)]
      ScreenReleased(ScreenReleased),
      #[br(magic = 2u8)]
      LayerChanged(LayerChanged),
      #[br(magic = 3u8)]
      KeyEvent(KeyEvent),
  }

  // necesario para conversión entre tipos
  impl XapBroadcast for UserBroadcast {}
  ```,
  caption: [Definición de los mensajes de usuario],
)

A continuación, para que la aplicación pueda ejecutar lógica en base a estos mensajes, emitiremos un evento al recibirlos y tendremos código encargado de esperar y manejar dichos eventos en el frontend.
#snippet(
  ```diff
  --- a/src-tauri/src/rpc/events.rs
  +++ b/src-tauri/src/rpc/events.rs
  @@ @@ pub enum XapEvent {
       RemovedDevice {
           id: Uuid,
       },
  +    UserBroadcast(xap_specs::broadcast::UserBroadcast),
    }

  --- a/src-tauri/src/xap/client.rs
  +++ b/src-tauri/src/xap/client.rs
  @@ @@ impl XapClient {
            });
        }
        BroadcastType::Keyboard => error!("keyboard broadcasts are not implemented!"),
  -     BroadcastType::User => error!("user broadcasts are not implemented!"),
  +     BroadcastType::User => {
  +         events.push(XapEvent::UserBroadcast(
  +             broadcast.into_xap_broadcast()?,
  +         ));
  +     }
    }
  ```,
  caption: [Evento de mensaje recibido],
)

#snippet(
  ```ts
  import mitt, { Emitter } from "mitt"

  import { UserEvent } from "@generated/xap"

  // de momento, un solo tipo de evento: mensaje recibido del teclado
  type UserEvent = {
      broadcast: UserBroadcast,
  }

  // nuestro bus de eventos
  export const events: Emitter<UserEvent> = mitt<UserEvent>()

  // función que envía los eventos a la cola
  async function xapHandler(event: XapEvent) {
      if (event.kind == "UserBroadcast") {
          events.emit("broadcast", event.data)
      }
  }

  // registramos la función para que "escuche" en el bus de QMK
  eventBus.on("xap", xapHandler)

  // ahora podríamos suscribirnos a estos eventos
  events.on("broadcast", (ev) => {
    switch (ev.kind) {
      // código
    }
  )
  ```,
  caption: [Manejo del evento en frontend],
)

=== Variables de entorno
Para proteger información sensible (contraseñas, IP, tokens, ...) y evitar que aparezca en el código, almacenaremos estos parámetros como variables de entorno. Este método es bastante común y robusto, pero supone un problema en nuestra aplicación ya que el frontend no tiene acceso al sistema. La solución consiste en desarrollar un pequeño plugin para Tauri (en el directorio `tauri-plugin-env/` del repositorio), gracias al cual podremos solicitar la información al backend. Obviando pequeños detalles, el código se reduce a:
#snippet(
  ```rs
  // función invocada para obtener una variable
  #[tauri::command]
  async fn get<R: Runtime>(key: String, _app: AppHandle<R>) -> Result<String, String> {
      let ret = std::env::var(key).map_err(|e| e.to_string())?;

      Ok(ret)
  }

  // configurar plugin, cargando opcionalmente un archivo `.env`
  pub fn init<R: Runtime>() -> TauriPlugin<R> {
    #[cfg(feature = "dotenv")]
    let _ = dotenvy::dotenv();

    Builder::new("env")
        .invoke_handler(tauri::generate_handler![get])
        .setup(|app, _| {
            app.manage(EnvState::default());
            Ok(())
        })
        .build()
  }
  ```,
  caption: [Plugin para leer variables de entorno],
)

#snippet(
  ```diff
  --- a/src-tauri/src/main.rs
  +++ b/src-tauri/src/main.rs
  @@ @@
      tauri::Builder::default()
          .invoke_handler(specta_builder.invoke_handler())
          .plugin(shutdown_event_loop());
  +       .plugin(tauri_plugin_env::init())
          .setup(move |app| {
  ```,
  caption: [Instalar plugin en Tauri],
)

#snippet(
  ```ts
  import { invoke } from "@tauri-apps/api/core"

  type Result<T, E> =
      | {status: "ok", data: T}
      | {status: "error", error: E}

  export async function get(key: string): Promise<Result<string, string>> {
      try {
        return {status: "ok", data: await invoke("plugin:env|get", { key }) }
      } catch (e) {
        return {status: "error", error: e as any}
      }
  }
  ```,
  caption: [TypeScript para invocar el comando del plugin],
)

#if (config.render_todos) {
  todo("Añadir imagen/video IRL")
}
