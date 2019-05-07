############
Installation
############

The installation of this project defaults to /usr.
Because the search path for these tools requires special handling,
however, some parts of the project will install to paths other than /usr.
These include:
systemd - /
udev -
gobject-introspection -

These paths can be changed by explicitly setting them during the cmake configuration step.

=========
Locations
=========

GStreamer
=========

The GStreamer installation folder defaults to the location given by GSTREAMER_1.0_PLUGINSDIR
in the pkgconfig gstreamer description.

To manually add different folders to the search path, set the environment variable GST_PLUGIN_PATH_1_0

Private Libraries
=================

The tiscamera project includes multiple shared libraries that are not meant for general usage.
They will be installed into a subfolder named tcam-MAJOR_VERSION.
These libs should not be linked as their API might change without notice.
