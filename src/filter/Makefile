

LDFLAGS += -L/home/edt/work/tis_imaging -L/home/edt/lib -ltis_auto_alg -ltis_imaging
INCLUDE += -I/home/edt/work/drvbuildtools/trunk/lib/dutils_image/

FILTER=\
	tisfilter_autopass.so \
	tisfilter_bayer2rgb.so

CXXFLAGS+= -shared -fPIC -I.. -I. -std=c++11 -g3 -O0 $(INCLUDE)


.PHONY: all F clean


F: $(FILTER)

all: F


tisfilter_autopass.so: AutoPassFilter.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

tisfilter_bayer2rgb.so: BayerRgbFilter.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) $(FILTER)
	$(RM) *.o


