# Docker for osi-visualizer

The aim of this is to be able to run the osi-visualizer in docker, providing an automated and flexible way.
For downloading, installing and setting up all necessary files and dependencies.
Ideally it would only take time to build the first docker image, after that starting the image i.e. running the visualizer will be straight forward.


## General Information

The following Docker version was used:
Docker version 18.03.1-ce, build 9ee9f40

For download and installation please refer to the following link:
https://docs.docker.com/install/linux/docker-ce/ubuntu/


## Build
### From Terminal:
- cd ${SRC_DIRECTORY}
- make build-image-osi-visualizer (will build the image for you)
- make start-osi-visualizer (will run the visualizer for you)

## OSI-Visualizer protobuf files
- In order to load files into osi-visualizer, you need to palce your .pb files in "osi_pb" (mounted as a volume to the image)

## Note:

- Currently the osi-visualizer is set up and tested to work with the playback option, for using other functionalities of the osi-visualizer like "Socket" communication then some updates should probably be done to the docker file.
- If after running the image the following error appears:
Creating network "osi-visualizer_default" with the default driver
No protocol specified
QXcbConnection: Could not connect to display :1
Creating network "osi-visualizer_default" with the default driver
No protocol specified
QXcbConnection: Could not connect to display :1

then add the follwoing command and run the image again (no need to build the docker image again):
-sudo xhost +