FROM ubuntu:latest

RUN mkdir -p /project
RUN apt-get update && apt-get install -y git vim unzip zip cmake gcc g++ qtbase5-dev libprotobuf-dev protobuf-compiler libzmq3-dev wget
RUN mkdir -p /build
WORKDIR /build
RUN wget https://jmodelica.org/fmil/FMILibrary-2.0.3-src.zip && unzip FMILibrary-2.0.3-src.zip && mkdir build-fmil && cd build-fmil && cmake -DFMILIB_INSTALL_PREFIX=/usr/local ../FMILibrary-2.0.3 && make install

VOLUME [ "/project"]
WORKDIR /project
