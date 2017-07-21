all: gigewrapper.so

CFLAGS=-g -O0

gigewrapper.so: gige_wrapper.cpp gige_wrapper.h
	$(CXX) $(CFLAGS) -shared -Wall -Werror -std=c++0x -fpic -o $@ $< -I../../src -I../../src/tcam-network -ltcam-network `pkg-config --libs --cflags tcam gobject-introspection-1.0`

clean:
	rm -f gigewrapper.so
