import gst
import gobject
import os

VIDEODEVICE = "/dev/video1"
WIDTH = 1280
HEIGHT = 960
FRAMERATE = "15/1"

try:
    import psutil
except ImportError:
    psutil = None

def show_resources_cb (*args):
    process = psutil.Process(os.getpid())
    if getattr(process, "memory_info"):
        print ("Resource usage: %dkB" % (int(process.memory_info()[0]) / 1024))
    elif getattr (process, "get_memory_info"):
        print ("Resource usage: %dkB" % (int(process.memory_info()[0]) / 1024))
    else:
        print ("Unsupported psutil module version")
    return True

def bus_watch(bus, message):
    if message.type == gst.MESSAGE_ERROR:
        print ("Got error message: ", message)
    return True
    
loop = gobject.MainLoop()

source = gst.element_factory_make ("v4l2src")
source.set_property("device", VIDEODEVICE)
flt1 = gst.element_factory_make ("capsfilter")
flt1.set_property("caps", gst.Caps("video/x-raw-gray,width=%d,height=%d,framerate=(fraction)%s" % (WIDTH, HEIGHT, FRAMERATE)))
autoexp = gst.element_factory_make ("tis_auto_exposure")
autoexp.set_property("auto-exposure", True)
bufferfilter = gst.element_factory_make ("tisvideobufferfilter")
csp = gst.element_factory_make ("ffmpegcolorspace")
scale = gst.element_factory_make ("videoscale")
flt2 = gst.element_factory_make ("capsfilter")
flt2.set_property("caps", gst.Caps("video/x-raw-yuv,width=640,height=480"))
sink = gst.element_factory_make ("xvimagesink")

pipeline = gst.Pipeline()
pipeline.get_bus().add_watch(bus_watch)

pipeline.add_many (source, flt1, autoexp, bufferfilter, csp, scale, flt2, sink)

source.link(flt1)
flt1.link(autoexp)
autoexp.link(bufferfilter)
bufferfilter.link(csp)
csp.link(scale)
scale.link(flt2)
flt2.link(sink)

print ("Starting Pipeline")
pipeline.set_state(gst.STATE_PLAYING)

if psutil:
    gobject.timeout_add_seconds (1,show_resources_cb)
else:
    print ("Install psutil package to get resource usage information")

loop.run()
