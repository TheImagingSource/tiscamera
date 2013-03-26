all: libgsttiswhitebalance.so libgsttisvideobufferfilter.so

pkg=gstreamer-base-0.10

lib%.so: %.c
	$(CC) -Wall -fPIC $(CPPFLAGS) `pkg-config --cflags gstreamer-0.10 $(pkg)` -c -o $(patsubst %.c,%.o,$<) $<
	$(CC) -shared -o $@ $(patsubst %.c,%.o,$<) `pkg-config --libs gstreamer-0.10 $(pkg)`


clean:
	rm -f *.o *.so *~


install: all
	install -D libgsttiswhitebalance.so $(HOME)/.gstreamer-0.10/plugins
	install -D libgsttisvideobufferfilter.so $(HOME)/.gstreamer-0.10/plugins