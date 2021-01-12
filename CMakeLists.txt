cmake_minimum_required(VERSION 3.1)
add_subdirectory(open-simulation-interface)
file(COPY resources DESTINATION .)
project(osi-visualizer)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC OFF)

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

# Unfortunately CMAKE_AUTORCC does not automatically rebuild the qrc file if one
# of the contained files changes. Set up rcc manually to ensure correct builds
# even if e.g. the shaders change.
set(RCC_OUTPUT_FILE ${CMAKE_CURRENT_BINARY_DIR}/rcc_resources.cpp)
add_custom_command(OUTPUT ${RCC_OUTPUT_FILE}
    COMMAND ${Qt5Core_RCC_EXECUTABLE}
    ARGS -o ${RCC_OUTPUT_FILE}
         resources.qrc
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/resources
    MAIN_DEPENDENCY resources/resources.qrc
    DEPENDS resources/Shaders/FragmentShader.frag
            resources/Shaders/VertexShader.vert
            resources/Images/Grid.png
    VERBATIM)
set_property(SOURCE ${RCC_OUTPUT_FILE} PROPERTY SKIP_AUTOMOC ON)
set_property(SOURCE ${RCC_OUTPUT_FILE} PROPERTY SKIP_AUTOUIC ON)
set_property(SOURCE ${RCC_OUTPUT_FILE} PROPERTY SKIP_AUTOGEN ON)

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
    src/osiparser.cpp
    src/glfieldofview.cpp
    src/fmureceiver.cpp
    src/utils.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/rcc_resources.cpp
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

