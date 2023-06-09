.. _gstreamer:

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
**binning**:
    | Possible types: string
    | The value always has the structure `AxB`.
    |
    | `A` describes the horizontal binning factor.
    | `B` describes the vertical binning factor.
    | 
    | Example:
    | `2x2` describes factor 2 binning in both directions.
    | 
    | The default value is '1x1'.
    | This is implied for all cameras and can be ignored/left out.
**skipping**:
    | Possible types: string
    | The value always has the structure `AxB`.
    |
    | `A` describes the horizontal skipping factor.
    | `B` describes the vertical skipping factor.
    | 
    | Example:
    | `2x2` describes factor 2 skipping in both directions.
    | 
    | The default value is '1x1'.
    | This is implied for all cameras and can be ignored/left out.

A range will be displayed in ``[]`` with the minimum and maximum values:

``video/x-bayer,format=gbrg,width=[480,1920],height=[4,1200],framerate=[34375/37184,50/1];``

Ranges only need to be dealt with when using GigE cameras.

A list will be displayed in ``{}`` with each possible value:

``video/x-bayer,format=gbrg,width=640,height=480,framerate={6875/3136,3/1,4/1,5/1,6/1,7/1,8/1,9/1,10/1,11/1,12/1,13/1,14/1,15/1,16/1,17/1,18/1,19/1,20/1,30/1,40/1,50/1,60/1,70/1,80/1,90/1,100/1,150/1,200/1,250/1,61875/238}``

.. warning::

   when dealing with binning/skipping the GstCaps that are retrievable will differ depending on the backend (v4l2/aravis).

   Cameras that use the aravis backend will only mention binning/skipping in the range description.
   As the GstCaps with fixed resolutions are typically generated from these ranges the binning/skipping values can also be used with them.

   V4L2 only has fixed caps. They will be explicitly listed.
   

Possible formats
----------------

GStreamer formats are defined by their names and format field.
Together they describe a unique format.

**video/x-raw**:
    Unstructured and uncompressed raw video data.

    Common format field entries include: ``GRAY8``, ``GRAY16_LE``, ``BGRx``, ``YUYV``, ``BGR``

**video/x-bayer**:

    All bayer formats will have this name. The pixel order and compression are described in the format field.

    Officially supported by GStreamer are ``bggr``, ``rggb``, ``grbg`` and ``gbrg`` uncompressed 8 bit Bayer patterns.
    10 to 16 bit Bayer formats are only supported by The Imaging Source modules.

**image/jpeg**:

    This format has no additional format field.


Different Memory
----------------

.. note::

   This is only required when dealing with tegra systems.
   Common pipelines that use the tcambin and/or tcamdutils and other platforms do not require this.

Custom memory types are described as ``video/x-raw(memory:NVMM)`` or similar.
This additional information are not part of the GstStructure that contains all caps information but are instead stored as GstCapsFeatures.

.. tabs::

   .. code-tab:: c

      GstCaps* caps = gst_caps_from_string("video/x-raw(memory:NVMM),format=BGRx");
      for (guint i = 0; i < gst_caps_get_size(all_caps); ++i)
      {
          // must not be freed
          GstCapsFeatures* features = gst_caps_get_features(all_caps, i);

          if (features)
          {
              if (gst_caps_features_contains(features, "memory:NVMM"))
              {
                  // do something with this information
              }
          }
      }

   .. code-tab:: python

      caps = Gst.Caps.from_string("video/x-raw(memory:NVMM),format=BGRx")
      for x in range(caps.get_size()):

          features = caps.get_features(x)
          if features:
              if features.contains("memory:NVMM"):
                  # do something with this information

********
Querying
********

It is possible to query a GstElement for specific information. For this, `GstQuery <https://gstreamer.freedesktop.org/documentation/gstreamer/gstquery.html>`_ is used.

Caps Query
----------

The Caps query allows the user to query the boundaries if wanted caps.

A common usage would be to verify the accepted framerate range for a specific resolution.

.. tabs::

   .. group-tab:: c
                  
      .. code-block:: c

         GstCaps* filter = gst_caps_from_string("video/x-bayer,format=rggb,width=1920,height=1080");
         GstQuery* query = gst_query_new_caps (filter);

         gboolean ret = gst_element_query(tcamsrc, query);
         if (ret)
         {
             GstCaps* result_caps = NULL;
             gst_query_parse_caps_result(query, &result_caps);

             const char* str = gst_caps_to_string(result_caps);
             // result_caps will contain the allowed framerates
             // video/x-bayer,format=rggb,width=1920,height=1080,framerate={75/1,30/1}
             printf("Caps are: %s\n", str);
             g_free(str);
             // result caps are still owned by query
             
         }
         else
         {
             // query could not be performed
         }

         gst_query_unref(query);

   .. group-tab:: python
         
      .. code-block:: python

         filter = Gst.Caps.from_string("video/x-bayer,format=rggb,width=1920,height=1080")
         query = Gst.Query.new_caps(filter)

         if tcamsrc.query(query):

             result_caps = query.parse_caps_result()
             # result_caps will contain the allowed framerates
             # video/x-bayer,format=rggb,width=1920,height=1080,framerate={75/1,30/1}
             print("Caps are: {}".format(result_caps.to_string()))
             
         else:

             # unable to perform query
         
         

