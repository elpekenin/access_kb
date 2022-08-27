#!/usr/bin/zsh

# Remove all nvim's swap files, as ssh crashes every now and then, creating a bunch of them
sudo rm ~/.local/share/nvim/swap/*.swp
