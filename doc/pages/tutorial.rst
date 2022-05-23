
.. _tutorial:

########
Tutorial
########

This page contains a extended tutorial on how to get started with The Imaging Source Cameras.
For a simpler version, read the README.md in the project's root directory.

.. _tutorial_setup:

=====
Setup
=====

To install tiscamera one can choose between manual compilation
and installation of precompiled ubuntu packages.

The precompiled packages can be found on `github <https://github.com/TheImagingSource/tiscamera/releases>`_
or in the `Linux download section <https://www.theimagingsource.com/support/downloads-for-linux/>`_.

| The packages contain support for all potential camera types.
| For a minimal setup it is recommended to manually compile.

The first half of this tutorial describes the configuration and build process
required to build tiscamera on a local PC.
For stable releases, precompiled .deb-files are available: see :ref:`packaging`.

.. _tutorial_cloning:

Cloning
=======

To retrieve the code, clone it from github:

.. code-block:: sh

   git clone https://github.com/TheImagingSource/tiscamera.git

For this, ``git`` must be installed.

Now change into the tiscamera directory:

.. code-block:: sh

   cd tiscamera

.. _tutorial_dependencies:
   
Dependencies
============

The project requires multiple dependencies to compile.
For a complete list of dependencies, see :ref:`dependencies`.

To install all dependencies, execute the following command in the tiscamera directory:

.. code-block:: sh

   ./scripts/dependency-manager install

To install only runtime or compilation dependencies use the additional arguments `--runtime` and `--compilation`.

.. _tutorial_configuration:

Configuration
=============

tiscamera is configured using ``cmake`` which
allows the (de)activation of entire sections of code.
To configure the project, call `cmake` from the build directory.

For an overview of available cmake options, see :any:`configuring`.

To interactively change options, use the program ``cmake-gui``.
Under Debian/Ubuntu, it can be installed with ``sudo apt install cmake-qt-gui``.

.. code-block:: sh

   mkdir build
   cd build
   cmake ..


Compilation
===========

.. code-block:: sh

   make -j

Installation
============

The default configuration of tiscamera will install into `/usr`.
This means all libraries, etc. will be available to all users.

Running without Installation
----------------------------

To integrate tiscamera into the system environment, source the `env.sh` script located in the build directory.
It will adjust environment variables so that GStreamer elements, etc can be found.

To do this call the following in the current terminal:

.. code-block:: sh
   
   source <tiscamera build directory>/env.sh
                

Verifying the Installation
==========================

To ensure that all libraries are correctly found, execute one of the following commands after connecting the camera.

``tcam-capture`` - The graphical example program that ships with tiscamera.

``gst-launch-1.0 tcambin ! video/x-raw,format=BGRx ! videoconvert ! ximagesink`` - GStreamer commandline that works with every camera.
   
===================
Camera Interactions
===================

This sections describes how a program can interact with a camera.

.. tabs::

   .. group-tab:: c

      A pkg-config named `tcam` is available.

   .. group-tab:: python

      No additional steps necessary.
      
      For custom installations :ref:`custom environment variables<environment>` may be necessary.

The API
=======

The tiscamera API consists of two parts: the tiscamera GStreamer elements and a GObject Interface.
For a technical overview of the API, continue reading here: :any:`api`.

To reference both APIs, add the following lines:

.. tabs::

   .. group-tab:: c

      .. code-block:: c
                  
         #include <gst/gst.h>
         #include <tcam-property-1.0.h>
                  
   .. group-tab:: python

      .. code-block:: python
                  
         import gi

         gi.require_version("Tcam", "1.0")
         gi.require_version("Gst", "1.0")

         from gi.repository import Tcam, Gst

The required libraries are mostly provided by gobject introspection and gstreamer.
The only tiscamera library that has to explicitly linked is `libtcam-property.so`.

It is recommended to use helper tools such as `pkg-config` or a cmake wrapper or similar for library lookup.

For real life examples, please refer to the :ref:`examples folder<examples_building>`

.. tabs::

   .. group-tab:: c

      Makefile settings could look like this:

      .. code-block:: Makefile

         LIBS:=$(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config gstreamer-1.0 --libs)
         LIBS:=$(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config gstreamer-video-1.0 --libs)
         LIBS+=$(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config gobject-introspection-1.0 --libs)
         LIBS+=$(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config tcam --libs)

         CFLAGS:=$(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config gstreamer-1.0 --cflags)
         CFLAGS:=$(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config gstreamer-video-1.0 --cflags)
         CFLAGS+=$(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config gobject-introspection-1.0 --cflags)
         CFLAGS+=$(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config tcam --cflags)

   .. group-tab:: python

      Automatically handled by gobject introspection.

      For custom installations set `GI_TYPELIB_PATH` to where the file `Tcam-1.0.typelib` is installed.
         
