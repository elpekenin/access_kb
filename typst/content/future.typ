Al diseñar la PCB hubo errores que debemos evitar repetir:
- *Test points*. Omitidos por la sencillez del diseño y las pruebas previas. No hubo fallos en el diseño y no hicieron falta, pero deberían haberse añadido para poder depurar problemas.
- *Mount points*. Se usaron M2, dificultando encontrar tornillos. M3 sería más accesible.
- *Conectores SPI*. Por un descuido no se expusieron SCK, MISO, MOSI; como sí se hizo con los pines libres. Esto impide usar dispositivos SPI extra.
- *Posición de pantallas*. En su ubicación actual, bajo las muñecas, resultan un poco incómodas; estarían mejor en una zona alejada.
- *Conexión entre mitades*. Mover jack el hacia el exterior porque el cable choca un poco con el borde de la placa en la posición actual. Barajar otras conectores.

Hay otras ideas que no se han hecho en este primer dispositivo para simplificar el proyecto, pero resultan interesantes
- *Backlight*. La iluminación de las pantallas se ha conectado a 3V3, se podría usar PWM o un GPIO como interruptor, para poder apagarlas y reducir consumo.
- *Pantalla táctil*. El sensor empleado es resistivo y no proporciona buena precisión, uno capacitivo daría mejores lecturas.
- *LVGL*. Los diseños mostrados en la pantalla son muy básicos, pero usando @lvgl podría hacerse una interfaz más vistosa y con más capacidades.
- *Diseño inalámbrico*. Podría utilizarse un MCU que proporcione conectividad mediante WiFi, BLE, Zigbee, ... Pudiendo conectarse directamente con los servicios necesarios y reduciendo (o eliminando) la necesidad de un software ejecutando en el ordenador. Un buen candidato para este uso es el NRF52840, soportado por ZMK y con un consumo energético bastante reducido
- *Montaje*. Dado que la fábrica obliga a pedir un mínimo de 5 placas, se ha empleado una segunda PCB para dar cuerpo al teclado, usando separadores entre ellas. En futuras iteraciones sería preferible diseñar e imprimir en 3D una caja donde guardar nuestra electrónica.
