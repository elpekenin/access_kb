#!/usr/bin/zsh

# This script relies on [ampy](https://github.com/scientifichackers/ampy) to pull the files in the RP2040 flash storage

# Path were data is pulled at
dest_path=../firmware/code

# Existing data is moved to a backup folder, this script wont be run often, so it *shouldnt* overwrite (much) uncommited code, but backuping just to be safe
backup_base=$dest_path/../backups/

# Create the error, nothing happens if already exists
mkdir $backup_base > /dev/null 2>&1

# Subfolder name based on current timestamp
backup_path=$backup_base$(date "+%F-%T")

# Move all /code contents but keep the -empty- directory
mkdir $backup_path && (mv $dest_path/* $backup_path) 
echo "==> Moved ${dest_path} to ${backup_path}\n"

function pull_a_file {
  # Interface feedback, the script didn't die
  echo -n "."

  # Try copying the file, silent output because it fails when run on directories
  dest=$dest_path$1
  ampy get $1 $dest > /dev/null 2>&1 

  if [[ $? -eq 1 ]]; #directory, so call recursively
  then
  # Create directory so we can copy files into it
  mkdir $dest
    # Get childs of the current path 
    paths=("${(@f)$(ampy ls $1)}") #https://unix.stackexchange.com/a/29748    
    for i ($paths)
    do
      # Call the function on the chil
      pull_a_file "$i"
    done
  fi
}

# Start the sript with empty input, aka "/"
echo -n "==> Pulling "
pull_a_file ""

# Show the files copied
echo "==> Process finised"
echo '-------------------'
tree $dest_path

