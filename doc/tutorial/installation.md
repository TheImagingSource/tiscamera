
# Installation

## Dependencies

To install tiscamera the following tools/libraries are required:

- cmake
- libtinyxml
- gcc >= 4.9
- lubudev


Depending on the modules you want to build the following additional dependencies may exist.

- gstreamer-0.10
- libgobject
- gstreamer-1.0
- libgstreamer-0.10
- libglib-2.0
- aravis

- doxygen

## Build steps

If you do not wnat to handle many commandline options you can use "cmake-gui" for most of the configuration steps.

The basic build and installation process consists out of these commands:

    cd <tiscamera>
    mkdir build
    cd build
    cmake ..
    make -j 4
    sudo make install

### cmake options

This is a list of important cmake options for tiscamera.

#### GigE support
default: OFF

    -DBUILD_ARAVIS:BOOL=ON/OFF

#### USB support
default: ON

    -DBUILD_V4L2:BOOL=ON/OFF

#### gstreamer-0.10 support

    -DBUILD_0_10:BOOL=ON/OFF

#### gstreamer-1.0 support

    -DBUILD_GST_1_0:BOOL=ON/OFF

#### tools

    -DBUILD_TOOLS:BOOL=ON/OFF

#### documentation
default: OFF

    -DBUILD_DOC:BOOL=ON/OFF

#### Installation path

Per default all files will be installed in /usr/local/.
If you want to change the directory set CMAKE_INSTALL_PREFIX.

### Crosscompiling

To cross-compile for different plattforms you can look at <tiscamera>/cmake/toolchain.

To use this files you have to call cmake like:

TODO
