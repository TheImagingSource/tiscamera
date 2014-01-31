#
# Copyright 2013 The Imaging Source Europe GmbH
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

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

