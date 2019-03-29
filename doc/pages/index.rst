
Welcome to tiscamera's documentation!
=====================================

This is the user documentation for The Imaging Source Linux libraries.

If assistance is required at any point :any:`contact our support <contact>`. 

.. todolist::

Philosophy
----------

Tiscamera is an open source project published under the Apache 2.0 license.
This means that user can integrate the project or parts of it into their projects
without hassle. Modifications or maintenance can be done without assistance of The Imaging Source.
This allows for extended product life cycles.

The fact that the entire software stack is open allows the usage of arbitrary hardware.
The only requirement is a Linux based operating system.

Included Modules
----------------

- gobject-introspection

  Integration into many language environments is done vie gobject-introspection.
  
- gstreamer modules

  Image streaming is done with gstreamer. 

- auto algorithms

  Software based property adjustments for cameras that do not have internal auto-algorithms.
  
- tools

  Tools to make camera interactions easier.
  Also includes things like firmware updates, GigE camera configuration, etc.

Overview
--------

.. toctree::
   :maxdepth: 3

   tutorial.rst
   reference.rst
   tools.rst
   examples.rst
   troubleshooting.rst
   advanced-topics.rst
   further-reading.rst
   contact.rst

Indices and tables
==================

* :ref:`genindex`
  
..
   * :ref:`modindex`
     
* :ref:`search`
