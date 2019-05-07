
########
Tutorial
########

This page contains a extended tutorial on how to get started with The Imaging Source Cameras.
For a simpler version, read the README.md in the project's root directory.

=====
Setup
=====

The first half of this tutorial describes the configuration and build process
required to build tiscamera on a local PC.
For stable releases, a precompiled .deb-file is available: see :ref:`packaging`.

Currently only amd64 releases are available.

Cloning
=======

To retrieve the code, clone it from github:

.. code-block:: sh

   git clone https://github.com/TheImagingSource/tiscamera.git

For this, ``git`` must be installed.

Now change into the tiscamera directory and create a build directory:

.. code-block:: sh

   cd tiscamera
   mkdir build

Dependencies
============

The project requires multiple dependencies to compile.
For a complete list of dependencies, see :any:`dependencies`.

To install all dependencies, execute the following command in the tiscamera directory:

.. code-block:: sh

   ./scripts/install-dependencies.sh --runtime --compilation


Configuration
=============

tiscamera is configured using ``cmake`` which
allows the (de)activation of entire sections of code.
To configure the project, call `cmake` from the build directory.

For an overview of available cmake options, see :any:`configuring`.

To interactively change options, use the program ``cmake-gui``.
Under Debian/Ubuntu, it can be installed with ``sudo apt install cmake-qt-gui``.


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

Verifying the Installation
==========================

To ensure that all libraries are correctly found, execute one of the following commands after connecting the camera.

``tcam-capture`` - The graphical example program that ships with tiscamera.

``gst-launch-1.0 tcambin ! video/x-raw,format=BGRx ! videoconvert ! ximagesink`` - GStreamer commandline that works with every camera.
   
===================
Camera Interactions
===================

This sections describes how a program can interact with a camera.

The API
=======

The tiscamera API consists of two parts: the tiscamera GStreamer elements and a GObject Interface.
For a technical overview of the API, continue reading here: :any:`api`.

To reference both APIs, add the following lines:

.. tabs::

   .. group-tab:: c

      .. code-block:: c
                  
         #include <gst/gst.h>
         #include <tcamprop.h>
                  
   .. group-tab:: python

      .. code-block:: python
                  
         import gi

         gi.require_version("Tcam", "0.1")
         gi.require_version("Gst", "1.0")

         from gi.repository import Tcam, Gst
                  
Camera Discovery
================

Listing Available Cameras
-------------------------

For a quick listing of available devices, execute the following in a terminal:

.. code-block:: sh

   tcam-ctrl -l

The responsible functions are :c:func:`tcam_prop_get_device_serials`
and :c:func:`tcam_prop_get_device_info`

.. tabs::

   .. group-tab:: c

      .. literalinclude:: ../../examples/c/00-list-devices.c
         :language: c
         :lines: 28-62
         :emphasize-lines: 7, 23-27
         :linenos:
         :dedent: 4

   .. group-tab:: python

      .. literalinclude:: ../../examples/python/00-list-devices.py
         :language: python
         :lines: 34-57
         :linenos:
         :dedent: 4

This code can be found in the example `00-list-devices`.

Opening and Closing a Camera
----------------------------

The recommended way of addressing a camera is by using its serial number.


.. tabs::

   .. group-tab:: c

      .. literalinclude:: ../../examples/c/02-set-properties.c
         :language: c
         :lines: 86-101
         :linenos:
         :dedent: 4
  
   .. group-tab:: python

      .. literalinclude:: ../../examples/python/02-set-properties.py
         :language: python
         :lines: 71-81
         :linenos:
         :dedent: 4

To close a device, it is sufficient to set the GStreamer state to NULL
which will free up all hardware resources.
                  
.. tabs::

   .. group-tab:: c

      .. literalinclude:: ../../examples/c/02-set-properties.c
         :language: c
         :lines: 138-141
         :linenos:
         :dedent: 4

   .. group-tab:: python

      .. literalinclude:: ../../examples/python/02-set-properties.py
         :language: python
         :lines: 95-96
         :linenos:
         :dedent: 4

                  
