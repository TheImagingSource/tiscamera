#!/usr/bin/env sh

# This example creates a live stream of the camera.

# usage:
# ./live-stream.sh <serial of your camera>
# e.g.:
# ./live-stream.sh 51310104

# variables used to describe the video
WIDTH=1920
HEIGHT=1080
FPS=15/1
SERIAL=

# assure we have a serial number
if [ -z "$1" ]; then
    echo "please provide the serial number of the camera."
    exit 1
else
    SERIAL="$1"
fi

# the actual pipeline
gst-launch-1.0 \
    tcamsrc serial=${SERIAL} \
    ! video/x-bayer,width=$WIDTH,height=$HEIGHT,framerate=$FPS \
    ! bayer2rgb \
    ! videoconvert \
    ! autovideosink

# for mono images please use this pipeline instead

# gst-launch-1.0 \
#     tcamsrc serial=${SERIAL} \
#     ! video/x-raw,format=GRAY8,width=$WIDTH,height=$HEIGHT,framerate=$FPS \
#     ! videoconvert \
#     ! autovideosink
