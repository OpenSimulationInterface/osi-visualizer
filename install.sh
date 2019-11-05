#!/usr/bin/env bash
echo "
#################################
# Installing OSI Visualizer                                                                    
#################################
"

echo "
#################################
# Update System                                                                
#################################
"
sudo apt-get update -y;
sudo apt-get upgrade -y;
sudo apt-get autoremove -y;

echo "
#################################
# Installing git                                                                 
#################################
"
sudo apt-get install git -y;

echo "
#################################
# Installing cmake                                                                 
#################################
"
sudo apt-get install cmake autoconf automake libtool curl make g++ unzip libtool pkg-config build-essential;

echo "
#################################
# Installing Qt                                                                 
#################################
"
sudo apt-get install qt5-default;

echo "
#################################
# Installing ZMQ                                                                   
#################################
"
sudo apt-get install libzmq5 libzmq3-dev -y;

sudo apt-get install doxygen doxygen-doc doxygen-gui graphviz libeigen3-dev libboost-dev ocl-icd-opencl-dev -y;

echo "
#################################
# Installing protobuf                                                                   
#################################
"
sudo apt-get install libprotobuf-dev protobuf-compiler -y;

echo "
#################################
# Installing OpenGL                                                                   
#################################
"
sudo apt-get install libgl1-mesa-dev libgl1-mesa-dri libgl1-mesa-glx -y;

echo "
#################################
# Installing OSI 3                                                                    
#################################
"
git submodule update --init
cd open-simulation-interface;
git clone https://github.com/OpenSimulationInterface/proto2cpp.git;
mkdir -p build;
cd build;
cmake ..;
make -j8;
sudo make install;
cd ../..;


echo "
#################################
# Installing FMI 2.0.2                                                                  
#################################
"
mkdir -p fmi_library
cd fmi_library

FMI_lib_version=2.0.2

fmi_library_include_install_dir=/usr/local/include/fmi-library
fmi_library_lib_install_dir=/usr/local/lib/fmi-library

install_fmi=false

if ! [ -e ${fmi_library_include_install_dir} -a -e ${fmi_library_lib_install_dir} ] ; then
     install_fmi=true
fi

if ${install_fmi}; then
echo "
Downloading FMI library...
"

if [ ! -d $FMI_lib_version ]; then
    wget --no-parent -nH --cut-dirs=2 -r https://svn.jmodelica.org/FMILibrary/tags/$FMI_lib_version/
fi

echo "
Building FMI library...
"
  cd ${FMI_lib_version}
  mkdir -p build
  cd build
  cmake ../. -DCMAKE_BUILD_TYPE=RELEASE
  make -j8 install
  cd ..
  if [[ -d "install" ]]; then
    cd install
      sudo mkdir -p ${fmi_library_include_install_dir}
    if [[ -d ${fmi_library_include_install_dir} ]]; then
      sudo cp -uvrf ./include/* ${fmi_library_include_install_dir}/
    fi
    sudo mkdir -p ${fmi_library_lib_install_dir}
    if [[ -d ${fmi_library_lib_install_dir} ]]; then
      sudo cp -uvrf ./lib/* ${fmi_library_lib_install_dir}/
    fi
  else
      echo "Could not install the fmi-library into usr/local"
  fi
fi

cd ..;

echo "
#################################
# Building OSI Visualizer                                                                
#################################
"
mkdir -p build;
cd build;
cmake ..;
make -j8;

echo "
#################################
# Starting OSI Visualizer                                                                
#################################
"
./osi-visualizer &
