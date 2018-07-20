
O_target := skystar2.o

list-multi := skystar2.o

EXTRA_CFLAGS = -I.. -I.

ifdef DVB_PACK 
  obj-m += skystar2.o
  EXTRA_CFLAGS += -I../../include
else
  obj-$(CONFIG_SKYSTAR2_DVB) += skystar2.o
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

clean:
	-rm -f *.o *.d .*.o.flags *~

# ignore errors here because insmod fails if you don't have a skystar2 card
insmod:
	-insmod skystar2.o;

rmmod:
	-rmmod skystar2;


##
#  Extra Rules
#

-include $(wildcard *.d) dummy