Accept Caps Query
-----------------

The Accept Caps query allows the verification of fixed caps.
This results in a simple yes/no mechanism, showing if the specified caps are valid.

.. tabs::

   .. group-tab:: c

      .. code-block:: c

         GstQuery* query = gst_query_new_accept_caps (GstCaps * caps);

         gboolean ret = gst_element_query(tcamsrc, query);
         if (ret)
         {
             gboolean result;
             gst_query_parse_accept_caps_result(query, &result);

             if (result)
             {
                  // caps are accepted
             }
             else
             {
                 // caps are not accepted
             }
         }
         else
         {
             // query could not be performed
         }
         gst_query_unref(query);

   .. group-tab:: python
                               
      .. code-block:: python

         c = Gst.Caps.from_string("video/x-bayer,format=rggb,width=2448,height=2048,framerate=70/1")

         query = Gst.Query.new_accept_caps(c)
         if tcamsrc.query(query):
         
             if (query.parse_accept_caps_result()):
                 print("Caps are accepted")
             else:
                 print("Caps are accepted")
                 
         else:
             print("Query could not be performed")

   
******************************
Negotiations & Transformations
******************************

As a multimedia framework GStreamer not only allows for image retrieval but also transformations between formats.
For this it is often necessary to add additional caps to a pipeline, so that elements will know what to expect and
negotiate with other elements accordingly.

GStreamer elements commonly have one input and one output pad.
Exceptions are sources (only out pads) and sinks (only input pads).
Negotiations happen upstream, meaning the element that will receive images will tell the sending element what kind of format it expects.

If multiple caps formats are acceptable they will be listed together.
The sending element can then decide what to use and reduce the offered caps further.

.. code-block:: sh

   tcamsrc ! appsink

In this pipeline `appsink` will offer the caps `ANY`, meaning any fixed caps will be accepted.
This leads the `tcamsrc` to select the caps it thinks a best.

For information about tcamsrc auto negotiation see :ref:`here <tcamsrc_caps_auto_selection>`.


The simplest way to see what caps the camera offers is `tcam-ctrl`.
   
.. code-block:: sh

   tcam-ctrl -c <SERIAL>

   
.. code-block:: sh

   tcamsrc ! video/x-bayer,format=bggr,width=640,height=480,framerate=30/1 ! appsink

   
In this pipeline the appsink get filtered by the given caps. Since the appsink caps are `ANY` this will always succeed.
The tcamsrc then filters its out going caps with the given caps. If the result is `EMPTY` a negotiation error will be returned,
resulting in a failed negotiation.

.. code-block:: sh

   tcamsrc \
   ! video/x-bayer,format=bggr,width=640,height=480,framerate=30/1;\
   video/x-bayer,format=bggr16,width=640,height=480,framerate=30/1 \
   ! appsink

   
This pipeline is the same as the one before, except that the tcamsrc can choose between bggr and bggr16.
Due to its own preferences it will use bggr.
For information about tcamsrc auto negotiation see :ref:`here <tcamsrc_caps_auto_selection>`.

.. code-block:: sh

   tcamsrc ! tcamconvert ! video/x-raw,format=BGRx ! appsink

Here tcamconvert is being told that the appsink will only accept BGRx.
This leads to tcamconvert only using caps for the negotiations with tcamsrc that can be converted to BGRx.
Since multiple formats can be converted to BGRx the tcamsrc once again can choose according to its own preferences.

.. code-block:: sh

   tcam-ctrl --transform --out video/x-raw,format=BGRx -e tcamconvert

To prevent auto selection explicit caps have to be given.

.. code-block:: sh

   tcamsrc ! video/x-raw,format=rggb16 ! tcamconvert ! video/x-raw,format=BGRx ! appsink

This pipeline will retrieve bayer 16-bit from the tcamsrc and let tcamconvert transform it to BGRx.
The rest of the caps (width, height, framerate) are still auto negotiated.

tcambin
-------

The previous description of negotiations summarizes much of the internal behavior of the tcambin.
The tcambin is a source bin, meaning it acts as a wrapper around a source element and includes additional steps.

These steps are transformations and image adjustments like white balance.

Like the previous examples that required an explicit restriction of the source caps for the transformation element to
work as wanted, the tcambin will require this too.

.. code-block:: sh

   tcamsrc ! video/x-bayer,format=rggb16 ! tcamconvert ! video/x-raw,format=BGRx ! appsink

is equal to:

.. code-block:: sh

   tcambin device-caps=video/x-bayer,format=rggb16 ! video/x-raw,format=BGRx ! appsink


*******************
General Suggestions
*******************

Set `sync=false` on your sink element. This will prevent images from arriving later than necessary.

Add a queue element to separate the image creation fro the image processing. Be aware that this will create an additional image copy.

*********
Debugging
*********

For further info, see :ref:`logging`.

***********
Environment
***********

For environment variables that change gstreamer behavior, see :ref:`environment`.
