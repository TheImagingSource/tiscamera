#
# Copyright 2014 The Imaging Source Europe GmbH
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

CFLAGS+=$(shell pkg-config --cflags libzip)
LDFLAGS+=$(shell pkg-config --libs libzip)

all: firmware-update

firmware-update: firmware-update.o UsbSession.o UsbHandler.o FileHandling.o UsbCamera.o Usb3Camera.o Usb2Camera.o
	$(CXX) -std=c++11 -O0 -g $^ -o $@ -lusb-1.0 $(LDFLAGS)

%.o: %.cpp
	$(CXX) -std=c++11 -Wall -g -O0 $(CPPFLAGS) $(CFLAGS) -c $^ -o $@

.PHONY: clean all

clean:
	$(RM) *.o
	$(RM) *~
	$(RM) firmware-update

