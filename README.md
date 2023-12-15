---
⚠️ _This file contains some icons that should render on GitHub, but **don't worry!** if they don't_

📝 While all the source and documentation I've developed is written on English, the report is on Spanish to be presented at my University, I might translate it on the future

---

Access keyboard
===============
This project's goal is to make a highly customizable keyboard with a bunch of accessibility add-ons, so that **most** people can use it.

File structure:
```
📂 access_kb
├─ 📂firmware/     - Code running on the keyboard, compiled using qmk_build
├─ 📂hardware/     - PCB files
|  └─ 📂libraries/     * References to KiCAD symbols and footprints 
├─ 📂software      - Client running on the PC to control the keyboard
├─ 📂latex/        - Sources used to create the PDF report
|  ├─ 📂images/        * Various visual resources 
|  ├─ 📂snippets/      * Relevant pieces of code and configuration
|  ├─ 📂tex/           * Fragments of the complete writing
|  ├─ 🔨conf.tex       * LaTeX configuration
|  ├─ 📋main.tex       * Wrapper file which joins everything together 
|  ├─ 📚references.bib * Bibliography
|  └─ 📖report.pdf     * Output file 
└─ 📂helpers/      - Some scripts for repetitive tasks
```


Firmware
========
Firmware made using [QMK](https://github.com/qmk/qmk_firmware)(C).

All the extra code/patches needed to build the firmware can be found on the folder, it it built using [qmk_build](https://github.com/elpekenin/qmk_build)


Hardware
========
The PCB was designed with KiCAD, based on ***Raspberry Pi Pico*** development board, and using some symbol/footprint libraries:
- [marbastlib](https://github.com/ebastler/marbastlib) -- Keyboard parts
- [rp-pico](https://github.com/ncarandini/KiCad-RP-Pico/) -- Raspberry Pi Pico
- [custom](./hardware/libraries/my_components.pretty/)
   - Screen models based on 1xN female 2.54mm pin connectors + roughly measured rectangles. Used for placing and sizing, not used in final model
   - Symbol for MC74HC589ADR2G, footprint is just a 16-pin SOIC

Features:
- Split
- Ortholinear
- Shift registers to reduce the pins needed
  - PISO for matrix scanning
  - SIPO for control signals
- Unused GPIO's are broken out so they be used too
- e-Ink display to show the current configuration


Utils
=====
This folder contains some bash scripts to make life easier:
- download.sh -- Pulls the code from the RP2040's flash memory into a folder, and backups the code that was there before pulling 
- pdf.sh      -- Compiles the latex file and opens the resulting file
- rmswap.sh   -- Removes all .swp files in nvim's cache folder, as a bunch will be created upon SSH disconnecting 
- upload.sh   -- Pushes a folder's content to the computer into the MCU's flash

*Note*: `download` and `upload` are only useful when working with MicroPython


Software 
========
Program using [Vue](https://vuejs.org/) and [Tauri](https://tauri.app/) on your computer, that can control some features of the keyboard and send information. Based on [KarlK90's work](https://github.com/qmk/qmk_xap)

