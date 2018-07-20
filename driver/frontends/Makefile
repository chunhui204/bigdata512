obj-y           := 
obj-m           := 
obj-n           := 
obj-            := 

SUB_DIRS     :=  
MOD_SUB_DIRS := $(SUB_DIRS) 
ALL_SUB_DIRS := $(SUB_DIRS) 

O_TARGET := frontends.o 

# All of the (potential) objects that export symbols. 
# This list comes from 'grep -l EXPORT_SYMBOL *.[hc]'. 
 
export-objs     := 
 
list-multi      := 

EXTRA_CFLAGS = -MD

ifdef DVB_PACK 
obj-m +=  alps_bsrv2.o alps_tdmb7.o alps_tdlb7.o grundig_29504-401.o grundig_29504-491.o stv0299.o ves1820.o tda1004x.o mt312.o
EXTRA_CFLAGS += -I ../../include -I . -I.. 
else
obj-$(CONFIG_DVB_ALPS_BSRV2) += alps_bsrv2.o
obj-$(CONFIG_DVB_ALPS_TDMB7) += alps_tdmb7.o
obj-$(CONFIG_DVB_ALPS_TDLB7) += alps_tdlb7.o
obj-$(CONFIG_DVB_GRUNDIG_29504_491) += grundig_29504-491.o
obj-$(CONFIG_DVB_GRUNDIG_29504_401) += grundig_29504-401.o
obj-$(CONFIG_DVB_STV0299) += stv0299.o
obj-$(CONFIG_DVB_VES1820) += ves1820.o
obj-$(CONFIG_DVB_TDA1004X) += tda1004x.o
obj-$(CONFIG_DVB_MT312) += mt312.o
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
 
fastdep: 
 
clean:
	-rm -f $(M_OBJS) $(MX_OBJS) *.o *.d .*.o.flags *~

insmod:
	$(foreach frontend, $(obj-m), insmod $(frontend); )

rmmod:
	$(foreach frontend, $(obj-m), rmmod $(frontend:.o=); )

-include $(wildcard *.d) dummy
