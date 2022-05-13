
.. _examples:

########
Examples
########

The following is a list of examples and their purpose.
These examples exist in C and Python3.

The standard installation path for examples is `/usr/share/theimagingsource/tiscamera/examples`.

It may be neccessary to install additional development packages when using pre-compiled tiscamera packages.
See :ref:`dependencies`.

.. _examples_building:

Build Instructions
==================

Instructions to build applications using tiscamera.

.. raw:: html

   <details>
   <summary><a>Show sample code</a></summary>

.. tabs::

   .. group-tab:: c

      .. literalinclude:: ../../examples/c/Makefile
         :language: Makefile
         :linenos:
         :lines: 16-

   .. group-tab:: python
                       
      Automatically handled by gobject introspection.

      For custom installations set `GI_TYPELIB_PATH` to where the file `Tcam-1.0.typelib` is installed.
      

.. _examples_list_devices:

00 - list-devices
=================

Shows what cameras there are and how to identify them.

.. raw:: html

   <details>
   <summary><a>Show sample code</a></summary>

.. tabs::

   .. group-tab:: c

      .. literalinclude:: ../../examples/c/00-list-devices.c
         :language: c
         :linenos:
         :lines: 16-

   .. group-tab:: python
                          
      .. literalinclude:: ../../examples/python/00-list-devices.py
         :language: python
         :linenos:
         :lines: 1, 16-
            
.. raw:: html
         
   </details>

.. _examples_list_properties:
                    
01 - list-properties
====================

Shows the properties of a camera and their settings (range, current value, etc.).

.. raw:: html

   <details>
   <summary><a>Show sample code</a></summary>
   
.. tabs::

   .. group-tab:: c

      .. literalinclude:: ../../examples/c/01-list-properties.c
         :language: c
         :linenos:
         :lines: 16-

   .. group-tab:: python
                          
      .. literalinclude:: ../../examples/python/01-list-properties.py
         :language: python
         :linenos:
         :lines: 1, 16-

.. raw:: html
         
   </details>

.. _examples_set_properties:
        
02 - set-properties
===================
Shows how to set a specific property.

.. raw:: html

   <details>
   <summary><a>Show sample code</a></summary>
   
.. tabs::

   .. group-tab:: c

      .. literalinclude:: ../../examples/c/02-set-properties.c
         :language: c
         :linenos:
         :lines: 16-

   .. group-tab:: python
                          
      .. literalinclude:: ../../examples/python/02-set-properties.py
         :language: python
         :linenos:
         :lines: 1, 16-

.. raw:: html
         
   </details>

.. _examples_live_stream:
        
03 - live-stream
================
Delivers live-image stream from the camera.

.. raw:: html

   <details>
   <summary><a>Show sample code</a></summary>
   
.. tabs::

   .. group-tab:: c

      .. literalinclude:: ../../examples/c/03-live-stream.c
         :language: c
         :linenos:
         :lines: 16-

   .. group-tab:: python
                          
      .. literalinclude:: ../../examples/python/03-live-stream.py
         :language: python
         :linenos:
         :lines: 1, 16-

.. raw:: html
         
   </details>

.. _examples_list_format:
        
04 - list-format
================
Lists what formats the camera offers.

.. raw:: html

   <details>
   <summary><a>Show sample code</a></summary>
   
.. tabs::

   .. group-tab:: c

      .. literalinclude:: ../../examples/c/04-list-formats.c
         :language: c
         :linenos:
         :lines: 16-

   .. group-tab:: python
                          
      .. literalinclude:: ../../examples/python/04-list-formats.py
         :language: python
         :linenos:
         :lines: 1, 16-

.. raw:: html
         
   </details>

.. _examples_set_format:

05 - set format
===============
Sets the camera to a specific format.

.. raw:: html

   <details>
   <summary><a>Show sample code</a></summary>
   
.. tabs::

   .. group-tab:: c

      .. literalinclude:: ../../examples/c/05-set-format.c
         :language: c
         :linenos:
         :lines: 16-

   .. group-tab:: python
                          
      .. literalinclude:: ../../examples/python/05-set-format.py
         :language: python
         :linenos:
         :lines: 1, 16-

.. raw:: html
         
   </details>

.. _examples_softwaretrigger:
        
06 - softwaretrigger
====================
Triggers single images - instead of a continuous image stream.

.. raw:: html

   <details>
   <summary><a>Show sample code</a></summary>
   
