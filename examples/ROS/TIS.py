import os
import subprocess
import gi
from collections import namedtuple

gi.require_version("Gst", "1.0")
gi.require_version("Tcam", "0.1")

from gi.repository import Tcam, Gst, GLib, GObject

DeviceInfo = namedtuple("DeviceInfo", "status name identifier connection_type")
CameraProperty = namedtuple("CameraProperty", "status value min max default step type flags category group")


class TIS:
    """"""
    def __init__(self,serial, width, height, framerate, color, liveview):
        """ Constructor.
        Creates the sink pipeline and the source pipeline.

        :param serial: Serial number of the camera to use.
        :param width: Width of the video format, e.g. 640, 1920 etc,
        :param height: Height of the video format, e.g. 480, 1080
        :param framerate: Numerator of the frame rate, e.g. 15, 30, 60 etc
        :param color: If True, color is used, else gray scale
        :param liveview: If True an own live window is opened.
        """
        Gst.init([])
        self.height = height
        self.width = width
        self.sample = None
        self.samplelocked = False
        self.newsample = False
        self.pid = -1

        self.RemoveTmpFile()

        format = "BGRx"
        if not color:
            format = "GRAY8"

        if liveview:
            p = 'tcambin serial="%s" name=source ! video/x-raw,format=%s,width=%d,height=%d,framerate=%d/1' % (serial, format, width, height, framerate,)
            p += ' ! tee name=t'
            p += ' t. ! queue ! videoconvert ! video/x-raw,format=RGB ,width=%d,height=%d,framerate=%d/1! shmsink socket-path=/tmp/ros_mem'  % (width, height, framerate,)
            p += ' t. ! queue ! videoconvert ! ximagesink'
        else:
            p = 'tcambin serial="%s" name=source ! video/x-raw,format=%s,width=%d,height=%d,framerate=%d/1' % (
            serial, format, width, height, framerate,)
            p += ' ! videoconvert ! video/x-raw,format=RGB ,width=%d,height=%d,framerate=%d/1! shmsink socket-path=/tmp/ros_mem'  % (width, height, framerate,)

        print(p)

        try:
            self.pipeline = Gst.parse_launch(p)
        except GLib.Error as error:
            print("Error creating pipeline: {0}".format(err))
            raise

        self.pipeline.set_state(Gst.State.READY)
        self.pipeline.get_state(4000000000)
        # Query a pointer to our source, so we can set properties.
        self.source = self.pipeline.get_by_name("source")

        # Create gscam_config variable with content
        gscam = 'shmsrc socket-path=/tmp/ros_mem ! video/x-raw-rgb, width=%d,height=%d,framerate=%d/1' % (width, height, framerate,)
        gscam += ',bpp=24,depth=24,blue_mask=16711680, green_mask=65280, red_mask=255 ! ffmpegcolorspace'
        os.environ["GSCAM_CONFIG"] = gscam

    def Start_pipeline(self):
        """ Starts the camera sink pipeline and the rosrun process

        :return:
        """
        try:
            self.pipeline.set_state(Gst.State.PLAYING)
            self.pid = subprocess.Popen( ["rosrun", "gscam", "gscam"])

        except GLib.Error as error:
            print("Error starting pipeline: {0}".format(error))
            raise

    def Stop_pipeline(self):
        """ Stops the camera pipeline. Should also kill the rosrun process, but is not implemented

        :return:
        """
        self.pipeline.set_state(Gst.State.PAUSED)
        self.pipeline.set_state(Gst.State.READY)
        self.pipeline.set_state(Gst.State.NULL)

    def List_Properties(self):
        """ Helper function. List available properties

        :return:
        """
        for name in self.source.get_tcam_property_names():
            print( name )

    def Get_Property(self, PropertyName):
        """ Return the value of the passed property. Use List_Properties for querying names of available properties.

        :param PropertyName: Name of the property, e.g. Gain, Exposure, Gain Auto.
        :return: Current value of the property.
        """
        try:

            return CameraProperty(*self.source.get_tcam_property(PropertyName))
        except GLib.Error as error:
            print("Error get Property {0}: {1}",PropertyName, format(error))
            raise

    def Set_Property(self, PropertyName, value):
        """ Set a property. Use List_Properties for querying names of available properties.

        :param PropertyName: Name of the property, e.g. Gain, Exposure, Gain Auto.
        :param value: Value to be set.
        :return:
        """
        try:
            self.source.set_tcam_property(PropertyName,GObject.Value(type(value), value))
        except GLib.Error as error:
            print("Error set Property {0}: {1}",PropertyName, format(error))
            raise

    def Push_Property(self, PropertyName):
        """ Simplify push properties, like Auto Focus one push

        :param PropertyName: Name of the property to be pushed
        :return:
        """
        try:
            self.source.set_tcam_property(PropertyName, GObject.Value(bool, True))

        except GLib.Error as error:
            print("Error set Property {0}: {1}", PropertyName, format(error))
            raise

    def RemoveTmpFile(self):
        """ Delete the memory file used by the pipelines to share memory

        :return:
        """
        try:
            os.remove('/tmp/ros_mem')
        except OSError:
            pass

