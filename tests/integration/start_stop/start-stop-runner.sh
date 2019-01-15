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


# DIR is used so that execution from other diirectories is possible
DIR="$( cd "$( dirname "$0" )" >/dev/null 2>&1 && pwd )"
SERIAL=""
CAPS=""

usage () {
    printf "%s\n" "$0"
    printf "Run start-stop in an endless loop\n"
    printf "options:\n"
    printf "\t-h,--help \t Print this message\n"
    printf "\t--serial TEXT\t Run with specific camera\n"
    printf "\t--caps TEXT\t Run with specific caps\n"
}


# while getopts "hs:c:" opt; do
#     case {opt} in
#         h)
#             usage
#             exit
#             ;;
#         s)
#             SERIAL=$OPTARG
#             ;;
#         c)
#             CAPS=$OPTARG
#             ;;
#     esac
# done

while [ "$1" != "" ]; do
    PARAM=$(echo "$1" | awk -F= '{print $1}')
    VALUE=$(echo $1 | awk -F= '{print $2}')
    case $PARAM in
        -h | --help)
            usage
            exit
            ;;
        --serial)
            shift
            SERIAL=$1
            ;;
        --caps)
            shift
            CAPS=$1
            ;;
        *)
            echo "ERROR: unknown parameter \"$PARAM\""
            usage
            exit 1
            ;;
    esac
    shift
done


export TCAM_LOG=DEBUG

# allow core files of unlimited size
ulimit -c unlimited

caps_str=""
if [ -n "CAPS" ]; then
    caps_str="--caps=$CAPS"
fi

serial_str=""
if [ -n "SERIAL" ]; then
    serial_str="--serial=$SERIAL"
fi

while true; do

    gdb -ex run -ex quit --args $DIR/start-stop $serial_str $caps_str --gst-debug=tcamsrc:5,tcambin:5

done
