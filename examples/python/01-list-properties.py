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
# This example will show you how to list available properties
#

import sys
import gi

gi.require_version("Tcam", "1.0")
gi.require_version("Gst", "1.0")
gi.require_version("GLib", "2.0")

from gi.repository import Tcam, Gst, GLib


def flag_strings(prop):
    """

    """

    ret = "Available: "

    if prop.is_available():
        ret += "yes"
    else:
        ret += "no"

    ret += "\tLocked: "

    if prop.is_locked():
        ret += "yes"
    else:
        ret += "no"

    return ret


def list_properties(camera):

    property_names = camera.get_tcam_property_names()

    for name in property_names:
        try:
            base = camera.get_tcam_property(name)

            if base.get_property_type() == Tcam.PropertyType.INTEGER:

                default = base.get_default()
                mini, maxi, step = base.get_range()

                unit = base.get_unit()
                if not unit:
                    unit = ""

                print(("{name}\ttype: Integer\tDisplay Name: \"{disp_name}\"\tCategory: {cat}\n"
                       "\t\t\tDescription: {desc}\n"
                       "\t\t\tUnit: {unit}\n"
                       "\t\t\tVisibility: {vis}\n"
                       "\t\t\tPresentation: {pres}\n"
                       "\t\t\t{flags}\n\n"
                       "\t\t\tMin: {mini}\t Max: {maxi}\tStep: {step}\n"
                       "\t\t\tDefault: {default}\n"
                       "\t\t\tValue: {val}\n\n").format(name=name,
                                                        disp_name=base.get_display_name(),
                                                        cat=base.get_category(),
                                                        desc=base.get_description(),
                                                        unit=unit,
                                                        vis=base.get_visibility(),
                                                        pres=base.get_representation(),
                                                        flags=flag_strings(base),
                                                        mini=mini,
                                                        maxi=maxi,
                                                        step=step,
                                                        default=default,
                                                        val=base.get_value()))
            elif base.get_property_type() == Tcam.PropertyType.FLOAT:

                default = base.get_default()
                mini, maxi, step = base.get_range()

                unit = base.get_unit()
                if not unit:
                    unit = ""

                print(("{name}\ttype: Float\tDisplay Name: \"{disp_name}\"\tCategory: {cat}\n"
                       "\t\t\tDescription: {desc}\n"
                       "\t\t\tUnit: {unit}\n"
                       "\t\t\tVisibility: {vis}\n"
                       "\t\t\tPresentation: {pres}\n"
                       "\t\t\t{flags}\n\n"
                       "\t\t\tMin: {mini}\t Max: {maxi}\tStep: {step}\n"
                       "\t\t\tDefault: {default}\n"
                       "\t\t\tValue: {val}\n\n").format(name=name,
                                                        disp_name=base.get_display_name(),
                                                        cat=base.get_category(),
                                                        desc=base.get_description(),
                                                        unit=unit,
                                                        vis=base.get_visibility(),
                                                        pres=base.get_representation(),
                                                        flags=flag_strings(base),
                                                        mini=mini,
                                                        maxi=maxi,
                                                        step=step,
                                                        default=default,
                                                        val=base.get_value()))
            elif base.get_property_type() == Tcam.PropertyType.ENUMERATION:
                print(("{name}\ttype: Enumeration\tDisplay Name: \"{disp_name}\"\tCategory: {cat}\n"
                       "\t\t\tDescription: {desc}\n"
                       "\t\t\tVisibility: {vis}\n"
                       "\t\t\t{flags}\n\n"
                       "\t\t\tEntries: {entries}\n"
                       "\t\t\tDefault: {default}\n"
                       "\t\t\tValue: {val}\n\n").format(name=name,
                                                        disp_name=base.get_display_name(),
                                                        cat=base.get_category(),
                                                        desc=base.get_description(),
                                                        vis=base.get_visibility(),
                                                        flags=flag_strings(base),
                                                        entries=base.get_enum_entries(),
                                                        default=base.get_default(),
                                                        val=base.get_value()))
            elif base.get_property_type() == Tcam.PropertyType.BOOLEAN:
                print(("{name}\ttype: Boolean\tDisplay Name: \"{disp_name}\"\tCategory: {cat}\n"
                       "\t\t\tDescription: {desc}\n"
                       "\t\t\tVisibility: {vis}\n"
                       "\t\t\t{flags}\n\n"
                       "\t\t\tDefault: {default}\n"
                       "\t\t\tValue: {val}\n\n").format(name=name,
                                                        disp_name=base.get_display_name(),
                                                        cat=base.get_category(),
                                                        desc=base.get_description(),
                                                        vis=base.get_visibility(),
                                                        flags=flag_strings(base),
                                                        default=base.get_default(),
                                                        val=base.get_value()))
            elif base.get_property_type() == Tcam.PropertyType.COMMAND:
                print(("{name}\ttype: Command\tDisplay Name: \"{disp_name}\"\tCategory: {cat}\n"
                       "\t\t\tDescription: {desc}\n"
                       "\t\t\tVisibility: {vis}\n"
                       "\t\t\t{flags}\n\n").format(name=name,
                                                   disp_name=base.get_display_name(),
                                                   cat=base.get_category(),
                                                   desc=base.get_description(),
                                                   vis=base.get_visibility(),
                                                   flags=flag_strings(base)))
        except GLib.Error as err:

            print("Error for {}: {}".format(name, err.message))


def block_until_playing(pipeline):

    while True:
        # wait 0.1 seconds for something to happen
        change_return, state, pending = pipeline.get_state(100000000)
        if change_return == Gst.StateChangeReturn.SUCCESS:
            return True
        elif change_return == Gst.StateChangeReturn.FAILURE:
            print("Failed to change state {} {} {}".format(change_return,
                                                           state,
                                                           pending))
            return False


def main():
    Gst.init(sys.argv)  # init gstreamer

    # this line sets the gstreamer default logging level
    # it can be removed in normal applications
    # gstreamer logging can contain verry useful information
    # when debugging your application
    # see https://gstreamer.freedesktop.org/documentation/tutorials/basic/debugging-tools.html
    # for further details
    Gst.debug_set_default_threshold(Gst.DebugLevel.WARNING)

    pipeline = Gst.parse_launch("tcambin name=source ! fakesink")

    if not pipeline:
        print("Unable to create pipeline")
        return 1

    # set this to a specific camera serial if you
    # do not want to use the default camera
    serial = None

    # get the tcambin to retrieve a property list through it
    source = pipeline.get_by_name("source")

    # serial is defined, thus make the source open that device
    if serial is not None:
        source.set_property("serial", serial)

    # the pipeline/tcamsrc/tcambin element must
    # at least be in Gst.State.READY
    # for a device to be open.
    # with Gst.State.NULL
    # no properties will be returned
    pipeline.set_state(Gst.State.READY)

    list_properties(source)

    # This closes the device
    # All properties are now invalid
    # and have to be deleted
    pipeline.set_state(Gst.State.NULL)

    return 0


if __name__ == "__main__":
    sys.exit(main())
