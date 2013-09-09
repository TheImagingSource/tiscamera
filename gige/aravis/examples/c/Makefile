
LIBS:=$(shell pkg-config gstreamer-0.10 --libs)
LIBS+=$(shell pkg-config gtk+-2.0 --libs)
LIBS+=$(shell pkg-config gstreamer-interfaces-0.10 --libs)
LIBS+= -lm
LIBS+=-laravis-0.4

CFLAGS:=$(shell pkg-config gstreamer-0.10 --cflags)
CFLAGS+=$(shell pkg-config gtk+-2.0 --cflags)
CFLAGS+=$(shell pkg-config gstreamer-interfaces-0.10 --cflags)

#
# adjust these two to point to aravis
#
L_PATH:=-L aravis/src/.libs
CFLAGS+=-I aravis/src

.PHONY: all clean

all: gstexample

gstexample: gstexample.c
	$(CC) -g -DGST_DISABLE_DEPRECATED -O0 $(CFLAGS) $^ -o $@ $(L_PATH) $(LIBS)

clean:
	$(RM) *.o
	$(RM) gstexample

