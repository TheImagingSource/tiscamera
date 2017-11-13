#!/usr/bin/env python3

# Copyright 2017 The Imaging Source Europe GmbH
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

#
# This example will show you how to list information about the available devices
#

import sys
import gi

gi.require_version("Tcam", "0.1")
gi.require_version("Gst", "1.0")

from gi.repository import Tcam, Gst

import common

def list_devices():
    """"""
    source = Gst.ElementFactory.make("tcambin")

    serials = source.get_device_serials()

    for single_serial in serials:

        # This returns someting like:
        # (True, name='DFK Z12GP031',
        # identifier='The Imaging Source Europe GmbH-11410533', connection_type='aravis')
        # The identifier is the name given by the backend
        # The connection_type identifies the backend that is used.
        #     Currently 'aravis', 'v4l2' and 'unknown' exist
        (return_value, model,
         identifier, connection_type) = source.get_device_info(single_serial)

        # return value would be False when a non-existant serial is used
        # since we are iterating get_device_serials this should not happen
        if return_value:

            print("Model: {} Serial: {} Type: {}".format(model,
                                                         single_serial,
                                                         connection_type))

def select_camera(source):
    """Helper function that prompts the user to select a camera.

    Returns: serial number or None on error
    """

    # retrieve all available serial numbers
    serials = source.get_device_serials()

    # create a list to have an easy index <-> serial association
    device_list = []
    # we add None to have a default value for the case 'serial not defined'
    # this also pushes our first serial index to 1.
    device_list.append(None)

    print("Available devices:")
    index = 1
    print("0 - Use default device")

    for s in serials:

        device_list.append(s)
        print("{} - {}".format(index, s))
        index = index + 1

    # get input from user and only stop asking when
    # input is legal
    legal_input = False
    while not legal_input:
        selection = int(input("Please select a device: "))
        if 0 <= selection < len(device_list):
            legal_input = True
        else:
            print("Please select a device.")

    return device_list[selection]


if __name__ == "__main__":
    Gst.init(sys.argv)  # init gstreamer
    list_devices()
