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

### Commentary:

# make all  to build both programs
# make cli  to only build the commandline tool
# make gui  to only build the graphical interface
#
# make clean      cleans up object files
# make distclean  deletes every generated file
# make realclean  cleans up object files + executables

CCFLAGS= -g -Wall -W -std=c++0x -O2 -Wextra $(shell pkg-config --cflags libzip) $(shell pkg-config --cflags tinyxml)
LDFLAGS= -pthread $(shell pkg-config --libs libzip) $(shell pkg-config --libs tinyxml)

# Gui related variables
GUI_DIR=gui
GUI_PROJECT_FILE=camera-ip-conf.pro


OBJ= main.o ConsoleManager.o CameraDiscovery.o Camera.o Socket.o NetworkInterface.o utils.o FirmwareUpgrade.o

# general build section
.PHONY: all gui cli
all: cli gui

cli: camera-ip-conf-cli

gui:
	cd $(GUI_DIR); \
	qmake $(GUI_PROJECT_FILE) -o Makefile; \
	$(MAKE)

camera-ip-conf-cli: $(OBJ)
	$(CXX) $(CCFLAGS) $^ -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CCFLAGS) -c $^ -o $@ $(LDFLAGS) $(INC)

# clean-up section
.PHONY: clean clean-gui clean-cli realclean distclean

clean-cli:
	$(RM) *.o

clean-gui:
	cd $(GUI_DIR); make clean

clean: clean-cli clean-gui

realclean: clean
	$(RM) camera-ip-conf-cli
	$(RM) camera-ip-conf

distclean: realclean
	cd $(GUI_DIR); make distclean
