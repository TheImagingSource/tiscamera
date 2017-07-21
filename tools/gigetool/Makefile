all: gigewrapper.so

CFLAGS=-g -O0

gigewrapper.so: gige_wrapper.cpp gige_wrapper.h
	$(CXX) $(CFLAGS) -shared -Wall -Werror -fpic -o $@ $< -I../../src -I../../src/tcam-network -ltcam-network `pkg-config --libs --cflags tcam gobject-introspection-1.0`