.. _tutorial_discovery:
         
Camera Discovery
================

Listing Available Cameras
-------------------------

For a quick listing of available devices, execute the following in a terminal:

.. code-block:: sh

   tcam-ctrl -l

.. tabs::

   .. group-tab:: c

      .. code-block:: c

         gst_init(&argc, &argv); // init gstreamer

         GstDeviceMonitor* monitor = gst_device_monitor_new();
         // We are only interested in devices that are in the categories
         // Video and Source and tcam
         gst_device_monitor_add_filter(monitor, "Video/Source/tcam", NULL);

         GList* devices = gst_device_monitor_get_devices(monitor);

         for (GList* elem = devices; elem; elem = elem->next)
         {
             GstDevice* device = (GstDevice*) elem->data;

             GstStructure* struc = gst_device_get_properties(device);

             printf("\tmodel:\t%s\tserial:\t%s\ttype:\t%s\n",
                    gst_structure_get_string(struc, "model"),
                    gst_structure_get_string(struc, "serial"),
                    gst_structure_get_string(struc, "type"));

             gst_structure_free(struc);
         }

         g_list_free_full(devices, gst_object_unref);
         gst_object_unref(monitor);


   .. group-tab:: python

      .. code-block:: python

         Gst.init(sys.argv)
                      
         monitor = Gst.DeviceMonitor.new()
         # We are only interested in devices that are in the categories
         # Video and Source and tcam
         monitor.add_filter("Video/Source/tcam")

         for device in monitor.get_devices():

             struc = device.get_properties()

             print("\tmodel:\t{}\tserial:\t{}\ttype:\t{}".format(struc.get_string("model"),
                                                                 struc.get_string("serial"),
                                                                 struc.get_string("type")))


This code can be found in the example :ref:`00-list-devices<examples_list_devices>`.

Opening and Closing a Camera
----------------------------

The recommended way of addressing a camera is by using its serial number.


.. tabs::

   .. group-tab:: c

      .. code-block:: c
                   
         /* create a tcambin to retrieve device information */
         GstElement* source = gst_element_factory_make("tcambin", "source");
   
         const char* serial = NULL;
   
         if (serial != NULL)
         {
             GValue val = {};
             g_value_init(&val, G_TYPE_STRING);
             g_value_set_static_string(&val, serial);
   
             g_object_set_property(G_OBJECT(source), "serial", &val);
         }
   
         /* in the READY state the camera will be initialized
            and properties will be available */
         gst_element_set_state(source, GST_STATE_READY);

   .. group-tab:: python

      .. code-block:: python
                  
         # Set this to a serial string for a specific camera
         serial = None

         source = Gst.ElementFactory.make("tcambin")
   
         if serial:
             # This is gstreamer set_property
             source.set_property("serial", serial)
   
         # in the READY state the camera will be initialized
         # and properties will be available
         source.set_state(Gst.State.READY)

To close a device, it is sufficient to set the GStreamer state to NULL.
All hardware resources will be freed.
                  
.. tabs::

   .. group-tab:: c

      .. code-block:: c

         gst_element_set_state(source, GST_STATE_NULL);

         gst_object_unref(source);

   .. group-tab:: python

      .. code-block:: python

         # cleanup, reset state
         source.set_state(Gst.State.NULL)
                           
This code can be found in the example :ref:`02-set-properties<examples_set_properties>`.


.. _tutorial_streaming:

Streaming
=========

For image retrieval, use the GStreamer element :any:`tcamsrc`.

Available Caps
--------------

For an overview of supported GStreamer caps, type the following into a terminal:

.. code-block:: sh

   tcam-ctrl -c <SERIAL>

The printed caps are GStreamer compatible and can be copy-pasted for configuration purposes.


.. tabs::

   .. group-tab:: c

      .. code-block:: c

         /* create a tcambin to retrieve device information */
         GstElement* source = gst_element_factory_make("tcambin", "source");

         /* Setting the state to ready ensures that all resources
         are initialized and that we really get all format capabilities */
         gst_element_set_state(source, GST_STATE_READY);

         GstPad* pad = gst_element_get_static_pad(source, "src");

         GstCaps* caps = gst_pad_query_caps(pad, NULL);

   .. group-tab:: python

      .. code-block:: c
                  
         source = Gst.ElementFactory.make("tcambin")
         source.set_state(Gst.State.READY)
         caps = source.get_static_pad("src").query_caps()

This code can be found in the example :ref:`04-list-formats<examples_list_format>`.

            
Setting Caps
------------

