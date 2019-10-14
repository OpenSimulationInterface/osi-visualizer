cmake_minimum_required(VERSION 3.1)
add_subdirectory(open-simulation-interface)
file(COPY resources DESTINATION .)
project(osi-visualizer)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Core Gui OpenGL Widgets Xml Network REQUIRED)

find_path(
  FMILIB_INCLUDE_DIRS
  fmilib.h
  HINTS /usr/include/fmi-library /usr/include /usr/local/include/fmi-library /usr/local/include
)

find_library(
  FMILIB_LIBRARY
  fmilib_shared
  HINTS /usr/lib/fmi-library /usr/lib /usr/local/lib/fmi-library /usr/local/lib
)

set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/pythoncomparedialog.cpp
    src/displayobjectdialog.cpp
    src/glwidget.cpp
    src/globject.cpp
    src/camera.cpp
    src/tcpreceiver.cpp
    src/osireader.cpp
    src/glvehicle.cpp
    src/gllanemarking.cpp
    src/lane.cpp
    src/glgrid.cpp
    src/customtreewidgetitem.cpp
    src/gltriangle.cpp
    src/appconfig.cpp
    src/glpoint.cpp
    src/gltrafficsign.cpp
    src/qrc_resources.cpp
    src/osiparser.cpp
    src/glfieldofview.cpp
    src/fmureceiver.cpp
    src/utils.cpp
)

set(HEADERS
    include/mainwindow.h
    include/pythoncomparedialog.h
    include/displayobjectdialog.h
    include/glwidget.h
    include/globject.h
    include/camera.h
    include/imessagesource.h
    include/tcpreceiver.h
    include/osireader.h
    include/glvehicle.h
    include/gllanemarking.h
    include/lane.h
    include/glgrid.h
    include/types.h
    include/customtreewidgetitem.h
    include/gltriangle.h
    include/appconfig.h
    include/global.h
    include/glpoint.h
    include/gltrafficsign.h
    include/osiparser.h
    include/glfieldofview.h
    include/fmureceiver.h
    include/utils.h
)


include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${FMILIB_INCLUDE_DIRS}
)


add_executable(${PROJECT_NAME}
    ${HEADERS}
    ${SOURCES}
)

target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Widgets
    Qt5::Xml
    Qt5::Gui
    Qt5::OpenGL
    Qt5::Network

    open_simulation_interface
    ${FMILIB_LIBRARY}
    protobuf
    zmq
)

