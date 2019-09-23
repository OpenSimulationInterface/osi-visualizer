# OSI Visualizer
[![Travis Build Status](https://travis-ci.org/OpenSimulationInterface/osi-visualizer.svg?branch=master)](https://travis-ci.org/OpenSimulationInterface/osi-visualizer)

OSI Visualizer serves as a visualization tool for the current implementation of [OSI (Open Simulation Interface)](https://github.com/OpenSimulationInterface/open-simulation-interface) messages. It supports `GroundTruth`, `SensorView` and `SensorData` messages and allows the visualization of two independent data channels using different input types (file and network stream). For more information see the documentation [here](https://opensimulationinterface.github.io/osi-documentation/osi-visualizer/README.html).

## Usage

Use the configuration panel at the right (see Screenshot below) to load a playback file or stream for channel 1 and choose a port to which it will be send. Enter the host and input port for channel 2 in the panel below. Select the data type it is expected to visualize (here `SensorData`). Make sure the appropriate communication server for the data exchange is up and running. 

![](Resources/Images/Over_View.png)

Now you can click on play for channel 1 and 2 and you can visualize the communication between these two channels. Channel 1 plays an OSI input file and sends out the OSI message to port 5564 at the same time. Channel 2 receives an OSI message from port 5564 showing it on the canvas (see demo screenshot below).

![](Resources/Images/Demo.png)

## Installation

#### Environment and dependencies

Currently we recommend users to use the osi-visualizer under Ubuntu Linux 18.04 LTS. You can see a working development environment based on Ubuntu 18.04 LTS in the Docker file in the repository.

* [cmake](https://cmake.org/) (>= 3.5) 
* [Qt](http://download.qt.io/official_releases/qt/) (>= 5.5.0)
* [ZeroMQ](http://zeromq.org/intro:get-the-software) (>= 4.2.1) OSI Visualizer needs to use ZeroMQ libraries to complete the socket communication between different sensor / traffic / scenario simulators. Note that the C++ Bindings are required as well.
* [protobuf](https://github.com/google/protobuf) (>= 2.6.1)
* [FMILibrary](https://svn.jmodelica.org/FMILibrary/tags) (>= 2.0.2)
* [OSI](https://github.com/OpenSimulationInterface/open-simulation-interface.git) (>= 3.1.2) 

#### Build and run

```bash
$ git clone https://github.com/OpenSimulationInterface/osi-visualizer.git
$ cd osi-visualizer
$ mkdir build
$ cd build
$ cmake ..
$ make
$ ./osi-visualizer
```
