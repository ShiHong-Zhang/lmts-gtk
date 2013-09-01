
CC = gcc
vpath %.c $(PWD)
vpath %.o $(PWD)

CFLAGS = -Wall -g			\
	-DG_DISABLE_DEPRECATED 	 	\
	-DGDK_DISABLE_DEPRECATED 	\
	-DGDK_PIXBUF_DISABLE_DEPRECATED \
	-DGTK_DISABLE_DEPRECATED		\
	-I/usr/include/lua5.1			\
	`pkg-config gtk+-2.0 --cflags`

LFLAGS = -Wall -g			\
	-DG_DISABLE_DEPRECATED 	 	\
	-DGDK_DISABLE_DEPRECATED 	\
	-DGDK_PIXBUF_DISABLE_DEPRECATED \
	-DGTK_DISABLE_DEPRECATED		\
	-I/usr/include/lua5.1			\
	-L/usr/lib/lua/5.1 -llua5.1	\
	`pkg-config gtk+-2.0 --cflags --libs gthread-2.0`

object = audio.o burnmac.o ckntp.o ckrom.o cpu_memcap.o \
		 keyboard.o lcdwave.o scanmbsn.o sdcard.o upload.o \
		 vgaout.o wifi.o battery.o ckec.o ckpmon.o cmos.o \
		 fanstem.o lan.o ntp.o scanpsn.o touchpad.o usb.o webcam.o

all: $(object)

$(object): %.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@ 

.PHONY: clean
clean: 
	rm -f *.o
