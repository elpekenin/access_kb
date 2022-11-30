pub(crate) fn on_device_connection(device: &XAPDevice) {
    // Necesitamos esperar un poco para que el teclado pueda escucharnos
    std::thread::sleep(std::time::Duration::from_millis(280));

    // Show connection
    let _ = device.query(PainterDrawTextRecolor(
        PainterTextRecolor {
            dev: SCREEN_ID,
            x: 15,
            y: 15,
            font: 0,
            fg_color: FG_COLOR,
            bg_color: BG_COLOR,
            text: "Connected to Tauri".into(),
        }
    ));

    // Print buttons
    for id in 0..N_BUTTONS {
        let _ = device.query(PainterDrawImageRecolor (
            PainterImageRecolor {
                dev: SCREEN_ID,
                x: BUTTONS_X[id],
                y: BUTTONS_Y[id],
                img: BUTTON2IMG[id],
                fg_color: FG_COLOR,
                bg_color: BG_COLOR,
            }
        ));
    }
}


pub(crate) fn on_close(state: Arc<Mutex<XAPClient>>) {
    for device in state.clone().lock().get_devices() {
        // Clear screen
        let _ = device.query(PainterDrawRect (
            PainterRect {
                dev: SCREEN_ID,
                left: 0,
                top: 0,
                right: SCREEN_WIDTH,
                bottom: SCREEN_HEIGHT,
                color: BG_COLOR,
                filled: 1
            }
        ));

        // Show text
        let _ = device.query(PainterDrawTextRecolor(
            PainterTextRecolor {
                dev: SCREEN_ID,
                x: 15,
                y: 15,
                font: 0,
                fg_color: FG_COLOR,
                bg_color: BG_COLOR,
                text: "Tauri app was closed".into(),
            }
        ));
    };
}
