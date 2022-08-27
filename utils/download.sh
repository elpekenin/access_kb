#!/usr/bin/zsh

# This script relies on [ampy](https://github.com/scientifichackers/ampy) to pull the files from my RP2040 flash storage

# Path were data is downloaded at
dest_path=../firmware/py-code           # <-- Adjust as you wish

# Existing data is moved to a backup folder, this script wont be run often, so it *shouldnt* overwrite (much) uncommited/unsaved code, but backuping just to be safe
backup_base=$dest_path/../backups/   # <-- Adjust as you wish

# Create the backup directory (if doesn't exist already)
mkdir $backup_base > /dev/null 2>&1

# Subfolder name based on current timestamp
backup_path=$backup_base$(date "+%F-%T")

# Check if destination path has any content
if [ $(ls -A "$dest_path" | wc -l) -ne 0 ]; then
  # Skip if empty
  echo "==> Destination folder was empty, proceeding ...\n"
else 
  # Backup it otherwise
  mkdir $backup_path && unsetopt nomatch; mv $dest_path/* $backup_path > /dev/null 2>&1; setopt nomatch 
  echo "==> Destination folder had content, moved it into $(realpath $backup_path) before proceeding ...\n" #realpath will solve absolute path
fi

function pull_a_file {
  # To have some feedback, the script didn't die
  echo -n "."

  # Try copying the file, silent output because it will fail when run on directories
  dest="$dest_path$1"
  ampy get "$1" "$dest" > /dev/null 2>&1 

  if [[ $? -eq 1 ]]; then # Fail --> Directory --> Call recursively
    # Create directory so we can copy files into it (if doesn't exist already)
    mkdir "$dest" > /dev/null 2>&1
      # Get childs of the current path 
      paths=("${(@f)$(ampy ls $1)}") #https://unix.stackexchange.com/a/29748    
      for i ($paths); do
        # Call the function on the child
        pull_a_file "$i"
      done
  fi
}

# Start the sript with empty input, aka "/"
echo -n "==> Pulling "
pull_a_file ""

# Show the files copied
echo "\n"
echo "==> Process finised"
echo '-------------------'
tree $(realpath $dest_path)
