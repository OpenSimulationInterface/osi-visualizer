#!/usr/bin/env bash

echo "
#################################
# Building OSI Visualizer                                                                
#################################
"
mkdir -p build
cd build
cmake ..
make -j8
