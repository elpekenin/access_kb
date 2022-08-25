---
 _This file contains some icons that may not render with your font, **don't worry!**_

 While all the source and documentation I've developed is written on English, the report is on Spanish to be presented at my University, I might translate it on the future

---

Access keyboard
===============
This project's goal is to make a highly customizable keyboard with a bunch of accessibility add-ons, so that **most** people can use it.

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
   ├─  main.tex       * Wrapper file which joins everything together 
   └─  references.bib * Bibliography
```


Firmware
========
The firmware runs [MicroPython](https://micropython.org/) using [TinyUSB](https://docs.tinyusb.org/en/latest/), I modified the USB configuration so it can also use HID.

My code was made inspired by [QMK](https://github.com/qmk/qmk_firmware)(C) and [KMK](https://github.com/KMKfw/kmk_firmware)(CircuitPython, Adafruit's fork of MicroPython)

Features:
- Multiprocessing using `_thread`
  - Main core: Matrix scanning & HID messaging
  - Second core: Peripherals control (mostly WS2812's using PIO)
- Inline assembly: Speeds up the matrix scanning routine
- QMK/KMK-like:
  - Tap dance

I also made a QMK version of the firmware, check it out [here](https://github.com/qmk/qmk_firmware/keyboards/elpekenin/access/v1)


Hardware
========
The PCB was designed from scratch with KiCAD based on `Sleepdealr`'s [RP2040 design guide](https://github.com/Sleepdealr/RP2040-designguide) and uses some symbol/footprint libraries such as:
- [marbastlib](https://github.com/ebastler/marbastlib) -- Keyboard parts
- [SnapEDA](https://www.snapeda.com/parts/RP2040/Raspberry%20Pi/view-part/) -- RP2040 MCU

Features:
- Split
- Ortholinear
- Encoder+OLED to change configuration 
- The PCB has a 8-pin header with VCC, GND, 2 analog-capable and 4 regular GPIO's so you can add custom modules of hardware
- Unused GPIO's are broken out so they be used too
- Shift registers to reduce the amount of pins needed to scan the matrix
- e-Ink display to show the current configuration
- USB-A connector, so you can plug a device such a pen-drive
- Debugging interface


<!--
(Future plan)
Software 
========
Program running on your computer that can control some features of the keyboard
-->
