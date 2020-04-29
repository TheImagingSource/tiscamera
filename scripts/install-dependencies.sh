#!/usr/bin/env sh

# Copyright 2019 The Imaging Source Europe GmbH
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# This script attempts to ease the installation of dependencies
# It does this by checking the distribution and installing the packages
# according to the dependency list described in the correlating file.


# project base folder
TISCAMERA_DIR=



# Distribution in use
DISTRIBUTION="DEBIAN"
ASSUME_YES=""

# possible values currently are:
# DEBIAN
# RHEL
# ARCH


# Retrieve the base dir of the tiscamera project
#
get_dir () {
    # Absolute path to this script, e.g. /home/user/bin/foo.sh
    SCRIPT=$(readlink -f "$0")
    # Absolute path this script is in, thus /home/user/bin
    SCRIPTPATH=$(dirname "$SCRIPT")
    # echo ${SCRIPTPATH%/*}
    TISCAMERA_DIR="${SCRIPTPATH%/*}"
}


#
# Check package manager to determine used distribution
# Sets global DISTRIBUTION accordingly
#
get_distribution () {

    if [ -x "$(command -v dpkg)" ]; then
        DISTRIBUTION="DEBIAN"
    else
        echo "Unable to determine distribution."
        DISTRIBUTION=
    fi
}


# read file given as $1
# remove comments
# convert it to a single line
# remove version information and commata
# echo a string as return value
read_file () {

    if [ -z "$1" ]; then
        echo "No file name given from which to read dependencies!"
        exit 1
    fi

    echo $(grep -vE "^\\s*#" $1  | tr "\n" " " | sed -e 's/([^()]*)//g; s/,//g')
}

#
# DEBIAN SECTION
#

install_dependencies_debian_compilation () {
    sudo apt-get install $ASSUME_YES $(read_file "$TISCAMERA_DIR/dependencies-debian-compilation.txt")
}


install_dependencies_debian_runtime () {
    sudo apt-get install $ASSUME_YES $(read_file "$TISCAMERA_DIR/dependencies-debian-runtime.txt")
}


# General install/remove routines
#

install_compile_dependencies () {
    case "$DISTRIBUTION" in
        DEBIAN)
            install_dependencies_debian_compilation
            ;;
        *)
            printf "Distribution '%s' is not supported.\n" $DISTRIBUTION
            exit 1
    esac
}


install_runtime_dependencies () {
    case "$DISTRIBUTION" in
        DEBIAN)
            install_dependencies_debian_runtime
            ;;
        *)
            printf "Distribution '%s' is not supported.\n" $DISTRIBUTION
            exit 1
    esac
}


update_package_cache_debian () {
    sudo apt update
}


update_package_cache () {
    case "$DISTRIBUTION" in
        DEBIAN)
            update_package_cache_debian
            ;;
        *)
            printf "Distribution '%s' is not supported.\n" $DISTRIBUTION
            exit 1
    esac
}


usage () {
    printf "%s\n" "$0"
    printf "install dependencies for the tiscamera project\n"
    printf "options:\n"
    printf "\t--compilation \t Install compilation dependencies\n"
    printf "\t--runtime \t Install runtime dependencies\n"
    printf "\t--no-update \t Do not update the package cache\n"
    printf "\t--yes \tassume user agrees to actions"
    printf "\t--help \t\t Print this message\n"
}

#
# start of actual script
#

install_compilation=0
install_runtime=0
update_cache=1

if [ $# -eq 0 ]; then
    usage
    exit 1
fi

get_dir

get_distribution


while [ "$1" != "" ]; do
    PARAM=$(echo "$1" | awk -F= '{print $1}')
    # VALUE=$(echo $1 | awk -F= '{print $2}')
    case $PARAM in
        -h | --help)
            usage
            exit
            ;;
        --compilation)
            install_compilation=1
            ;;
        --runtime)
            install_runtime=1
            ;;
        --yes)
            ASSUME_YES="-y"
            ;;
        --no-update)
            update_cache=0
            ;;
        *)
            echo "ERROR: unknown parameter \"$PARAM\""
            usage
            exit 1
            ;;
    esac
    shift
done

if [ $install_compilation -eq 0 ] && [ $install_runtime -eq 0 ]; then
    echo "Please specify if compilation and/or runtime dependencies should be installed."
    exit 1
fi

if [ $update_cache -eq 1 ]; then
    update_package_cache
fi

if [ $install_compilation -eq 1 ]; then
    install_compile_dependencies
fi

if [ $install_runtime -eq 1 ]; then
    install_runtime_dependencies
fi
