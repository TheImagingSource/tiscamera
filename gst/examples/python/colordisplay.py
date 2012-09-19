#
# Example to display a color image with
# The Imaging Source USB CMOS cameras
#

import gst
import glib
import gobject

#
# Change the DEVICE and width/height according to your camera
# ( Max width/height are:
#   Dxx 72UC02: 2592x1944
#

DEVICE = "/dev/video1"
WIDTH = 2592
HEIGHT = 1944


# GStreamer is multithreaded so threading needs to be initialized to avoid blocking!
gobject.threads_init ()

def on_new_buffer (sink, buffer, pad, src):
    structure = buffer.get_caps ().get_structure (0)
    if buffer.size != (structure["width"] * structure["height"]):
	return
    bayer_caps = gst.Caps ("video/x-raw-bayer,format=gbrg,width=%d,height=%d" %(
	structure["width"],
	structure["height"]))
    buffer.set_caps (bayer_caps)
    src.emit ("push-buffer", buffer)

video_pipeline = gst.parse_launch ('v4l2src device=%s always-copy=true ! video/x-raw-gray,width=%d,height=%d ! fakesink signal-handoffs=true name="sink"' %
				   (DEVICE, WIDTH, HEIGHT))

display_pipeline = gst.parse_launch ('appsrc name="src" ! queue ! bayer2rgb name="debayer"')
link = display_pipeline.get_by_name ("debayer")

colorspace = gst.element_factory_make ("cogcolorspace")
display = gst.element_factory_make ("ximagesink")
display.set_property("sync", False)
display_pipeline.add (colorspace, display)
gst.element_link_many (link, colorspace, display)

sink = video_pipeline.get_by_name ("sink")
src = display_pipeline.get_by_name ("src")

sink.connect ("handoff", on_new_buffer, src)

print "Starting video pipeline"
display_pipeline.set_state (gst.STATE_PLAYING)
print "Starting display pipeline"
video_pipeline.set_state (gst.STATE_PLAYING)

loop = glib.MainLoop()

loop.run()
