#!/usr/bin/env python3

# Starter for The Imaging Source cameras in ros.
# Needs Python 3
# Please refer to https://github.com/TheImagingSource/tiscamera
# Goto: http://wiki.ros.org/gscam for info.
# Install sudo apt-get install ros-kinetic-gscam

import tiscamera
import time
import argparse

parser = argparse.ArgumentParser(
    description='The Imaging Source ROS camera driver.')
required_named = parser.add_argument_group('required arguments')
required_named.add_argument('-l', '--left_serial', type=str, dest="left_serial",
                            help="Left camera serial number", required=True)
parser.add_argument('-r', '--right_serial', type=str, dest="right_serial",
                    help="Right camera serial number", required=False)
args = parser.parse_args()

# Open the camera.
# Parameters are serial number, width, height, frame rate, color, liveview, ROS topic name.
first_cam = tiscamera.Camera(
    args.left_serial, 1920, 1208, 30, True, False, "camera_left_raw", "left_camera_node")
if args.right_serial:
    second_cam = tiscamera.Camera(
        args.right_serial, 1920, 1208, 30, True, False, "camera_right_raw", "right_camera_node")

# Start the live stream from the camera and also "rosrun"
first_cam.start_pipeline()
if args.right_serial:
    time.sleep(5)
    second_cam.start_pipeline()

# Set some properties
first_cam.set_property("Exposure Auto", True)
first_cam.set_property("Gain Auto", True)
first_cam.set_property("Brightness Reference", 128)

if args.right_serial:
    second_cam.set_property("Exposure Auto", True)
    second_cam.set_property("Gain Auto", True)
    second_cam.set_property("Brightness Reference", 128)

input("Press Enter to end program")

# Stop the camera pipeline.
first_cam.stop_pipeline()
if args.right_serial:
    second_cam.stop_pipeline()

print('Program ended')
