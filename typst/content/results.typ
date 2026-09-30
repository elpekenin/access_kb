#import "@elpekenin/tfm:0.1.0": images

Con el teclado completamente ensamblado, cargamos el firmware y comprobamos que todo funciona como se esperaba. Las integraciones con XAP y el M5 funcionan satisfactoriamente, obteniendo el siguiente ecosistema:
#figure(
  images.system,
  caption: [Conexiones del teclado],
)

Para la pantalla no podemos medir fotogramas por segundo ya que no se dibuja de forma periódica, sino que cada vez que se quiere alterar una zona, el código modifica directamente el área objetivo. En el código se ha incluido una función que dibuja el tiempo en línea (en segundos) a modo de prueba, se ha detectado que el refresco es perfectamente rápido para no ralentizar en exceso el teclado. Sin embargo, cambios grandes como dibujar una animación ralentizarían bastante y deben ser evitados.

Respecto a la frecuencia de escaneo de teclas, QMK permite mostrar esta métrica, en un intervalo de 5s de uso normal, se obtiene:
```
> matrix scan frequency: 864
> matrix scan frequency: 813
> matrix scan frequency: 768
> matrix scan frequency: 811
> matrix scan frequency: 767
> matrix scan frequency: 821
> matrix scan frequency: 860
> matrix scan frequency: 855
> matrix scan frequency: 861
> matrix scan frequency: 856
> matrix scan frequency: 864
```
Como vemos, la media es de unas 830 lecturas por segundo, incluso con el dibujo constante recién comentado. A pesar ser un valor relativamente pequeño, la latencia no supone una molestia al escribir y es cercana a lo mejor que se puede conseguir con QMK. La explicación es que el sistema operativo lee del teclado 1000 veces por segundo y, por tanto, estamos escaneando _casi_ tan rápido como podríamos reportar cambios.

Para cumplir con el objetivo de extensibilidad, se han expuesto multitud de GPIO y varios pines de alimentación que permitirían conectar componentes extra. Se han hecho pruebas satisfactorias con un debugger UART-USB o una pantalla I2C. Sin embargo, ha habido un fallo importante, se han quedado sin exponer los pines de SPI y los elementos que se añadan no se pueden conectar al bus.

El presupuesto del prototipo ha sido, aproximadamente:
- 5 placas: 40€
- Pantallas: 30€
- 70 interruptores: 20€
- Set de teclas: 10€
- 100 LEDs RGB: 10€
- Otros componentes (con sobrantes): 10€

Cada teclado rondar los 80€, si bien este precio es más elevado que un teclado de membrana, los teclados mecánicos comerciales raramente bajan de los 50€. Además, se podrían reducir gastos prescindiendo de alguna pantalla o la iluminación LED.
