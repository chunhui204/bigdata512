
O_target := av7110.o

export-objs := av7110_ksyms.o

list-multi := dvb-ttpci.o

dvb-ttpci-objs := saa7146_core.o saa7146_v4l.o av7110.o av7110_ir.o ttpci_eeprom.o

dvb-ttpci.o: $(dvb-ttpci-objs)
	$(LD) -r -o $@ $(dvb-ttpci-objs)


EXTRA_CFLAGS = -I.. -I. -DCONFIG_DVB_AV7110_OSD -MD


ifdef DVB_PACK 
  obj-m += dvb-ttpci.o
  EXTRA_CFLAGS += -I../../include
else
  obj-$(CONFIG_AV7110_DVB) += dvb-ttpci.o
endif

# Extract lists of the multi-part drivers. 
# The 'int-*' lists are the intermediate files used to build the multi's. 
 
multi-y         := $(filter $(list-multi), $(obj-y)) 
multi-m         := $(filter $(list-multi), $(obj-m)) 
int-y           := $(sort $(foreach m, $(multi-y), $($(basename $(m))-objs))) 
int-m           := $(sort $(foreach m, $(multi-m), $($(basename $(m))-objs))) 
 
# Files that are both resident and modular: remove from modular. 
 
obj-m           := $(filter-out $(obj-y), $(obj-m)) 
int-m           := $(filter-out $(int-y), $(int-m)) 
 
# Take multi-part drivers out of obj-y and put components in. 
 
obj-y           := $(filter-out $(list-multi), $(obj-y)) $(int-y) 
 
include $(TOPDIR)/Rules.make 


##
# Targets
#

newfirm: 
	DIR=`pwd`/arm_firm; (cd ./arm_firm/dvb25/; make ARM_FIRM=$$DIR)
	cp ./arm_firm/dvb25/arm_firm25/DPRAM arm_firm/Dpram
	cp ./arm_firm/dvb25/arm_firm25/ROOT arm_firm/Root

newfirm_dist: 
	DIR=`pwd`/arm_firm; (cd ./arm_firm/dvb25/; make ARM_FIRM=$$DIR FIRM_DIST=1)
	cp ./arm_firm/dvb25/arm_firm25/DPRAM Dpram
	cp ./arm_firm/dvb25/arm_firm25/ROOT Root

firmclean: 
	(cd ./arm_firm/dvb25/; make clean)

fdump: fdump.c
	$(HOSTCC) $(HOSTCFLAGS) -o fdump fdump.c

clean:
	-rm -f *.o *.d .*.o.flags *~ av7110_firm.h fdump

insmod:
	insmod input; \
	insmod evdev; \
	insmod dvb-ttpci.o;

rmmod:
	rmmod dvb-ttpci; \
	rmmod evdev; \
	rmmod input;


##
#  Extra Rules
#

av7110.c: av7110_firm.h

av7110_firm.h: Root Dpram fdump
	./fdump > av7110_firm.h

-include $(wildcard *.d) dummy

