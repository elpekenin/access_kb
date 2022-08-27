#!/usr/bin/zsh

# This script relies on [ampy](https://github.com/scientifichackers/ampy) to push files into my RP2040 flash storage

# Path were data is copied from
origin_path=../firmware/py-code           # <-- Adjust as you wish

# My code contains a main.py file and /lib folder with all stuff, you might want to re-do in a recursive fashion for complex structures, checkout `pull.sh` for ideas

# Wipe the contents, throws a bunch of error code, but wipes
echo "==> Wiping the storage before loading ... \n"
(ampy rmdir / > /dev/null 2>&1)

# Copy the file
echo "==> Pushing main.py ...\n"
ampy put $origin_path/main.py

# Make some stuff just to print some feedback
temp_file=temp.txt
#touch $temp_file

echo -n "==> Pushing /lib "
ampy put $origin_path/lib 2> /dev/null 1> $temp_file &

while ps | grep $! &> /dev/null; do
  echo -n "."
  sleep 1
done

echo "" #newline on end

rm $temp_file