.. tabs::

   .. group-tab:: c

      .. code-block:: c

         GError* err = NULL;
         const char* pipeline_desc = "tcambin name=source ! capsfilter name=filter ! videoconvert ! ximagesink";
         GstElement* pipeline = gst_parse_launch(pipeline_desc, &err);
         
         GstCaps* caps = gst_caps_new_empty();
         GstStructure* structure = gst_structure_from_string("video/x-raw", NULL);
         gst_structure_set(structure,
                           "format", G_TYPE_STRING, "BGRx",
                           "width", G_TYPE_INT, 640,
                           "height", G_TYPE_INT, 480,
                           "framerate", GST_TYPE_FRACTION, 30, 1,
                           NULL);
         gst_caps_append_structure (caps, structure);
         
         GstElement* capsfilter = gst_bin_get_by_name(GST_BIN(pipeline), "filter");
         
         g_object_set(G_OBJECT(capsfilter), "caps", caps, NULL);
         gst_object_unref(capsfilter);
         gst_caps_unref(caps);

                  
   .. group-tab:: python

      .. code-block:: python

         pipeline = Gst.parse_launch("tcambin name=bin"
                                     " ! capsfilter name=filter"
                                     " ! videoconvert"
                                     " ! ximagesink")
   
         caps = Gst.Caps.new_empty()
   
         structure = Gst.Structure.new_from_string("video/x-raw")
         structure.set_value("width", 640)
         structure.set_value("height", 480)
   
         try:
             fraction = Gst.Fraction(30, 1)
             structure.set_value("framerate", fraction)
         except TypeError:
             struc_string = structure.to_string()
   
             struc_string += ",framerate={}/{}".format(30, 1)
             structure.free()
             structure, end = structure.from_string(struc_string)

                  
This code can be found in the example :ref:`05-set-format<examples_set_format>`.

As an alternative to creating the GstCaps manually, you can also use ``gst_caps_from_string``.
This function takes a format string description and converts it to a valid GstCaps instance.
For more information, see :any:`the caps reference section.<gstreamer_caps>`.

Showing a Live Image
--------------------

In order to display a live image, a display sink is required.

Depending on the system being used, some display sinks may work better than others.
Generally, the `ximagesink` is a good starting point.

A simple pipeline would look like this:

``tcambin ! videoconvert ! ximagesink``

Working code can be found in the example :ref:`03-live-stream<examples_live_stream>`.

An alternative to simple trial-and-error setups is the use of the program ``gst-launch-1.0``.
This program enables the creation of pipelines on the command line, allowing for quick setups. 


Receiving Images
----------------

The easiest approach is to use an appsink.
The appsink element will call a function for each new image it receives.

To enable image retrieval, the following steps need to be taken.

.. tabs::

   .. group-tab:: c

      .. code-block:: c

         const char* pipeline_str = "tcambin name=source ! videoconvert ! appsink name=sink";

         GError* err = NULL;
         GstElement* pipeline = gst_parse_launch(pipeline_str, &err);
         /* retrieve the appsink from the pipeline */
         GstElement* sink = gst_bin_get_by_name(GST_BIN(pipeline), "sink");
   
         // tell appsink to notify us when it receives an image
         g_object_set(G_OBJECT(sink), "emit-signals", TRUE, NULL);
   
         // tell appsink what function to call when it notifies us
         g_signal_connect(sink, "new-sample", G_CALLBACK(callback), NULL);
   
         gst_object_unref(sink);
                  
   .. group-tab:: python

      .. code-block:: python

         pipeline = Gst.parse_launch("tcambin name=source"
                                     " ! videoconvert"
                                     " ! appsink name=sink")

         sink = pipeline.get_by_name("sink")

         # tell appsink to notify us when it receives an image
         sink.set_property("emit-signals", True)

         user_data = "This is our user data"

         # tell appsink what function to call when it notifies us
         sink.connect("new-sample", callback, user_data)
                  
The image `sample` that is given to the function contains the image, video caps and other additional information that may be required for image processing.


.. tabs::

   .. group-tab:: c

      .. code-block:: c

         /*
         This function will be called in a separate thread when our appsink
         says there is data for us. user_data has to be defined
         when calling g_signal_connect. It can be used to pass objects etc.
         from your other function to the callback.
         */
         static GstFlowReturn callback (GstElement* sink, void* user_data)
         {
             GstSample* sample = NULL;
             /* Retrieve the buffer */
             g_signal_emit_by_name(sink, "pull-sample", &sample, NULL);

             if (sample)
             {
                 GstBuffer* buffer = gst_sample_get_buffer(sample);

                 // delete our reference so that gstreamer can handle the sample
                 gst_sample_unref (sample);
             }
             return GST_FLOW_OK;
         }
                  
   .. group-tab:: python

      .. code-block:: python

         def callback(appsink, user_data):
             """
             This function will be called in a separate thread when our appsink
             says there is data for us. user_data has to be defined
             when calling g_signal_connect. It can be used to pass objects etc.
             from your other function to the callback.
             """
             sample = appsink.emit("pull-sample")

             if sample:

                 caps = sample.get_caps()

                 gst_buffer = sample.get_buffer()

                 try:
                     (ret, buffer_map) = gst_buffer.map(Gst.MapFlags.READ)
                 finally:
                     gst_buffer.unmap(buffer_map)

             return Gst.FlowReturn.OK

