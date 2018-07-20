# where the kernel sources are located
KERNEL_LOCATION=/usr/src/linux

# If your I2C stuff is builtin to kernel leave this blank
# otherwise change to the location of the I2C headers
#I2C_LOCATION=/usr/local/include
I2C_LOCATION=

#################################################
# some magic for using linux kernel settings
# when compiling module(s)

CC=gcc

EXTRA_CFLAGS = -g -I ../include -I $(KERNEL_LOCATION)/include/ \
		-DEM8300_VIDEOMODE_DEFAULT=EM8300_VIDEOMODE_PAL \
		-DEM8300_AUDIOMODE_DEFAULT=EM8300_AUDIOMODE_ANALOG

MODULES = em8300.o adv717x.o bt865.o adv717x_eeprom.o

EM8300_OBJS = em8300_main.o em8300_i2c.o em8300_audio.o em8300_fifo.o \
	   em8300_video.o em8300_misc.o em8300_dicom.o em8300_ucode.o \
	   em8300_ioctl.o em8300_spu.o em9010.o

MODVERS=$(shell cat $(KERNEL_LOCATION)/include/linux/autoconf.h | grep "CONFIG_MODVERSIONS" | cut -d " " -f 3)
KERNVER=$(shell cat $(KERNEL_LOCATION)/include/linux/version.h | grep "UTS" | sed -e 's/.*"\(.*\)"/\1/')

SOUND_BUILTIN=$(shell cat $(KERNEL_LOCATION)/include/linux/autoconf.h | grep " CONFIG_SOUND " | cut -d " " -f 3)
SOUND_MODULE=$(shell cat $(KERNEL_LOCATION)/include/linux/autoconf.h | grep " CONFIG_SOUND_MODULE " | cut -d " " -f 3)

I2CALGO_BUILTIN=$(shell cat $(KERNEL_LOCATION)/include/linux/autoconf.h | grep " CONFIG_I2C_ALGOBIT " | cut -d " " -f 3)
I2CALGO_MODULE=$(shell cat $(KERNEL_LOCATION)/include/linux/autoconf.h | grep " CONFIG_I2C_ALGOBIT_MODULE " | cut -d " " -f 3)

CFLAGS=-D__KERNEL__ -DMODULE $(EM8300_DEBUG) -Wall -Wstrict-prototypes -O2 -fomit-frame-pointer $(EXTRA_CFLAGS)

IS22=NO
IS22=$(patsubst 2.2.%,YES,$(KERNVER))

ifeq "$(MODVERS)" "1"
	CFLAGS+=-DMODVERSIONS -include $(KERNEL_LOCATION)/include/linux/modversions.h
endif

ifeq "$(SOUND_BUILTIN)" "1"
	CFLAGS+=-DREGISTER_DSP
else
	ifeq "$(SOUND_MODULE)" "1"
		CFLAGS+=-DREGISTER_DSP
	endif
endif

ifeq "$(I2CALGO_BUILTIN)" "1"
	CFLAGS+=-DI2C_BITBANGING
else
	ifeq "$(I2CALGO_MODULE)" "1"
		CFLAGS+=-DI2C_BITBANGING
	else
		ifeq "$(IS22)" "YES"
			ifeq "$(I2C_LOCATION)" ""
				CFLAGS+=-DI2C_BITBANGING
			else
				CFLAGS+=-I$(I2C_LOCATION) -DI2C_BITBANGING
			endif
		endif
	endif
endif



all : $(MODULES)

debug :
	EM8300_DEBUG="-DDEBUG" make

em8300.o : $(EM8300_OBJS)
	$(LD) $(EXTRA_LDFLAGS) -r -o $@ $(EM8300_OBJS)

install : $(MODULES)
	if [ `uname -r | awk -F '.' '{ print $$2 }'` -eq 4 ]; then \
	     for module in $^ ;\
	     do \
	     	install -D $$module $(prefix)/lib/modules/`uname -r`/kernel/drivers/video/$$module ;\
	    	echo "2.4: " $$module ;\
	     done \
	else \
	    for modules in $^ ;\
	    do \
	     	install -D $$module $(prefix)/lib/modules/`uname -r`/misc/$$module ;\
	    	echo "2.2: " $$module ;\
	    done \
	fi
	@depmod -a
	@echo em8300.h
	@install -D ../include/linux/em8300.h \
		$(KERNEL_LOCATION)/include/linux/em8300.h
	@echo Installed to running kernel

install-newkern : $(MODULES)
	@for module in $^ ;\
	do \
	install -D $$module /lib/modules/$(KERNVER)/kernel/drivers/video/$$module ;\
	echo $$module ;\
	@echo em8300.h
	@install -D ../include/linux/em8300.h \
		$(KERNEL_LOCATION)/include/linux/em8300.h
	done
	@echo Installed to kernel version of header files

clean :
	rm -f *.o
