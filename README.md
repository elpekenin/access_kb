Access keyboard
===============
This project's goal is to make a highly customizable keyboard with a bunch of accesibility add-ons, so that **most** people can use it.

File structure:
```
📂 access_kb
├─ 📂firmware/     - Code running on the keyboard
|  ├─  micropython/   * Python implementation for microcontrollers
|  └─  tinyusb/       * USB stack
├─ 📂hardware/     - PCB files
|  └─ 📂libraries/     * References to KiCAD symbols and footprints 
└─ 📂latex/        - Sources used to create the PDF report
   ├─ 📂images/        * Various visual resources 
   ├─ 📂snippets/      * Relevant pieces of code and configuration
   ├─ 📂tex/           * Fragments of the complete writing
   ├─  conf.tex       * LaTeX configuration
   └─  main.tex       * Wrapper file which joins everything together
```
Firmware
========
The firmware runs [MicroPython](https://micropython.org/) using [TinyUSB](https://docs.tinyusb.org/en/latest/) for HID emulation, this implementation was based upon `noobeepi`'s [work](https://github.com/noobee/micropython/tree/usb-hid). My code was made inspired by [QMK](https://github.com/qmk/qmk_firmware)(C) and [KMK](https://github.com/KMKfw/kmk_firmware)(CircuitPython, Adafruit's fork of MicroPython)

Hardware
========
The PCB was designed from scratch with KiCAD based on `Sleepdealr`'s [RP2040 design guide](https://github.com/Sleepdealr/RP2040-designguide) and uses some symbol/footprint libraries such as:
- [marbastlib](https://github.com/ebastler/marbastlib) -- Keyboard parts
- [SnapEDA](https://www.snapeda.com/parts/RP2040/Raspberry%20Pi/view-part/) -- RP2040 MCU

