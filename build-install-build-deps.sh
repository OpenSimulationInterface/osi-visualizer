#!/usr/bin/env bash

echo "
#################################
# Installing OSI 3                                                                    
#################################
"
git submodule update --init
cd open-simulation-interface
git clone https://github.com/OpenSimulationInterface/proto2cpp.git
mkdir -p build
cd build
cmake ..
make -j8
make install
cd ../..


echo "
#################################
# Installing FMI 2.0.2                                                                  
#################################
"
mkdir -p fmi_library
cd fmi_library

fmi_library_include_install_dir=/usr/local/include/fmi-library
fmi_library_lib_install_dir=/usr/local/lib/fmi-library

if ! [ -e ${fmi_library_include_install_dir} -a -e ${fmi_library_lib_install_dir} ] 
then
  echo "Downloading FMI library..."

  if [ ! -d FMILibrary-2.0.2 ] 
  then
    wget --no-parent -nH --cut-dirs=2 -r https://jmodelica.org/FMILibrary/FMILibrary-2.0.2-src.zip
  fi

  echo "Building FMI library..."
  unzip FMILibrary-2.0.2-src.zip
  cd FMILibrary-2.0.2
  mkdir -p build
  cd build
  cmake ..
  make -j8
  make -j8 install
  cd ../install

  if [ ! -d ${fmi_library_include_install_dir} ] 
  then
  sudo mkdir -p ${fmi_library_include_install_dir}
  sudo cp -uvrf ./include/* ${fmi_library_include_install_dir}/
  fi

  if [ ! -d ${fmi_library_lib_install_dir} ] 
  then
  sudo mkdir -p ${fmi_library_lib_install_dir}
  sudo cp -uvrf ./lib/* ${fmi_library_lib_install_dir}/
  fi
fi