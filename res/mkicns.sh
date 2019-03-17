#!/usr/bin/env bash

###
# Adapted from: https://stackoverflow.com/a/20703594
###

if [ -z "${1+set}" ]; then
  echo "Pass path to the inkscape command as first argument"
  exit 1
fi
if [ -z "${2+set}" ]; then
  echo "Pass svg file as second argument"
  exit 1
fi
if [ -z "${3+set}" ]; then
  echo "Pass output name (w/o .icns) as third argument"
  exit 1
fi

inkscape=$1
svg_file=$2
output_name=$3

set -e
mkdir $output_name.iconset
$inkscape -z -e "$PWD/$output_name.iconset/icon_16x16.png"      -w   16 -h   16 -y 0 "$PWD/$svg_file"
$inkscape -z -e "$PWD/$output_name.iconset/icon_16x16@2x.png"   -w   32 -h   32 -y 0 "$PWD/$svg_file"
$inkscape -z -e "$PWD/$output_name.iconset/icon_32x32.png"      -w   32 -h   32 -y 0 "$PWD/$svg_file"
$inkscape -z -e "$PWD/$output_name.iconset/icon_32x32@2x.png"   -w   64 -h   64 -y 0 "$PWD/$svg_file"
$inkscape -z -e "$PWD/$output_name.iconset/icon_128x128.png"    -w  128 -h  128 -y 0 "$PWD/$svg_file"
$inkscape -z -e "$PWD/$output_name.iconset/icon_128x128@2x.png" -w  256 -h  256 -y 0 "$PWD/$svg_file"
$inkscape -z -e "$PWD/$output_name.iconset/icon_256x256.png"    -w  256 -h  256 -y 0 "$PWD/$svg_file"
$inkscape -z -e "$PWD/$output_name.iconset/icon_256x256@2x.png" -w  512 -h  512 -y 0 "$PWD/$svg_file"
$inkscape -z -e "$PWD/$output_name.iconset/icon_512x512.png"    -w  512 -h  512 -y 0 "$PWD/$svg_file"
$inkscape -z -e "$PWD/$output_name.iconset/icon_512x512@2x.png" -w 1024 -h 1024 -y 0 "$PWD/$svg_file"
iconutil -c icns "$output_name.iconset"
rm -R "$output_name.iconset"

