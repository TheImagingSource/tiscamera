#
# Makefile
# Compile cli version and/or generate gui makefiles
#
# Copyright (C) 2013 The Imaging Source GmbH; Edgar Thier <edgarthier@gmail.com>
#

### Commentary:

# make all  to build both programs
# make cli  to only build the commandline tool
# make gui  to only build the graphical interface
#
# make clean      cleans up object files
# make distclean  deletes every generated file
# make realclean  cleans up object files + executables

CCFLAGS= -g -Wall -W -std=c++0x -O2 -Wextra
LDFLAGS= -pthread -lzip -ltinyxml

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
