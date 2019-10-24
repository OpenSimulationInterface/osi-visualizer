Nvidia Docker 
==============

The aim is to be able to run the osi-visualizer in docker to
provide an automated and flexible way for downloading, installing and setting up all necessary files and dependencies for osi-visualizer. Ideally it would only take time to build the first docker image, after that starting the image, i.e. running the visualizer, will be straight forward.

General Information
-------------------

The following Docker version was used: Docker version 18.03.1-ce, build 9ee9f40

For download and installation please refer to the following link:
https://docs.docker.com/install/linux/docker-ce/ubuntu/

You will also need nvidia-docker2, please refer to the following link:
https://github.com/NVIDIA/nvidia-docker/wiki/Installation-(version-2.0)

Build
-----
.. code-block:: bash

    cd osi-visualizer
    sh build-nvidia-docker.sh # will build the image for you
    sh run-nvidia-docker.sh # will run the visualizer for you

OSI-Visualizer protobuf files
-----------------------------

In order to load files into osi-visualizer, you need to place your *.pb files in "osi_pb" (mounted as a volume to the image).

Note
-----

Currently the osi-visualizer is set up and tested to work with the
playback option, for using other functionalities of the
osi-visualizer like socket communication then some updates should
probably be done to the docker file.

If after running the image the following error appears:

.. code-block:: bash

    Creating network “osi-visualizer_default” with the default driver
    No protocol specified QXcbConnection: Could not connect to display:1

then add the following command and run the image again (no need to
build the docker image again):

.. code-block:: bash

   sudo xhost +