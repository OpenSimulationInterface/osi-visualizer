cmake_minimum_required(VERSION 3.1)
add_subdirectory(open-simulation-interface)
project(osi-visualizer)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Core Gui OpenGL Widgets Xml Network REQUIRED)

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
)

include_directories(include)

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

    open_simulation_interface_pic
    protobuf
    zmq
)
