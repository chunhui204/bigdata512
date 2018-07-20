##
#  Configs
#

CARDS = av7110
#CARDS += margi
#CARDS += em8300
CARDS += b2c2

#export USE_DVB_DSP=1
#dvb_dsp-ins = insmod dvb_dsp.o;
#dvb_dsp-rmm = rmmod dvb_dsp;

#  END OF CONFIGS

##
#  Globals
#

export DVB_PACK = 1

export KERNEL_VERSION := $(shell uname -r)
export KERNEL_LOCATION = /lib/modules/$(KERNEL_VERSION)/build

export CONFIG_SHELL := $(shell if [ -x "$$BASH" ]; then echo $$BASH; \
	  else if [ -x /bin/bash ]; then echo /bin/bash; \
	  else echo sh; fi ; fi)

export TOPDIR=$(KERNEL_LOCATION)

DVB_MOD_SUBDIR=misc
DVB_MOD_DIR=/lib/modules/$(KERNEL_VERSION)/$(DVB_MOD_SUBDIR)


##
#  Targets
#

here:
	DIR=`pwd`; (cd $(KERNEL_LOCATION); $(MAKE) SUBDIRS=$$DIR modules)
	@echo

install:
	install -v -m 0755 -d $(DESTDIR)$(DVB_MOD_DIR)
	install -m 0644 dvb-core.o $(DESTDIR)$(DVB_MOD_DIR)
	install -m 0644 av7110/dvb-ttpci.o $(DESTDIR)$(DVB_MOD_DIR)
	find frontends/ -name "*.o" -exec install -v -m 0644 {} $(DESTDIR)$(DVB_MOD_DIR) \;
	depmod -a

reload: rmmod insmod


insmod: here
	sync
	(						\
	insmod videodev;				\
	$(dvb_dsp-ins)          			\
	insmod dvb-core.o dvb_shutdown_timeout=0; 	\
	make -C frontends insmod; 			\
	$(foreach card, $(CARDS), make -C $(card) insmod;) \
	)
	sync

rmmod:
	sync
	(	\
	$(foreach card, $(CARDS), make -C $(card) rmmod;) \
	make -C frontends rmmod; \
	rmmod dvb-core;          \
	$(dvb_dsp-rmm)          \
	rmmod videodev;		 \
	)
	sync

clean:
	-rm -f $(M_OBJS) $(MX_OBJS) *.o *.d .*.o.flags *~
	$(MAKE) -C frontends clean
	$(foreach card, $(CARDS), $(MAKE) -C $(card) clean;)


##
#  Rules
#

export-objs = dvb_ksyms.o compat.o
obj-m       = dvb-core.o

list-multi := dvb-core.o

dvb-core-objs = dmxdev.o dvb_demux.o dvb_net.o dvb_i2c.o \
		dvb_filter.o dvb_frontend.o dvbdev.o compat.o \
		dvb_ksyms.o

subdir-y    := frontends $(CARDS)
mod-subdirs := frontends $(CARDS)

EXTRA_CFLAGS = -I ../include -I . -MD

dvb-core.o: $(dvb-core-objs)
	$(LD) -r -o $@ $(dvb-core-objs)

include $(TOPDIR)/Rules.make
-include $(wildcard *.d) dummy


