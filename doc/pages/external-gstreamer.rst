#######################
Other Gstreamer Modules
#######################

This is a collection of gstreamer modules that are not provided by The Imaging Source


clockoverlay
============

Allows overlays of timestamps or duration's.

capssetter
==========

Capssetter allows to overwrite existing caps or to add information to existing ones.

tee
===

Tee allows the sending of buffers to multiple modules at once.
A typical example would be to send a videostream to a window for live preview
and to additionally process it in the background.

After adding tee always use a queue to ensure that the following pipeline runs
in its own thread.

For further info please consult the
`gstreamer documentation <https://gstreamer.freedesktop.org/data/doc/gstreamer/head/manual/html/section-threads-uses.html>`_.

valve
=====

Let image buffer and bus messages pass only while the valve is open

videorate
=========

Change the framerate by duplicating or dropping frames.

videoscale
==========

Change the resolution of an image stream by scaling up or down.

appsink
=======

Receive image buffer in an application and use them directly.
For examples how this may work, look at our examples folder.

multifilesink
=============

Automatically creates sequential files.
Certain actions are not possible with a multifilesink.
Saving an avi file will not work as expected.
Use a filesink or other sink instead.

filesink
========

Save a stream or stream buffer into a file.
