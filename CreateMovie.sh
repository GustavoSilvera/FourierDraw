#!/bin/bash

set -e # fail on error

# don't even need to convert to png's with ffmpeg
# for f in $(ls Out/*.ppm | sort -V)
# do
#     convert $f $f.png
#     rm $f
#     echo -ne "Converted $f \r"
# done
rm Out/Movie.mp4 || true

echo -e "\nRendering Movie..."
# omit all messages (non-verbose), all ppm files in Output directory
# NOTE: remove "-qscale 0" for a much smaller file (~20x smaller) at a cost of quality
ffmpeg -hide_banner -loglevel error \
       -pattern_type glob -i 'Out/*.ppm' \
       -c:v mpeg4 -pix_fmt yuv420p -qscale 0 Out/Movie.mp4 
echo -e "...Done!"
# clean directory
# for f in Out/*.png
# do
#     rm $f
# done