This code can be found in the example `02-set-properties`.
            
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

      .. literalinclude:: ../../examples/c/04-list-formats.c
         :language: c
         :lines: 33-35, 45-52
         :linenos:
         :dedent: 4

   .. group-tab:: python

      .. literalinclude:: ../../examples/python/04-list-formats.py
         :language: python
         :lines: 112, 124, 34
         :linenos:
         :dedent: 4

This code can be found in the example `04-list-formats`.

            
Setting Caps
------------

.. tabs::

   .. group-tab:: c

      .. literalinclude:: ../../examples/c/05-set-format.c
         :language: c
         :lines: 32-36,55-69,76-79
         :linenos:
         :dedent: 4
                  
   .. group-tab:: python

      .. literalinclude:: ../../examples/python/05-set-format.py
         :language: python
         :lines: 38-42, 49-64
         :linenos:
         :dedent: 4
                  
This code can be found in the example `04-set-format`.

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

Working code can be found in the example `05-live-stream`.

An alternative to simple trial-and-error setups is the use of the program ``gst-launch-1.0``.
This program enables the creation of pipelines on the command line, allowing for quick setups. 


Receiving Images
----------------

The easiest approach is to use an appsink.
The appsink element will call a function for each new image it receives.

To enable image retrieval, the following steps need to be taken.

.. tabs::

   .. group-tab:: c

      .. literalinclude:: ../../examples/c/07-appsink.c
         :language: c
         :lines:  102-106, 114-123
         :linenos:
         :dedent: 4
                  
   .. group-tab:: python

      .. literalinclude:: ../../examples/python/07-appsink.py
         :language: python
         :lines: 94-97, 108-116
         :linenos: 
         :dedent: 4
                  
The image `sample` that is given to the function contains the image, video caps and other additional information that maybe required for image processing.


.. tabs::

   .. group-tab:: c

      .. literalinclude:: ../../examples/c/07-appsink.c
         :language: c
         :lines: 32-45, 51, 90-95
         :linenos:
                  
   .. group-tab:: python

      .. literalinclude:: ../../examples/python/07-appsink.py
         :language: python
         :lines: 37-51, 86
         :linenos:

This code can be found in the example `07-appsink`.


Properties
==========

The camera offers multiple properties to assist with image acquisition.
Depending on the device at hand, these properties include functions
such as software trigger, exposure, and complete auto adjustment algorithms.

Get/List Properties
-------------------

The responsible function is `tcam_prop_get_tcam_property_names`.

For an overview of available properties, type the following into a terminal:

.. code-block:: sh

   tcam-ctrl -p <SERIAL>

.. tabs::

   .. group-tab:: c

      .. literalinclude:: ../../examples/c/01-list-properties.c
         :language: c
         :lines: 33-35, 45-78, 140-143
         :linenos:
         :dedent: 4
                     
   .. group-tab:: python

      .. literalinclude:: ../../examples/python/01-list-properties.py
         :language: python
         :lines: 37-40, 44-56 
         :linenos:
         :dedent: 4

                  
This code can be found in the example `01-list-properties`.

  
Set Property
------------

The responsible function is `tcam_prop_set_tcam_property`.

.. tabs::

   .. group-tab:: c

      .. literalinclude:: ../../examples/c/02-set-properties.c
         :language: c
         :lines: 86-88, 100-101, 114-132
         :linenos:
         :dedent: 4
                  
   .. group-tab:: python

      .. literalinclude:: ../../examples/python/02-set-properties.py
         :language: python
         :lines: 74-75, 80-82, 88-91
         :linenos:
         :dedent: 4
                  
This code can be found in the example `02-set-properties`.

Where to Go from Here
=====================

Take a look at our :any:`reference`, the :any:`GStreamer documentation<reading_gstreamer>` or :any:`ask us a question<contact>`.

For extended examples (including OpenCV, ROS and GUI frameworks), please have a look at our :ref:`extended examples<examples_further>`.
