A la vista del contexto expuesto, la solución será el diseño, fabricación y programación de un teclado mecánico de código abierto, dotado de periféricos (pantallas, sensor táctil, ...) y con los mecanismos de comunicación necesarios para actuar como interfaz de control. Se proponen los siguientes requisitos:

- *Ergonomía y aprovechamiento del espacio.* Teclado split con una disposición ortolineal de las teclas, reduciendo el movimiento de los dedos y prescindiendo de las teclas menos utilizadas para obtener un teclado más compacto y simétrico, con una huella menor sobre la mesa.

- *Diseño y código abiertos.* Publicar con licencias libres el hardware (esquemas y diseño de la @pcb), firmware y software de control, junto con documentación para reproducir el proyecto. De forma que cualquier persona pueda fabricar su propio ejemplar, modificar la distribución de teclas, añadir funcionalidades o corregir fallos.

- *Funcionalidad más allá de la de un teclado convencional.* Desarrollar un sistema que, partiendo de la comunicación bidireccional con el entorno, permita integrar de forma sencilla nuevas utilidades: control domótico, notificaciones, control por voz, etc.

- *Extensibilidad.* Concebir el circuito para que sea posible ampliar sus capacidades añadiendo otros componentes sin fabricar una nueva @pcb. Para ello, deben exponerse @gpio y pines de alimentación que permitan conectar nuevos dispositivos.

- *Baja latencia.* Alcanzar una alta frecuencia de escaneo de las teclas, para que el retardo entre pulsar una tecla y ejecutar la acción asociada sea imperceptible. Este requisito es importante por un lado, para que escribir resulte cómodo; por otro, para que su uso como interfaz de control funcione en tiempo real.

- *Coste reducido.* Mantener el presupuesto en un nivel económicamente viable, empleando componentes de fácil adquisición y técnicas de fabricación estándar, de manera que el prototipo no suponga una barrera económica para replicarlo y sea una alternativa viable respecto a teclados comerciales.

Cabe señalar que algunos de estos objetivos entran en cierto conflicto: incorporar pantallas y otros periféricos consume pines del @mcu y tiempo de CPU, lo que afecta a la frecuencia de escaneo e incrementa el coste. El diseño deberá, por tanto, buscar un equilibrio razonable entre funcionalidad y rendimiento.
