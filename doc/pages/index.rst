
Welcome to tiscamera's documentation!
=====================================

This is the user documentation for The Imaging Source Linux libraries.

If assistance is required at any point, please :any:`contact our support <contact>`. 

.. todolist::

Philosophy
----------

Tiscamera is an open source project published under the Apache 2.0 license.
This allows user to easily integrate the project (or parts of it) into their own applications.
Modifications or maintenance can be carried out without assistance of The Imaging Source.
This allows for extended product life cycles.

The entire software stack is open allows the usage of arbitrary hardware.
The only requirement is a Linux based operating system.

Included Modules
----------------

- GObject-Introspection

  Integration into various language environments is achieved via GObject-Introspection.
  
- GStreamer modules

  Image streaming is performed with GStreamer. 

- auto algorithms

  Software based property adjustments for cameras not having internal auto algorithms.
  
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
