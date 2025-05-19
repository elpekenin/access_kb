#import "@elpekenin/tfm:0.1.0": cli, snippet

Durante el desarrollo del proyecto, barajé usar MicroPython y aunque terminé descartando la opción, creo que puede ser interesante para las prácticas de alguna asignatura de electrónica o programación en la Universidad, puesto que es más amigable que C o C++.

MicroPython es una implementación en C del lenguaje Python diseñada para su uso en microcontroladores, enfocándose en reducir al mínimo el consumo de recursos a costa de perder algunas funcionalidades.

== Compilar para RP2040
Primero instalamos la toolchain para la arquitectura del procesador, en este caso `arm-none-eabi-gcc`, el comando para hacer esto varía según el gestor de paquetes de la distribución utilizada.

Cabe destacar que podemos añadir más código al firmware. Por ejemplo, en las pruebas realizadas, añadí configuración para reportar y usar un endpoint HID -es decir: funcionar como un teclado- siguiendo y adaptando la información encontrada en @tusb-rp2:
- Definimos en C el módulo @mod-usb-hid, lo más importante del código es el uso de `MP_REGISTER_MODULE` para que podamos hacer `import` sobre él.
- Editar configuración del sistema de compilación (CMake), para que añada el nuevo archivo
#snippet(
  ```diff
  set(MICROPY_SOURCE_PORT
  + modusb_hid.c
    fatfs_port.c

  set(MICROPY_SOURCE_QSTR
  + ${PROJECT_SOURCE_DIR}/modusb_hid.c
    ${MICROPY_SOURCE_PY}
  ```,
  caption: [Configurar compilación de archivo],
)

Por último, compilamos el binario
#cli(
  ```bash
  $ cd ports/rp2    # carpeta con codigo especifico para esta MCU
  $ make submodules # descargar dependencias
  $ make            # compilar
  ```,
  caption: [Compilar MicroPython para RP2040],
)

Y ya podremos flashear este binario en nuestra placa de desarrollo, para RP2040 esto es tan simple como copiar el archivo UF2.

A partir de aquí podemos usar algún IDE para modificar el código en el sistema de archivos interno del dispositivo, sin necesidad de compilar otra vez.

== Probar en Linux
Si queremos hacer pruebas en un ordenador, podemos compilarlo de forma similar, pero haciendo `cd` a `ports/unix`.

Esto sería útil para comprobar si el código que queremos emplear (por ejemplo, alguna librería) funcionará en nuestra MCU, ya que MicroPython implementa un *sub*-conjunto de CPython (implementación estándar del lenguaje)

Una vez compilado, podremos ejecutarlo como haríamos con la versión "normal".
#cli(
  ```bash
  $ cd build-standard  # carpeta donde se guarda el ejecutable
  $ ./micropython      # abrimos en modo interactivo
  MicroPython <COMMIT> on <YYYY-MM-DD>; linux [GCC <X.Y.Z>] version Use Ctrl-D to exit, Ctrl-E for paste mode
  >>>
  ```,
  caption: [Ejecutar MicroPython en Linux],
)
