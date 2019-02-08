# scripts {#scripts}

Tiscamera has helper scripts that aim to help with deployment and installation.

## install-dependencies.sh     {#dependencies}

install-dependencies.sh is a shell scripts that allows the installation of dependencies on Debian
based systems.  
It allows for the installation of compilation and/or runtime dependencies. These can be
selected with --compilation and --runtime.

Since the usage of installed packages by other software can not be tracked an uninstall option is not available.

## env.sh

env.sh is a bourne shell script that you can source to integrate the build directory
into your environment. 

It will append directories to your PATH and library search path for the dynamic linker
and gstreamer.

<!-- it will add multi <\!--  -\-> -->
