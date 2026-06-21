#import "@elpekenin/tfm:0.1.0": cli, h, snippet, vars

El ordenador actuará como gestor
- Recibe eventos del teclado, como la pulsación de una tecla o un toque en la pantalla táctil, para ejecutar acciones tales como encender una luz.
- Monitoriza valores como la temperatura o el número de emails sin leer y los muestra en las pantallas.

Usaremos XAP para comunicación entre teclado y ordenador mediante `qmk_xap` @qmk_xap, el cliente oficial de QMK. Este programa usa Tauri @tauri para obtener código multiplataforma (Mac, Android, ...), esto se consigue implementando la interfaz gráfica como una aplicación web y generando binarios que integran un navegador.

Siempre que sea posible ejecutaremos la lógica en el frontend de modo que evitemos recompilar el backend (proceso lento), otro beneficio es que usaremos un lenguaje más sencillo (TypeScript vs Rust)

#h[Instalación][
  Para el _backend_ podríamos instalar Rust @rust directamente, pero se recomienda usar `rustup` @rustup ya que permite gestionar las instalaciones. Con este instalador obtendremos la última versión del compilador para nuestro sistema y arquitectura.

  Para el frontend, instalamos Node @node y lo usamos para instalar `yarn` @yarn. Una vez hecho esto, podremos instalar las librerías necesarias.
  #cli(
    ```bash
    # descargar rustup y la toolchain para nuestro sistema
    curl --proto "=https" --tlsv1.2 -sSf https://sh.rustup.rs | sh
    rustup toolchain install stable

    # instalar node y yarn
    sudo apt update
    sudo apt install nodejs
    npm install --global yarn

    # descargar el repositorio de qmk_xap y sus dependencias
    git clone https://github.com/qmk/qmk_xap
    cd qmk_xap
    yarn install

    # ejecutar qmk_xap
    yarn run dev
    ```,
    caption: [Instalación de cliente XAP],
  )
]

#h[Eventos del teclado][
  Primero, definimos los mensajes que puede enviar el teclado de forma autónoma (vistos en @xap:broadcast). Esto se hace con un `enum` donde cada tipo de mensaje se define con su identificador y los parámetros que lo componen.
  #snippet(
    ```rust
    // uno de los mensajes que se van a recibir
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
    caption: [Recepción mensaje XAP],
    size: 7pt,
  )

  Para poder ejecutar lógica al recibir estos mensajes, emitimos un evento en el frontend que las diversas funcionalidades podrán recibir para actuar en consecuencia
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
    size: 8pt,
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

    // registramos la función para que "escuche" en el bus de qmk_xap
    eventBus.on("xap", xapHandler)

    // ahora podríamos suscribirnos a estos eventos en cualquier parte del frontend
    events.on("broadcast", (ev) => {
      switch (ev.kind) {
        // código
      }
    )
    ```,
    caption: [Manejo del evento en frontend],
    size: 9pt,
  )
]

#h[Control del teclado][
  Para hacer uso de los mensajes definidos en `xap.hjson` @xap:hjson, copiamos el archivo en `xap-specs/assets/xap_user.hjson` y ejecutamos `cargo run` desde `xap-specs`. Con esto, la herramienta que genera código Rust para definir los mensajes "nativos" de XAP hará el mismo proceso con los nuestros.

  En este momento, desde el frontend ya podremos ejecutar estas operaciones
  #snippet(
    ```ts
    function toAsciiArray(str: string): number[] {
      return Array.from(str, c => c.charCodeAt(0))
    }

    // NOTA: la API no es 100% legible por ser autogenerada
    // podríamos hacer un pequeño wrapper para nombre más legible, aceptar string y rellenar el terminador
    const ret = xap.commands.quantumPaintergetGeometry(keyboard_id, {
      device_name: toAsciiArray("ili9341"),
      dev_terminator: 0,
    })

    if (ret.status === "error") {
      console.error("could not send message: ", ret.error)
    }
    ```,
    caption: [Envío mensaje XAP],
    size: 9pt,
  )
]

