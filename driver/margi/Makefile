O_TARGET := margilib.o

EXTRA_CFLAGS = -MD -I.. -I ../../include -I . -DUSE_OSD

obj-y           :=
obj-m           :=
obj-n           :=
obj-            :=

list-multi      :=      margi_cs.o

margi_cs-objs   :=      margi.o cardbase.o i2c.o dram.o osd.o audio.o \
                        video.o streams.o decoder.o spu.o crc.o ringbuffy.o \
                        cvdv.o

export-objs     :=

obj-m += margi_cs.o

include $(TOPDIR)/Rules.make

margi_cs.o: $(margi_cs-objs)
	$(LD) $(LD_RFLAG) -r -o $@ $(margi_cs-objs)

clean:
	rm -f *.o *.o.flags *.d
