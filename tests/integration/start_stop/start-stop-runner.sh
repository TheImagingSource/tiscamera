#!/usr/bin/env bash

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


# DIR is used so that execution from other directories is possible
DIR="$( cd "$( dirname "$0" )" >/dev/null 2>&1 && pwd )"
SERIAL=""
CAPS=""
REST=""
NUMBER_RUNS=-1
LOG_FILE=""
ONLY_LOG=0


usage () {
    printf "%s\n" "$0"
    printf "Run start-stop in an endless loop\n"
    printf "options:\n"
    printf "\t-h,--help \t Print this message\n"
    printf "\t--serial TEXT\t Run with specific camera\n"
    printf "\t--caps TEXT\t Run with specific caps\n"
    printf "\t--rest {NULL,READY}\t Go down to the specified GStreamer State. Default: NULL\n"
    printf "\t--runs N\t How often to run.. Default: -1 (without end)\n"
    printf "\t--log FILE\tWrite to log file and stdout\n"
    printf "\t--only-log\t Only write to log file and not stdout\n"
}


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
        --rest)
            shift
            REST=$1
            ;;
        --runs)
            shift
            NUMBER_RUNS=$1
            ;;
        --log)
            shift
            LOG_FILE=$1
            ;;
        --only-log)
            ONLY_LOG=1
            ;;
        *)
            echo "ERROR: unknown parameter \"$PARAM\""
            usage
            exit 1
            ;;
    esac
    shift
done

if [ $ONLY_LOG != 0 ] && [ "$LOG_FILE" == "" ]; then

    echo "--only-log requires a log file. Specify on with --log"

    exit 1
fi


# allow core files of unlimited size
ulimit -c unlimited

caps_str=""
if [ ! -z "$CAPS" ]; then
    caps_str="--caps=$CAPS"
fi

serial_str=""
if [ ! -z "$SERIAL" ]; then
    serial_str="--serial=$SERIAL"
fi

rest_str="--rest=NULL"
if [ ! -z "$REST" ]; then
    rest_str="--rest=$REST"
fi

logging=""
if [ ! -z "$LOG_FILE" ]; then

    if [ $ONLY_LOG == 0 ]; then
        logging=" 2>&1 | tee -a ${LOG_FILE}"
    else
        logging=" 2>&1 >> ${LOG_FILE}"
    fi
fi

if [[ $NUMBER_RUNS != ?(-)+([0-9]) ]] ; then

    echo "--runs requires an integer"
    exit1
fi

# this is the command that will be executed in the loop
# we want gdb attached
# gdb needs to exit after a successfull run
CMD="gdb -ex run -ex quit --args $DIR/start-stop ${serial_str} ${caps_str} ${rest_str} ${logging}"

# due to the logging implementation
# the usage of `eval` is required.
# eval evaluates a cmd as if it was
# typed by the user. This allows
# pipes to work as intended.



# helper function
# prints to stdout
# and respects --log settings
say () {

    say_print_cmd="echo $@"
    eval ${say_print_cmd} $logging
}


if [[ $NUMBER_RUNS == -1 ]] ; then

    echo "Running for ever."
    while true; do

        eval $CMD

    done

else

    for number in $( seq 1 $NUMBER_RUNS )
    do
        echo -e "\n\n==== Run number: ${number}\n\n"
        eval $CMD
    done

fi
