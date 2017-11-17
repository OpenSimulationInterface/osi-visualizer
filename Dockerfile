FROM ubuntu:latest

RUN mkdir -p /project
RUN apt-get update && apt-get install -y git vim unzip zip cmake gcc g++ qtbase5-dev libprotobuf-dev protobuf-compiler libzmq3-dev

VOLUME [ "/project"]
WORKDIR /project
