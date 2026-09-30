A la vista de los resultados obtenidos y fallos encontrados en el prototipo construido, se proponen las siguientes mejoras para futuras revisiones:
- Analizar y optimizar el código para mejorar la frecuencia de escaneo. El propio código de escaneo y el dibujado en las pantallas son los principales puntos a estudiar.
- Exponer los pines SPI para permitir la conexión de dispositivos adicionales que usen este bus.
- Añadir puntos de prueba para poder hacer mediciones o debug de señales.- Los mount points, añadidos para poder anclar la placa a una caja, son para tornillos M2. Reemplazarlos por M3 haría más sencillo encontrar tornillería.
- La ubicación de pantallas, bajo las muñecas, resulta un poco incómoda. Sería mejor dejar en la zona baja la pantalla táctil (accesible con el pulgar) y mover el resto a la zona superior. Además, moverlas hacia el exterior reduciría el espacio entre los bloques de teclas en cada mitad del teclado.
- La iluminación de las pantallas LCD se ha conectado directamente a 3V3 para simplificar el diseño, en versiones posteriores se debería usar PWM para controlar el brillo y reducir consumo.
- La conexión entre mitades a veces falla porque el cable puede chocar con el borde de las placas. Se podrían mover los conectores hacia el borde o estudiar otras alternativas (por ejemplo: USB-C).
- Utilizar un MCU que tenga conectividad inalámbrica para conectarse directamente a los servicios y eliminando la necesidad de un segundo dispositivo o software en el ordenador para este fin. Un buen candidato para este uso es el NRF52840, soportado por ZMK y con un consumo energético reducido.
- La pantalla táctil es resistiva, por lo que su precisión es limitada. Es preferible usar un sensor capacitivo en su lugar.
