# osi-visualizer

Visualization tool for OSI messages. This software serves as visualization tool for the current implementation of the [OSI (Open Simulation Interface)](https://github.com/OpenSimulationInterface/open-simulation-interface) mesages.The osi-visualizer supports to visualize the OSI GroundTruth and SensorData messages. It supports two independent visual channels with two different input types (file and network stream). 

![alt text](https://github.com/OpenSimulationInterface/osi-visualizer/tree/master/Resources/Images/Over_View.png "Over View")

## General Information

Some software library dependencies are needed to run the osi-visualizer:

* cmake (version >= 3.5): https://cmake.org/
* Qt (> 5.5.0): http://download.qt.io/official_releases/qt/
* ZeroMQ (>4.2.1): http://zeromq.org/intro:get-the-software
  The osi-visualizer needs to use ZeroMQ libraries to complete the socket communication between different sensor / traffic / scenario simulators. Note that the C++ Bindings are required as well.
* protobuf 2.6.1: https://github.com/google/protobuf

The required version of [Open Simulation Interface](https://github.com/OpenSimulationInterface/open-simulation-interface.git) is referenced as a git submodule, so be sure to run `git submodule init` && `git submodule update` after cloning the osi-visualizer repository.

If the CMake build process does not automatically locate the required libraries, please use the usual CMake options to set the relevant CMake variables to the proper paths.

Currently we strongly recommend users to use the osi-visualizer under Ubuntu Linux 16.04 LTS. You can see a working development environment based on Ubuntu 16.04 LTS in the Dockerfile in the repository.

## Build
### From Terminal:
- cd ${SRC_DIRECTORY}
- mkdir build
- cd build
- cmake ..
- make
- cp osi-visualizer ..
- cd ..
- ./osi-visualizer

### From QtCreator:
Open New Project -> CMakeLists.txt 

## Usage 
### Connection:
The channel receives osi message from the network with specific IP address and Port number, and shows up different data types.

![alt text](https://github.com/OpenSimulationInterface/osi-visualizer/tree/master/Resources/Images/Connection.png "Connection Setting")

### Playback:
The channel receives osi message from the loaded file. The tool will check a header file of same name with a extension: .txth. If it doesn't exist, the tool will create one automatically for the first time.
A "Delta-Delay" variable can be set to slow down the playback speed in case of very large message files. By default this value is "0" ms. 

Example: set it to 10 ms, each message will have 10 ms more delay for playing.

Besides the tool provides sending out osi message to network with specific port number.

![alt text](https://github.com/OpenSimulationInterface/osi-visualizer/tree/master/Resources/Images/Playback.png "Playback Setting")

### Save OSI Message:
Log or cut out part of the osi message stream and save it to another file. By default the threshold is 1000 osi messages. User can change this threshold in the config file.


## Menu options
### File Menu:
`Play` -> play/pause Channel 1 by default.

`Python Compare` -> A standalone compare tool to compare two osi message file directly. By default, it takes the playback file from these two channels. It supports flexible selecting random file as well. Just put the python scripts into folder: ./Resources/Python/ before start this tool. Then all the scripts will be shown up in the "Python Scripts" select menu. The compare script should take the two osi files' full path as arguments. The compare result can be exported to a text file by clicking the "Export" button. 

`Quit` -> Quit the visualizer.

### View Menu:
`Combine Channels` -> Config two channels' "Play/Pause" simultaneously, or not. It can only be checked when the channels are at the same status: both "Play" or "Pause". By default, it is unchecked.

`Show Grid` -> Show up the grid in the canvas. By default, it is checked.

`Show Object details` -> Select object from the Objects Overview tree. Show up the selected object's position, velocity and acceleration in another independent dialog. By default, it is checked.

The others are self-explaining.

## Notice
When playing large osi files, try to adapt the Delta Delay. Then the tool renders smoothly.


## Run time screen shot
Channel 1 plays an osi input file and sends out the osi message to port 5564 at the same time. 
Channel 2 receives osi message from port 5564 and shows up on the canvas.

![alt text](https://github.com/OpenSimulationInterface/osi-visualizer/tree/master/Resources/Images/Demo.png "Run time screen shot")



