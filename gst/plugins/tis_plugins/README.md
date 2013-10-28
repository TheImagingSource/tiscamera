
TIS Gstreamer Plugins
=====================

What is it?
-----------

This is a collection of gstreamer elements that offer additional functionality
for TIS cameras.

Dependencies
------------

To successfully compile the elements you need the following libraries/header.

libglib2.0
libudev
libgstreamer0.10
gstreamer0.10-plugins-base

As an optional dependency we use the aravis library to support network cameras.

How to build
------------

call bootstrap.sh

configure

make

if you want to enable aravis support, call configure with:
--enable-aravis



The makefile offers the followng options:

all: build everything

tags: create etags

clean: delete temporary files, object files and tags

realclean: calls clean and deletes the creates libraries

distclean: delete everything so that folder returns to originally distributed state

