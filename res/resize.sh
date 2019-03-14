#!/bin/bash
for size in 16 32 48 128 256; do
    inkscape -z -e $size.png -w $size -h $size logo.svg >/dev/null 2>/dev/null
done
convert 16.png 32.png 48.png 128.png 256.png -colors 256 icon.ico
rm 16.png 32.png 48.png 128.png 256.png 
