CFLAGS	:= -W -Wall
CC	:= gcc

all: euvccam-fw

euvccam-fw: euvccam-fw.c
	$(CC) -O0 -g -o $@ $< `pkg-config --cflags --libs libusb glib-2.0`

clean:
	rm -f *.o
	rm -f euvccam-fw

distclean: clean
	rm -f TAGS
	rm -f backups/*
	rm -f *.cache
	rm -f *~
