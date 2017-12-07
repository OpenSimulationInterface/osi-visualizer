# The purpose of this package is to compare two sets of osi data. 

## General Information
The script was created to detect abnormalities between two different osi files. 
The first file is treated as a benchmark for the second file. 


## Usage

The script should be run with python 2.7.
The script expect at least two positional arguments. Both of those are osi-encoded messages.
The first file is treated as a benchmark.

It can be run using the following command: <br />
*./compare_data.py input.txt input_2.txt* <br />
Because the parsing of the file takes significant amount of time it is recommended, at least for the initial tests, to restrict number of read messages.



### Command line options
*-h -help* Display help <br />
*-v* "verbose mode", Display more informations  <br />
*-rl* "read line limit", Limits the number of lines read from the input file. If set, only firs first X number of osi messages will be read. 


### Dependencies 
The script is written for python 2.7.

The script uses
- Python modules os, sys, argparse and logging
- numpy http://www.numpy.org/
- termcolor https://pypi.python.org/pypi/termcolor
