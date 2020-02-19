#!/usr/bin/env bash

echo "
#################################
# Installing git                                                                 
#################################
"
apt-get install git -y

echo "
#################################
# Installing cmake                                                                 
#################################
"
apt-get install cmake autoconf automake libtool curl make g++ unzip libtool pkg-config build-essential -y

echo "
#################################
# Installing Qt                                                                 
#################################
"
apt-get install qt5-default -y

echo "
#################################
# Installing ZMQ                                                                   
#################################
"
apt-get install libzmq5 libzmq3-dev -y

apt-get install doxygen doxygen-doc doxygen-gui graphviz libeigen3-dev libboost-dev ocl-icd-opencl-dev -y

echo "
#################################
# Installing protobuf                                                                   
#################################
"
apt-get install libprotobuf-dev protobuf-compiler -y

echo "
#################################
# Installing OpenGL                                                                   
#################################
"
apt-get install libgl1-mesa-dev libgl1-mesa-dri libgl1-mesa-glx libqt5opengl5-dev -y
