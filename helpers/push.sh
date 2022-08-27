#!/usr/bin/zsh

# This script relies on [ampy](https://github.com/scientifichackers/ampy) to push files into my RP2040 flash storage

# Path were data is copied from
dest_path=../firmware/py-code           # <-- Adjust as you wish

# My code contains a main.py file and /lib folder with all stuff, you might want to re-do in a recursive fashion for complex structures, checkout `pull.sh` for ideas

# Wipe the contents, throws a bunch of error code, but wipes
(ampy rmdir / > /dev/null 2>&1)

# Copy the file and folder
ampy put $dest_path/main.py
ampy put $dest_path/lib