.. tabs::

   .. group-tab:: c

      .. literalinclude:: ../../examples/c/06-softwaretrigger.c
         :language: c
         :linenos:
         :lines: 16-

   .. group-tab:: python
                          
      .. literalinclude:: ../../examples/python/06-softwaretrigger.py
         :language: python
         :linenos:
         :lines: 1, 16-

.. raw:: html
         
   </details>

.. _examples_appsink:
        
07 - appsink
============
Receives images in an application instead of just showing them.

.. raw:: html

   <details>
   <summary><a>Show sample code</a></summary>
   
.. tabs::

   .. group-tab:: c

      .. literalinclude:: ../../examples/c/07-appsink.c
         :language: c
         :linenos:
         :lines: 16-

   .. group-tab:: python
                          
      .. literalinclude:: ../../examples/python/07-appsink.py
         :language: python
         :linenos:
         :lines: 1, 16-

.. raw:: html
         
   </details>

.. _examples_save_stream:
        
08 - save-stream
================
Stores a stream in a file.

.. raw:: html

   <details>
   <summary><a>Show sample code</a></summary>
   
.. tabs::

   .. group-tab:: c

      .. literalinclude:: ../../examples/c/08-save-stream.c
         :language: c
         :linenos:
         :lines: 16-

   .. group-tab:: python
                          
      .. literalinclude:: ../../examples/python/08-save-stream.py
         :language: python
         :linenos:
         :lines: 1, 16-

.. raw:: html
         
   </details>

.. _examples_device_lost:
        
09 - device-lost
================
Receives device-lost and other messages and react to them.

.. raw:: html

   <details>
   <summary><a>Show sample code</a></summary>
   
.. tabs::

   .. group-tab:: c

      .. literalinclude:: ../../examples/c/09-device-lost.c
         :language: c
         :linenos:
         :lines: 16-

   .. group-tab:: python
                          
      .. literalinclude:: ../../examples/python/09-device-lost.py
         :language: python
         :linenos:
         :lines: 1, 16-

.. raw:: html
         
   </details>

.. _examples_metadata:
        
10 - metadata
=============
Read meta information like is-damaged, camera capture time, etc.

.. raw:: html

   <details>
   <summary><a>Show sample code</a></summary>
   
.. tabs::

   .. group-tab:: c

      .. literalinclude:: ../../examples/c/10-metadata.c
         :language: c
         :linenos:
         :lines: 16-

   .. group-tab:: python
                          
      .. literalinclude:: ../../examples/python/10-metadata.py
         :language: python
         :linenos:
         :lines: 1, 16-

.. raw:: html
         
   </details>

.. _examples_json_state:
        
11 - json-state
===============
Save and load JSON device state.

.. raw:: html

   <details>
   <summary><a>Show sample code</a></summary>
   
.. tabs::

   .. group-tab:: c

      .. literalinclude:: ../../examples/c/11-json-state.c
         :language: c
         :linenos:
         :lines: 16-

   .. group-tab:: python
                          
      .. literalinclude:: ../../examples/python/11-json-state.py
         :language: python
         :linenos:
         :lines: 1, 16-

.. raw:: html
         
   </details>

.. _examples_tcam_properties:
        
12 - tcam-properties
====================
Save and load properties via GstStructure.

.. raw:: html

   <details>
   <summary><a>Show sample code</a></summary>
   
.. tabs::

   .. group-tab:: c

      .. literalinclude:: ../../examples/c/12-tcam-properties.c
         :language: c
         :linenos:
         :lines: 16-

   .. group-tab:: python
                          
      .. literalinclude:: ../../examples/python/12-tcam-properties.py
         :language: python
         :linenos:
         :lines: 1, 16-

.. raw:: html
         
   </details>


.. _examples_gstquery:

13 - GstQuery
=============

Shows how to use GstQuery for GstCaps verification.

.. raw:: html

   <details>
   <summary><a>Show sample code</a></summary>

.. tabs::

   .. group-tab:: c

      .. literalinclude:: ../../examples/c/13-gstquery.c
         :language: c
         :linenos:
         :lines: 16-

   .. group-tab:: python

      .. literalinclude:: ../../examples/python/13-gstquery.py
         :language: python
         :linenos:
         :lines: 1, 16-

.. raw:: html

   </details>



.. _examples_further:

Further Examples
================

For extended examples, look through the examples repository.

https://github.com/TheImagingSource/Linux-tiscamera-Programming-Samples

It contains examples on how to interact with OpenCV, ROS, GUI toolkits and much more.
