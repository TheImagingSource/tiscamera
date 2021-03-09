#########
GStreamer
#########

GStreamer is a general-purpose multimedia framework.
GStreamer provides the best means of interacting with The Imaging Source cameras.

GStreamer plugins fall into three categories: good, bad and ugly.
Note: This is merely a movie reference and is not reflective of the code quality itself.

.. list-table:: plugin types
   :header-rows: 1
   :widths: 20 80
                
   * - Plug-in set name
     - Description
   * - Good
     - This package contains the GStreamer plugins from the "good" set: a collection of high-quality plugins under the LGPL license.
   * - Bad
     - GStreamer "bad" plugins are a collection of plugins not up-to-par compared to the rest. Their quality might closely approach "good"-quality plugins, but they lack something: a good code review, some documentation, a set of tests, a real-live maintainer, or actual wide use.
   * - Ugly
     - This package contains plugins from the "ugly" set: a collection of good-quality plugins that might pose distribution problems.

The advantage of using GStreamer lies in the pre-existing language bindings and the
number of plugins available.
These provide users a great amount of flexibility.
With GStreamer, it is easy to save a video to a file or to rescale it.
Whereas without GStreamer a large amount of time and effort has to be invested in
video acquisition and processing now a simple pipeline suffices,
thus freeing developer resources for more important tasks.

For more information, please refer to :ref:`the GStreamer documentation<reading_gstreamer>`.
       
All information concerning a plugin can be queried by executing 'gst-inspect-1.0 elementname'


It is recommended to use a GStreamer Device Monitor.

.. toctree::
   
   tcam-gstreamer-device-provider.rst


We offer the following GStreamer elements:

.. toctree::
      
   tcam-gstreamer.rst

The following elements might also prove useful:
   
.. toctree::
   
   external-gstreamer.rst

************
Introduction
************

There are a few things that can make working with GStreamer easier.
The following is an incomplete list of these things.

Initialization
--------------

Always pass command line arguments to GStreamer.
This allows the use of arguments like ``--gst-debug-level=5`` which ease debugging.
For an alternative, see :ref:`GStreamer Environment Variables <env_gstreamer>`.

.. tabs::

   .. code-tab:: c

      gst_init(&argc, &argv);

   .. code-tab:: python

      Gst.init(sys.argv)

Pipeline Creation
-----------------

Creating pipelines can be bothersome. A shortcut is to create the pipeline in the same
way as gst-launch. Simply write a string description and let GStreamer handle the rest.

.. tabs::

   .. code-tab:: c

      GstElement* pipeline = gst_parse_launch("tcambin ! videoconvert ! ximagesink");

   .. code-tab:: python

      pipeline = Gst.parse_launch("tcambin ! videoconvert ! ximagesink")

.. _gstreamer_caps:
      
****
Caps
****

How to Read Caps
----------------

In general, GStreamer capabilities have five fields that
are used to describe the video format.

These are:

**name**:
    The generic name given to the format at hand.
**format**:
    | Possible types: string, list
    | The specific format the generic format has.
**width**:
    | Possible types: int, int-range
    | Image width in pixel.
**height**:
    | Possible types: int, int-range
    | Image height in pixel.
**framerate**:
    | Possible types: GstFraction, GstValuelist, GstFractionRange
    | Other elements might also use the field `fps`. This is not used by tiscamera elements.
    | The framerate is in frames per second.

A range will be displayed in ``[]`` with the minimum and maximum values:

``video/x-bayer,format=gbrg,width=[480,1920],height=[4,1200],framerate=[34375/37184,50/1];``

A list will be displayed in ``{}`` with each possible value:

``video/x-bayer,format=gbrg,width=640,height=480,framerate={6875/3136,3/1,4/1,5/1,6/1,7/1,8/1,9/1,10/1,11/1,12/1,13/1,14/1,15/1,16/1,17/1,18/1,19/1,20/1,30/1,40/1,50/1,60/1,70/1,80/1,90/1,100/1,150/1,200/1,250/1,61875/238}``

Ranges only need to be dealt with when using GigE cameras.

Possible formats
----------------

GStreamer formats are defined by their names and format field.
Together they describe a unique format.

**video/x-raw**:
    Unstructured and uncompressed raw video data.

    Common format field entries include: ``MONO8``, ``GRAY16_LE``, ``BGRx``, ``YUYV``, ``BGR``

**video/x-bayer**:

    All bayer formats will have this name. The pixel order and compression are described in the format field.

    Officially supported by GStreamer are ``bggr``, ``rggb``, ``grbg`` and ``gbrg`` uncompressed 8 bit Bayer patterns.
    10 to 16 bit Bayer formats are only supported by The Imaging Source modules.

**image/jpeg**:

    This format has no additional format field.

*******************
General Suggestions
*******************

Set `sync=false` on your sink element. This will prevent images from arriving later than necessary.
    
*********
Debugging
*********

For further info, see :ref:`logging`.

***********
Environment
***********

For environment variables that change gstreamer behavior, see :any:`environment`.
