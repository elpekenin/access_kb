Al diseñar la PCB hubo errores que debemos evitar repetir:
- *Test points*. Omitidos por la sencillez del diseño, por suerte no hubo fallos en el diseño, pero deberían haberse añadido para poder depurar problemas.
- *Mount points*. Se usaron M2, dificultando encontrar tornillos. M3 sería más accesible.
- *Conectores SPI*. Por un descuido no se expusieron SCK, MISO, MOSI junto a los otros GPIO. Esto impide usar extensibilidad SPI sin rediseño.
- *Posición de pantallas*. En su ubicación actual, bajo las muñecas, resultan un poco incómodas; estarían mejor en una zona alejada.
- *Conector jack*. Moverlo hacia el exterior, el cable choca un poco con el borde de la placa en la posición actual.

Hay cosas que no se han hecho en este primer dispositivo para simplificar el proyecto, que quedan pendientes de investigar e implementar
- *Backlight*. La iluminación de las pantallas se ha conectado a 3V3, se podría usar PWM o un GPIO como interruptor, para poder apagarlas y reducir consumo.
- *Pantalla táctil*. El sensor empleado es resistivo y no proporciona buena precisión, uno capacitivo sería más caro y complejo pero daría mejores lecturas.
- *LVGL*. Los diseños mostrados en la pantalla son muy básicos, pero usando @lvgl podría hacerse una interfaz más vistosa y con más capacidades. QMK proporciona integración con esta librería
- *Diseño inalámbrico*. Podría utilizarse un MCU que proporcione conectividad mediante WiFi, BLE, Zigbee, ... Pudiendo conectarse directamente con los servicios necesarios y reduciendo (o eliminando) la necesidad de un software ejecutando en el ordenador. Un buen candidato para este uso es el NRF52840, soportado por ZMK y con un consumo energético bastante reducido
- *Montaje*. Dado que la fábrica obliga a pedir un mínimo de 5 placas, se ha empleado una segunda PCB para dar cuerpo al teclado, usando separadores entre ellas. En futuras iteraciones sería preferible diseñar e imprimir en 3D una caja donde guardar nuestra electrónica.
