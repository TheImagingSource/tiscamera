#########
Gstreamer
#########

Gstreamer is a general purpose multimedia framework. It is the recommended way
to interact with The Imaging Source cameras.

GStreamer plugins fall into three categories, good, bad and ugly.
Yes, this is a movie reference.
No, it is not reflective of the code quality at hand.

.. list-table:: plugin types
   :header-rows: 1
   :widths: 20 80
                
   * - Plug-in set name
     - Description
   * - Good
     - This package contains the GStreamer plug-ins from the "good" set, a set of high quality plug-ins under the LGPL license.
   * - Bad
     - GStreamer Bad Plug-ins comprises a set of plug-ins not up-to-par compared to the rest. They might closely approach good-quality plug-ins, but they lack something: perhaps a good code review, some documentation, a set of tests, a real live maintainer, or some actual wide use.
   * - Ugly
     - This package contains plug-ins from the "ugly" set, a set of good-quality plug-ins that might pose distribution problems.

The advantage of using gstreamer lies in the pre-existing language bindings and the
amount of plugins available.
Through these users are offered a great amount of flexibility.
With gstreamer it is easy to save a video to a file or to rescale it.
Whereas without gstreamer a large amount of time and effort has to be invested in
video acquisition and processing now a simple pipeline suffices,
thus freeing developer resources for more important tasks.

For more information, please refer to :ref:`the gstreamer documentation<reading_gstreamer>`.
       
All information concerning a plugin can be queried by executing 'gst-inspect-1.0 elementname'

We offer the following gstreamer elements:

.. toctree::
      
   tcam-gstreamer.rst

Additionaly the following elements might prove useful:
   
.. toctree::
   
   external-gstreamer.rst

************
Introduction
************

There are a few things that can make working with easier.
The following is an incomplete list of these things.

Initialization
--------------

Always pass the commandline arguments to gstreamer.
This allows the usage of arguments like ``--gst-debug-level=5``, which ease debugging.
For an alternative, see :ref:`GStreamer Environment Variables <env_gstreamer>`.

.. tabs::

   .. code-tab:: c

      gst_init(&argc, &argv);

   .. code-tab:: python

      Gst.init(sys.argv)

Pipeline Creation
-----------------

Creating pipelines can be bothersome. A shortcut is to create the pipeline in the same
way as gst-launch. Simply write a string description and let gstreamer handle the rest.

.. tabs::

   .. code-tab:: c

      GstElement* pipeline = gst_parse_launch("tcambin ! videoconvert ! ximagesink");

   .. code-tab:: python

      pipeline = Gst.parse_launch("tcambin ! videoconvert ! ximagesink")
      
****
Caps
****

How to read caps
----------------

In general gstreamer capabilities have five fields that are used to describe the videoformat that is used.

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

Ranges can only be encountered when using GigE cameras.

Possible formats
----------------

GStreamer formats are defined by their names and format field.
Together they describe a unique format.

**video/x-raw**:
    Unstructured and uncompressed raw video data.

    Common format field entries include: ``MONO8``, ``GRAY16_LE``, ``BGRx``, ``YUYV``, ``BGR``

**video/x-bayer**:

    All bayer formats will have this name. The pixel order and compression are described in the format field.

    Officially supported by gstreamer are ``bggr``, ``rggb``, ``grbg`` and ``gbrg`` uncompressed 8-bayer patterns.
    10 to 16 bayer formats are only supported by The Imaging Source modules.

**image/jpeg**:

    This format has no additional format field.


*********
Debugging
*********

For further info, see :ref:`logging`.

***********
Environment
***********

For environment variables that change gstreamer behavior, see :any:`environment`.
