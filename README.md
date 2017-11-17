# osi-visualizer

Visualization tool for OSI messages.

## General Information

This software serves as visualization tool for the current implementation of the [OSI (Open Simulation Interface)](https://github.com/OpenSimulationInterface/open-simulation-interface) mesages.

It supports two independent visual channels with two different input types (file and network stream). 

## Usage 
### Connection:
The channel receives osi message from the network with specific IP address, Port number and data type.

### Playback:
The channel receives osi message from the loaded file. 
A "Delta-Delay" variable can be set to slow down the playback speed in case of very large message files. By default this value is "0" ms. 
Example: set it to 10 ms, each message will have 10 ms more delay for playing.

### Save OSI Message:
Log or cut out part of the osi message stream. Current the threshold is 100 osi messages. It can be saved to another osi file.


## Menu options
### File Menu:
"Play" -> play/pause Channel 1;

"Python Compare" -> A standalone compare tool to compare two osi message file directly. By default, it takes the playback file from these two channels. It supports flexible selecting random file as well. Just put the python scripts into folder: ./Resources/Python/ before start this tool. Then all the scripts will be shown up in the "Python Scripts" select menu. The compare script should take the two osi files' full path as arguments. The compare result can be exported to a text file by clicking the "Export" button. 

### View Menu:
"Combine Channels" -> Config two channels' "Play/Pause' button act simultaneously, or not. It can only be checked when the channels are at the same status: both "Play" or "Pause". By default, it is unchecked.
"Show Grid" -> Show up the grid in the canvas. By default, it is checked.
"Show Object details" -> Select object from the Objects Overview tree. Show up the selected object's position, velocity and acceleration in another independent dialog. By default, it is checked.

The others are self-explaining.

## Notice
When playing large osi files, try to adapt the Delta Delay and it renders smoothly.



