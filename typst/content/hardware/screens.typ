En los últimos años se ha hecho común encontrar teclados con pequeños displays. Generalmente son OLED de 2 colores (SH1106/SSD1306) y con una resolución bastante reducida (128x32 o 128x64) píxeles, en torno a la pulgada de diagonal.

En nuestro diseño incorporamos pantallas más potentes para poder mostrar más información y que resulten más útiles. IL91874 en el lado izquierdo
y ILI9163 +ILI9341 en el derecho.

Estas pantallas se controlan mediante SPI y algunos pines extra (DC, CS, RST). Al estar en el mismo bus, necesitamos:
- 3 pines para SPI
- Podemos conectar DC en paralelo (ya que no podemos mandar información a varias pantallas simultáneamente, estando en el mismo bus)
- Señales de CS y RST individuales para cada pantalla
  - La ILI9341 necesita otro CS, para el sensor táctil incoporado en el modelo utilizado

Dado que esto consumiría muchos pines, usaremos registros de desplazamiento SIPO (SN74HC595) cuyas salidas serán las señales de control de las pantallas. Estos registros se conectarán al mismo bus que los PISO que escanean las teclas y solo requiren una conexión extra (su señal CS). Para los datos enviados a las pantallas necesitaremos un segundo bus SPI, esto no supone mayor problema ya que el RP2040 tiene 2 componentes hardware para dicho protocolo.

De forma análoga al escaneo de teclas, con este diseño podríamos añadir más pantallas (o cualquier otra señal de salida), sin necesidad de gastar más pines del MCU, simplemente conectamos más registros en serie
