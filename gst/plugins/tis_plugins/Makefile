#
# Makefile for The Imaging Source gstreamer elements
#
# Copyright (C) 2013 The Imaging Source GmbH; Edgar Thier <edgarthier@gmail.com>
#

# necessary gstreamer packages
pkg=gstreamer-base-0.10 gstreamer-0.10


LDFLAGS+=`pkg-config --libs $(pkg)`
LDFLAGS+=-laravis-0.4
# LDFLAGS+=-Ladd_aravis_lib_path_here

CFLAGS+=`pkg-config --cflags $(pkg)`
# CFLAGS+=-Iadd_aravis_header_path_here


PLUGINS=libgsttiswhitebalance.so \
		libgsttisvideobufferfilter.so \
		libgsttiscamerasrc.so \
		libgsttis_auto_exposure.so \
		libgsttiscolorize.so

all: $(PLUGINS)

%.o: %.c
	$(CC) -O0 -g -Wall -fPIC $(CPPFLAGS) $(CFLAGS) -c -o $(patsubst %.c,%.o,$<) $< $(L_PATH) $(LDFLAGS)

lib%.so: %.o image_sampling.o bayer.o
	$(CC) -O0 -g -shared -o $@ $^ $(L_PATH) $(LDFLAGS)


clean:
	rm -f *.o *.so *~ TAGS

TAGS:
	etags *.[ch]

install: all
	install -D libgsttiswhitebalance.so $(HOME)/.gstreamer-0.10/plugins 
	install -D libgsttisvideobufferfilter.so $(HOME)/.gstreamer-0.10/plugins 
	install -D libgsttiscamerasrc.so $(HOME)/.gstreamer-0.10/plugins 