#h[Variables de entorno][
  Para proteger información sensible (contraseñas, IP, tokens, ...), las almacenamos como variables de entorno, necesitaremos un plugin para leerlas desde el frontend ya que estarán definidas en el backend.
  #snippet(
    ```rs
    // comando para obtener una variable
    #[tauri::command]
    async fn get<R: Runtime>(key: String, _app: AppHandle<R>) -> Result<String, String> {
      let ret = std::env::var(key).map_err(|e| e.to_string())?;

      Ok(ret)
    }

    // inicialización del plugin
    pub fn init<R: Runtime>() -> TauriPlugin<R> {
      // funcionalidad opcional para leer un archivo `.env`
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

  Y lo añadimos a nuestra aplicación
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
    caption: [Añadir plugin],
  )

  #block(breakable: false)[
    Lo usaremos en el frontend con la siguiente función
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
      caption: [Ejecutar plugin desde frontend],
    )
  ]
]

#h[Integración][
  En este punto, podemos hacer prácticamente cualquier cosa que se nos ocurra, algunas ideas:
  - Graficar el uso de CPU y RAM del ordenador en el teclado
  - Mostrar la fecha y hora, o predicción meteorológica
  - Ver cámara de la mirilla en la pantalla y abrir la puerta pulsando pantalla táctil/tecla
  - Mostar nombre y/o carátula de la canción que está sonando, junto con controles multimedia

  Como ejemplo se adjunta un código que permite usar la pantalla táctil para controlar una luz conectándonos a la API de Home Assistant @hasst. Cuando se pulsa la pantalla en un rango de coordenadas definido se encenderá la bombilla y al soltarla se apagará.
  #snippet(
    ```ts
    // credencial de acceso a home assistant
    const token = await env.get("HASST_TOKEN")
    if (token.status === "error") {
        console.error("could not get HASST_TOKEN from env:", token.error)
        return
    }

    // direccion IP y puerto donde tenemos instalado el servicio
    const base_url = await env.get("HASST_BASE_URL")
    if (base_url.status === "error") {
        console.error("could not get HASST_BASE_URL from env:", base_url.error)
        return
    }

    // identificador de la bombilla
    const lightbulb_id = await env.get("HASST_LIGHTBULB")
    if (lightbulb_id.status === "error") {
        console.error("could not get HASST_LIGHTBULB from env:", lightbulb_id.error)
        return
    }

    // nos suscribimos al bus de eventos, solo actuaremos al pulsar o soltar la pantalla
    elpekenin.events.on("broadcast", async event => {
      switch (event.kind) {
        case "ScreenPressed":
          const data = event.data

          // pulsar otra pantalla (si hubiera varias) -> nada
          if (data.screen_id !== config.screen_id) return

          // pulsar fuera del rango definido -> nada
          if (data.x < config.x.min || data.x > config.x.max) return
          if (data.y < config.y.min || data.y > config.y.max) return

          // encender la luz
          await fetch(`${base_url.data}/api/services/light/turn_on`, {
            method: "POST",
            headers: {
              authorization: `Bearer ${token.data}`,
              "content-type": "application/json",
            },
            body: JSON.stringify({
              entity_id: lightbulb_id.data,
            })
          })

          break

        case "ScreenReleased":
          // otra pantalla -> nad
          if (event.data.screen_id !== config.screen_id) return

          // apagar la luz
          await fetch(`${base_url.data}/api/services/light/turn_off`, {
            method: "POST",
            headers: {
              authorization: `Bearer ${token.data}`,
              "content-type": "application/json",
            },
            body: JSON.stringify({
              entity_id: lightbulb_id.data,
            })
          })

          break

        default:
          break
      }
    })
    ```,
    caption: [Control domótico desde teclado],
    size: 9pt,
  )
]
