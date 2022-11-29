#!/usr/bin/zsh

# Function to compile the LaTeX files
CUR_DIR=$(pwd) #store current dir

echo 'Cleaning previous compilation files ...'
rm /tmp/main*
  
echo 'Compiling ...'
cd ../latex && pdflatex -output-directory=/tmp main.tex > /dev/null 2>&1
  
echo 'Adding references...'
biber /tmp/main.bcf > /dev/null 2>&1
   
echo 'Recompiling ...'
pdflatex -output-directory=/tmp main.tex > /dev/null 2>&1
  
echo 'Copying file to working directory ...'
cp /tmp/main.pdf report.pdf
  
if pgrep -x "mupdf" > /dev/null
then # just in case, to avoid file not re-rendering upon re-compile
  echo 'Killing mupdf ...'
  pkill mupdf
fi

echo 'Opening report ...'
(mupdf report.pdf > /dev/null 2>&1 &)

cd $CUR_DIR #move back to original dir