.. _gobject:

#####################
GObject Introspection
#####################

To allow camera interaction from many different environments we offer a gobject
introspection interface. This interface, that can be accessed through our
gstreamer modules, allows full control of all device properties.

Accessing the interface
=======================

To access the tcamprop interface two things are required:
- gstreamer-1.0
- gobject introspection

To see examples of how the interface is used take a look at our examples
directory.

Any tcam* gstreamer element can be used as an TcamProp instance.

Only tcamsrc and tcambin will provide device information.

Tcambin will internally fuse the information of its internal elements and return
the combined properties of these elements.

**!!! Caution !!!**

When changing the currently used format a reiteration of the properties is
advised. The tcambin will delete internal elements should the not be required
for the current format. This typically happens when switching between bayer and
other formats.

Adding additional directories to the search path
================================================

To index additional directories set the environment variable `GI_TYPELIB_PATH`.

.. code-block:: sh

    export GI_TYPELIB_PATH=/home/user/tiscamera/build/src/gobject/
