
HAVE_ARAVIS=1

APP=library_test

LDFLAGS+= \
$(shell pkg-config --libs libudev) 
# $(shell pkg-config --libs gstreamer-0.10) \


DEBUG+= -g3 -O0 -fvar-tracking -fvar-tracking-assignments
TEST= -Wunreachable-code -Wconversion -Wswitch-enum -Wswitch-default

INCLUDES=	\
	$(shell pkg-config --cflags gstreamer-0.10) \
	$(shell pkg-config --cflags udev) \
	-I. 


CFLAGS+= -std=c99 -fpic -w -Wall -Wextra -pedantic -Wundef -Wshadow \
	$(DEBUG) $(TEST) \
	$(INCLUDES) \
	-DHAVE_UDEV


ifeq ($(HAVE_ARAVIS), 1)
# LDFLAGS+= -L/home/edt/work/aravis/src/.libs -laravis-0.4
LDFLAGS+= $(shell pkg-config --libs aravis-0.4)
CFLAGS+= -DHAVE_ARAVIS $(shell pkg-config --cflags aravis-0.4)
OBJECT_FILES+= AravisDevice.o
# OBJECT_FILES+= 			camera_gige_interface.o \

endif


# FILTER_DIR="./filter"

# FILTER:
# 	$(MAKE) -C $(FILTER_DIR) all




# $(shell pkg-config --cflags opencv) \

# -DHAVE_ARAVIS 

CPPFLAGS+= -std=c++11 -flto $(CFLAGS)

LINK_OPTIONS=-Wl,--gc-sections -ffunction-sections -fdata-sections -ldl


LIB= libtis_imaging.a libtis_imaging.so

OBJECT_FILES= \
 Error.o \
 CaptureDevice.o \
 DeviceInterface.o \
 Grabber.o \
 V4l2Device.o \
 VideoFormat.o \
 VideoFormatDescription.o \
 device_discovery.o \
 tis_logging.o \
 tis_utils.o \
 Property.o \
 Properties.o \
 PropertyGeneration.o \
 FilterLoader.o \
 MemoryBuffer.o \
 ImageSink.o \
 PipelineManager.o \
 ImageSource.o

ifeq ($(HAVE_ARAVIS), 1)
OBJECT_FILES += AravisDevice.o 
endif

# tis_v4l2.o \
# MemoryBuffer.o \


.PHONY: all clean

all: $(LIB)  $(APP)

.c.o:
	$(CXX) $(CFLAGS) -c -o $@ $^
# $(LDFLAGS)


.o: 
	$(CXX) $(CPPFLAGS) $(CFLAGS) -c -o $@ $^  $(LINK_OPTIONS) $(LDFLAGS)

libtis_imaging.a: $(OBJECT_FILES)
	ar rvs $@ $^

libtis_imaging.so: $(OBJECT_FILES)
	$(CXX) -shared -fpic $(CPPFLAGS)  $^ -o $@ $(LINK_OPTIONS) $(LDFLAGS)

$(APP): $(LIB) main.o
	$(CXX) main.o  -o $@  $(CPPFLAGS)  -L. -ltis_imaging 

clean:
	$(RM) $(APP)
	$(RM) $(LIB)
	$(RM) *.o

