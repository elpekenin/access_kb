#if defined (TOUCH_SCREEN)
uint32_t touch_timer = 0;
void housekeeping_task_user(void) { // Se ejecuta en todas las iteraciones del bucle principal
    if (touch_device == NULL) // Esperamos a que el dispositivo haya sido inicializado
        return;

    if (timer_elapsed32(touch_timer) < 200) // Con un timer controlamos la frecuencia de muestreo
        return;
    touch_timer = timer_read32();

    touch_report_t touch_report = get_spi_touch_report(touch_device); // Leemos el sensor
#    if defined(XAP_ENABLE) // Si XAP activo, enviamos las coordenadas al PC
    static bool release_notified = true;
    if (touch_report.pressed) {
        uint8_t payload[4] = { touch_report.x & 0xFF, touch_report.x >> 8, touch_report.y & 0xFF, touch_report.y >> 8 };
        // El identificador 0x03 significa que es un mensaje del usuario
        xap_broadcast(0x03, payload, sizeof(payload));

        release_notified = false;
    }

    else if (!release_notified) {
        // Mandamos un mensaje "falso" para limpiar la interfaz
        uint8_t payload[4] = { 0, 0, 0 , 0};
        xap_broadcast(0x03, payload, sizeof(payload));

        release_notified = true;
    }
#    endif // XAP_ENABLE
}
#endif // TOUCH_SCREEN
