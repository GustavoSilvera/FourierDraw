#!/bin/bash

# run the entire pipeline for generating a simulator movie

set -e

# don't need to remove old out directory
# rm -rf Out/ || true

make -B --silent

mkdir -p Out

./FourierDraw

./CreateMovie.sh

