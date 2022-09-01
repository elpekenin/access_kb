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
|  ├─ 📂backups/       * Files saved by `download.sh` **Untracked**
|  ├─  micropython/   * My fork of the Python implementation for microcontrollers
|  ├─ 📂py-code/       * Custom firmware implementation from scratch
|  └─ 📂qmk-code/      * QMK version of the firmware
├─ 📂hardware/     - PCB files
|  └─ 📂libraries/     * References to KiCAD symbols and footprints 
├─ 📂latex/        - Sources used to create the PDF report
|  ├─ 📂images/        * Various visual resources 
|  ├─ 📂snippets/      * Relevant pieces of code and configuration
|  ├─ 📂tex/           * Fragments of the complete writing
|  ├─  conf.tex       * LaTeX configuration
|  ├─  main.tex       * Wrapper file which joins everything together 
|  ├─  references.bib * Bibliography
|  └─  report.pdf     * Output file 
└─ 📂helpers/      - Some scripts for repetitive tasks
```


Firmware
========
The firmware runs [MicroPython](https://micropython.org/) using [TinyUSB](https://docs.tinyusb.org/en/latest/), but with a modified USB configuration so it can also use HID, this was based on [noobee's work](https://github.com/noobee/micropython/tree/usb-hid).

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


Utils
=====
This folder contains some bash scripts to make life easier:
- download.sh -- Pulls the code from the RP2040's flash memory into a folder, and backups the code that was there before pulling 
- pdf.sh      -- Compiles the latex file and opens the resulting file
- rmswap.sh   -- Removes all .swp files in nvim's cache folder, as a bunch will be created upon SSH disconnecting 
- upload.sh   -- Pushes a folder's content to the computer into the board


<!--
(Future plan)
Software 
========
Program running on your computer that can control some features of the keyboard
-->