This code can be found in the example :ref:`07-appsink<examples_appsink>`.

.. _tutorial_properties:

Properties
==========

The camera offers multiple properties to assist with image acquisition.
Depending on the device at hand, these properties include functions
such as software trigger, exposure, and complete auto adjustment algorithms.

Get/List Properties
-------------------

The responsible function is `tcam_property_provider_get_tcam_property_names`.

For an overview of available properties, type the following into a terminal:

.. code-block:: sh

   tcam-ctrl -p <SERIAL>

.. tabs::

   .. group-tab:: c

      .. code-block:: c

         /* create a tcambin to retrieve device information */
         GstElement* source = gst_element_factory_make("tcambin", "source");
         
         gst_element_set_state(source, GST_STATE_READY);

         GError* err = NULL;
         GSList* n =  tcam_property_provider_get_tcam_property_names(TCAM_PROPERTY_PROVIDER(source), &err);

         for (unsigned int i = 0; i < g_slist_length(names); ++i)
         {
             err = NULL;
             const char* name = (char*)cur->data;

             TcamPropertyBase* base_property = tcam_property_provider_get_tcam_property(TCAM_PROPERTY_PROVIDER(source),
                                                                                        name, &err);

             if (err)
             {
                 printf("Error while retrieving property \"%s\": %s\n", name, err->message);
                 g_error_free(err);
                 err = NULL;
                 continue;
             }

             if (!base_property)
             {
                 printf("Could not query property '%s'\n", name);
                 continue;
             }

             TcamPropertyType type = tcam_property_base_get_property_type(base_property);

             switch(type)
             {
                 // differentiate between specific property types
                 // ...
             }
             
             if (base_property)
             {
                 g_object_unref(base_property);
             }
         }

         g_slist_free_full(names, g_free);
         gst_element_set_state(source, GST_STATE_NULL);
         gst_object_unref(source);
                     
   .. group-tab:: python

      .. code-block:: python
                      
         # we create a source element to retrieve a property list through it
         camera = Gst.ElementFactory.make("tcambin")

         # serial is defined, thus make the source open that device
         property_names = camera.get_tcam_property_names()

         for name in property_names:

             try:
                 base_property = camera.get_tcam_property("name")

                 property_type = base_property.get_property_type()

                 # differentiate between specific property types
                 
             except GLib.Error as e:
                 print("could not receive value {}".format(name))
                  
This code can be found in the example :ref:`01-list-properties<examples_list_properties>`.

A differentiation between property types is necessary to have access to property values.



  
Set Property
------------



.. tabs::

   .. group-tab:: c

      .. code-block:: c
                  
         /* create a tcambin to retrieve device information */
         GstElement* source = gst_element_factory_make("tcambin", "source");

         gst_element_set_state(source, GST_STATE_READY);

         GError* err = NULL;
         GSList* n =  tcam_property_provider_get_tcam_property_names(TCAM_PROPERTY_PROVIDER(source), &err);

         tcam_property_provider_set_enumeration(TCAM_PROPERTY_PROVIDER(source), "ExposureAuto", "Off");
         // tcam_property_provider_set_float(TCAM_PROPERTY_PROVIDER(source), "ExposureTime", 10000.0);
         // tcam_property_provider_set_integer(TCAM_PROPERTY_PROVIDER(source), "Brightness", 128);
         // tcam_property_provider_set_boolean(TCAM_PROPERTY_PROVIDER(source), "ExposureAuto", "Off");
         
         if (err)
         {
             printf("Error while retrieving names: %s\n", err->message);
             g_error_free(err);
             err = NULL;
         }
         

                  
   .. group-tab:: python

      .. code-block:: python

         camera = Gst.ElementFactory.make("tcambin")

         # in the READY state the camera will always be initialized
         camera.set_state(Gst.State.READY)

         try:
             camera.set_tcam_enumeration("ExposureAuto", "Off")
             # camera.set_tcam_float("ExposureTime", 10000.0)
             # camera.set_tcam_integer("Brightness", 128)
             # camera.set_tcam_enumeration("ExposureAuto", "Off")
         except GLib.Error as err:
             print("Unable to set error: {}", err.message)

                  
This code can be found in the example :ref:`02-set-properties<examples_set_properties>`.

Where to go from here
=====================

Take a look at our :any:`reference`, the :any:`GStreamer documentation<reading_gstreamer>` or :ref:`ask us a question<contact>`.

For extended examples (including OpenCV, ROS and GUI frameworks), please have a look at our :ref:`extended examples<examples_further>`.
