PLUGINS=libgsttiswhitebalance.so libgsttisvideobufferfilter.so libgsttiscamerasrc.so

all: $(PLUGINS)

pkg=gstreamer-base-0.10

lib%.so: %.c
	$(CC) -O2 -Wall -fPIC $(CPPFLAGS) `pkg-config --cflags gstreamer-0.10 $(pkg)` -c -o $(patsubst %.c,%.o,$<) $<
	$(CC) -O2 -shared -o $@ $(patsubst %.c,%.o,$<) `pkg-config --libs gstreamer-0.10 $(pkg)` 


clean:
	rm -f *.o *.so *~ TAGS

TAGS:
	etags *.[ch]

install: all
	install -D libgsttiswhitebalance.so $(HOME)/.gstreamer-0.10/plugins 
	install -D libgsttisvideobufferfilter.so $(HOME)/.gstreamer-0.10/plugins 
	install -D libgsttiscamerasrc.so $(HOME)/.gstreamer-0.10/plugins 
