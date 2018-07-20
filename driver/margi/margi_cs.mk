#
# Makefile for margi_cs
# Marcus Metzler <mocm@metzlerbros.de>
#

include ../config.mk

CC = $(KCC) $(AFLAGS) $(KFLAGS)

CFLAGS += -g -O2 -Wall -Wstrict-prototypes -pipe -Wall -D__DVB_PACK__ -DUSE_OSD -DNOINT -DDVB  -DUSE_ZV 

CPPFLAGS += $(PCDEBUG) -D__KERNEL__ -DMODULE -DMODVERSIONS -I../include \
	   -I$(LINUX)/include -I$(LINUX) -Iinclude 

CC_MODULE = $(CC) -c $(CFLAGS) $(CPPFLAGS)

ETC = $(PREFIX)/etc/pcmcia
MANDIR = $(PREFIX)/usr/man
MX_OBJS =  dmxdev.o dvb_demux.o dvbdev.o dvb_filter.o

all: margi_cs.o $(MX_OBJS)

install-modules: $(MODULES) $(MX_OBJS)
	mkdir -p $(PREFIX)/$(MODDIR)/pcmcia
	cp $(MODULES) $(PREFIX)/$(MODDIR)/pcmcia
	su -c "mkdir -p $(MODDIR)/misc; cp -v $(MX_OBJS) $(MX_OBJS) $(MODDIR)/misc"

install-clients:
	for f in $(CLIENTS) ; do				\
	    [ -r $$f.conf ] && cp $$f.conf $(ETC)/$$f.conf ;	\
	    cmp -s $$f $(ETC)/$$f && continue ;			\
	    [ -r $(ETC)/$$f ] && mv $(ETC)/$$f $(ETC)/$$f.O ;	\
	    cp $$f $(ETC)/$$f ;					\
	    OPTS=$(ETC)/$$f.opts ;				\
	    test -r $$OPTS || cp $$f.opts $$OPTS ;		\
	done
	cp  cvdvext.h cvdvtypes.h /usr/include/linux
	mkdir -p /usr/include/ost/
	cp  include/ost/*.h /usr/include/ost		
	rm -rf /dev/ost
	./makedev.napi
	depmod -a

install-man4: $(MAN4)
	mkdir -p $(MANDIR)/man4
	cp $(MAN4) $(MANDIR)/man4


MMODULES =   margi.o cvdv.o cardbase.o i2c.o dram.o osd.o audio.o video.o streams.o decoder.o spu.o crc.o ringbuffy.o

margi_cs.o: $(MMODULES)
	$(LD) -r -o margi_cs.o $(MMODULES)
	chmod -x margi_cs.o


margi.o: margi.h cardbase.h l64014.h l64021.h i2c.h decoder.h dram.h\
	 video.h cvdv.h margi.c
	$(CC_MODULE) margi.c

cvdv.o: cvdv.c cvdv.h cardbase.h cvdvtypes.h l64021.h l64014.h dram.h\
	 osd.h audio.h video.h streams.h decoder.h spu.h \
	crc.h
	$(CC_MODULE) cvdv.c

cardbase.o: cardbase.c cardbase.h cvdvtypes.h
	$(CC_MODULE) cardbase.c

i2c.o: i2c.c i2c.h cardbase.h cvdvtypes.h l64014.h
	$(CC_MODULE) i2c.c

dram.o: dram.c dram.h cardbase.h cvdvtypes.h l64021.h l64014.h
	$(CC_MODULE) dram.c

osd.o: osd.c osd.h cardbase.h cvdvtypes.h dram.h l64021.h l64014.h
	$(CC_MODULE) osd.c

audio.o: audio.c audio.h cardbase.h cvdvtypes.h l64021.h l64014.h
	$(CC_MODULE) audio.c

video.o: video.c video.h cardbase.h cvdvtypes.h dram.h l64021.h l64014.h 
	$(CC_MODULE) video.c

streams.o: streams.c streams.h cardbase.h cvdvtypes.h dram.h l64021.h l64014.h \
video.h dram.h audio.h
	$(CC_MODULE) streams.c

decoder.o: decoder.c decoder.h cardbase.h cvdvtypes.h dram.h l64021.h l64014.h \
video.h dram.h audio.h streams.h i2c.h osd.h dram.h
	$(CC_MODULE) decoder.c

spu.o: spu.c spu.h cardbase.h cvdvtypes.h l64021.h l64014.h
	$(CC_MODULE) spu.c

crc.o: crc.c crc.h
	$(CC_MODULE) crc.c

ringbuffy.o: ringbuffy.h ringbuffy.c
	$(CC_MODULE) ringbuffy.c

dvb_filter.o: dvb_filter.h dvb_filter.c
	$(CC_MODULE) -include $(LINUX)/include/linux/modversions.h dvb_filter.c

dmxdev.o: dmxdev.h dvb_demux.h
	$(CC_MODULE) -include $(LINUX)/include/linux/modversions.h dmxdev.c

dvb_demux.o: dvb_demux.h dmxdev.h dvbdev.h
	$(CC_MODULE) -include $(LINUX)/include/linux/modversions.h dvb_demux.c

dvbdev.o: dvbdev.h
	$(CC_MODULE) -include $(LINUX)/include/linux/modversions.h -DEXPORT_SYMTAB -c dvbdev.c

clean:
	rm -f core core.* *.o .*.o *.s *.a *~ .depend .depfiles/*.d
