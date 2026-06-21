#import "@elpekenin/tfm:0.1.0": h, snippet

Para hacer al teclado independiente del programa de control en el ordenador, y permitir mejor conexión con el resto del ecosistema, se le ha conectado un M5 Atom @atom. Este dispositivo está basado en un ESP32 e integra luz RGB, micrófono y altavoz. Se conectará mediante UART al teclado y se encarga de enviar mediante MQTT los mismos eventos que el teclado envía por XAP y transmitir por un WebSocket el flujo de audio capturado por el micrófono. El LED se utiliza para mostrar el estado (azul: grabando, apagado: inactivo), el altavoz por ahora queda sin uso.

#h[Firmware][
  Para desarrollar el código, empleamos la librería proporcionada por el fabricante @m5unified. Es compatible tanto con Arduino @arduino como ESP-IDF @esp-idf. Usamos la primera opción ya que proporciona más funcionalidades y librerías, agilizando la implementación.

  Para notificar de los eventos, el teclado creará la estructura de datos y la envía por UART
  #snippet(
    ```c
    void m5_send(const void *data, size_t data_len) {
      const uint8_t *ptr = data;
      if (IS_ENABLED(M5_DEBUG)) {
        printf("[m5] sending: {");
        for (size_t i = 0; i < data_len; ++i) {
            printf(" %d", ptr[i]);
        }
        printf(" }\n");
      }

      uart_transmit(ptr, data_len);
    }

    const screen_pressed_msg_t msg = make_screen_pressed(touch_sensor_id, touch_sensor_reading);
    m5_send(&msg, sizeof(msg));
    ```,
    caption: [Envío de evento al M5],
  )

  Por su parte, el M5 leerá los mensajes recibidos y los publicará en MQTT
  #snippet(
    ```c
    // en MQTT, los mensajes son cadenas de texto
    // esta función permite generarlos cómodamente, parecido a `printf`
    void mqtt_send(const char *topic, const char *fmt, ...) {
      char buffer[64];

      va_list va;

      va_start(va, fmt);
      vsnprintf(buffer, sizeof(buffer), fmt, va);
      va_end(va);

      mqtt.publish(topic, buffer);
    }

    // mirando el primer byte (id), sabemos el tipo de mensaje
    switch (data[0]) {
    case SCREEN_PRESSED: {
      // interpretamos los bytes acorde al mensaje en cuestión
      const screen_pressed_msg_t *msg = (screen_pressed_msg_t *)data;

      // enviamos la información al broker
      mqtt_send("event/screen_pressed", "%d|%d|%d", msg->screen_id, msg->x, msg->y);
      break;
    }
    ```,
    caption: [Re-envío de eventos a MQTT],
  )
]

#h[Servidor][
  Para procesar el audio grabado por el micrófono vamos a hacer un servidor al que el M5 se conectará.

  Este programa irá almacenando la información recibida hasta que se detecte silencio (el usuario ha dejado de hablar) o la grabación sea considerablemente larga.
  #snippet(
    ```py
    def add(self, raw: bytes) -> State:
      """Handle information coming from microphone."""
      chunk = np.frombuffer(raw, dtype=np.int16)
      self._buf.append(chunk)

      # si el nivel medio es bajo -> silencio
      if np.abs(chunk).mean() > _THRESHOLD:
        self._in_speech = True
        self._silence_counter = 0
      else:
        self._silence_counter += len(chunk)

      # este check evita almacenar demasiada información
      samples = sum(len(item) for item in self._buf)
      if samples >= _MAX_SAMPLES:
        return {
          "action": "speech",
          "data": self._flush(),
        }

      # se detectó sonido pero llevamos un rato en silencio
      if self._in_speech and self._silence_counter >= _SILENCE_SAMPLES:
        seg = self._flush()
        if len(seg) < _MIN_SAMPLES:
          return {"action": "silence"}

        return {
          "action": "speech",
          "data": seg,
        }

      return {"action": "listening"}
    ```,
    caption: [Procesado del audio],
  )

  #snippet(
    ```py
    async def handler(connection: websockets.ServerConnection) -> None:
      """Implement handling of M5 messages."""
      buffer = AudioBuffer()

      asr = onnx_asr.load_model(model="nemo-parakeet-tdt-0.6b-v3")

      client = mqtt.Client(client_id="mic_server")
      client.connect(BROKER_IP, BROKER_PORT, keepalive=60)
      client.loop_start()

      async for msg in connection:
        if not isinstance(msg, bytes):
            continue

        state = buffer.add(msg)
        if state["action"] == "silence":
          continue

        if state["action"] == "listening":
            continue

        waveform: npt.NDArray[np.float32] = (state["data"] / 32768).astype(np.float32)
        prompt = asr.recognize(waveform).strip()
        if not prompt:
            continue

        client.publish("event/user_mic", prompt)


    server = await websockets.serve(handler, address, port)
    await server.wait_closed()
    ```,
    caption: [Lógica del servidor],
  )

  Terminada la grabación, ejecutamos un modelo ASR #footnote[Automatic Speech Recognition] para transcribirla a texto. El resultado obtenido se publica por MQTT a un topic que se usa de prompt para un LLM #footnote[Large Language Model] local que coordina y controla todos los elementos del sistema.
]
