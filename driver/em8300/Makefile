CURRENT:=$(shell head -1 /usr/include/linux/version.h|( read i j k; echo $$k|sed s/\"//g))
KERNEL_LOCATION=/usr/src/linux

DVBDRIV = ..
DXRDRIV = modules
DVB_INC = ../../include/

export CONFIG_SHELL := $(shell if [ -x "$$BASH" ]; then echo $$BASH; \
          else if [ -x /bin/bash ]; then echo /bin/bash; \
          else echo sh; fi ; fi)

export TOPDIR=$(KERNEL_LOCATION)

EM8300_OBJS = em8300_dvb.o $(DXRDRIV)/em8300_i2c.o $(DXRDRIV)/em8300_audio.o \
	      $(DXRDRIV)/em8300_fifo.o $(DXRDRIV)/em8300_video.o \
	      $(DXRDRIV)/em8300_misc.o $(DXRDRIV)/em8300_dicom.o \
	      $(DXRDRIV)/em8300_ucode.o $(DXRDRIV)/em8300_ioctl.o \
	      $(DXRDRIV)/em8300_spu.o $(DXRDRIV)/em9010.o

DXR_MODULES = $(DXRDRIV)/adv717x.o $(DXRDRIV)/bt865.o \
              $(DXRDRIV)/adv717x_eeprom.o

SOUND_BUILTIN=$(shell cat $(KERNEL_LOCATION)/include/linux/autoconf.h | grep " CONFIG_SOUND " | cut -d " " -f 3)
SOUND_MODULE=$(shell cat $(KERNEL_LOCATION)/include/linux/autoconf.h | grep " CONFIG_SOUND_MODULE " | cut -d " " -f 3)

I2CALGO_BUILTIN=$(shell cat $(KERNEL_LOCATION)/include/linux/autoconf.h | grep " CONFIG_I2C_ALGOBIT " | cut -d " " -f 3)
I2CALGO_MODULE=$(shell cat $(KERNEL_LOCATION)/include/linux/autoconf.h | grep " CONFIG_I2C_ALGOBIT_MODULE " | cut -d " " -f 3)

M_OBJS       = em8300.o 
MX_OBJS      = em8300_dvb.o  $(DXR_MODULES)

obj-m        = $(MX_OBJS) $(M_OBJS)
export-objs := $(MX_OBJS)

MODULES = $(M_OBJS)

EXTRA_CFLAGS = -I . -D__DVB_PACK__ -DUSE_OSD -I $(DVBDRIV)\
               -I $(DXRDRIV) -I $(KERNEL_LOCATION)/include/ -I $(DVB_INC)\
                -DEM8300_VIDEOMODE_DEFAULT=EM8300_VIDEOMODE_PAL \
                -DEM8300_AUDIOMODE_DEFAULT=EM8300_AUDIOMODE_ANALOG\
	        -D__DVB_PACK__  -DUSE_DVB

ifeq "$(SOUND_BUILTIN)" "1"
        EXTRA_CFLAGS+=-DREGISTER_DSP
else
        ifeq "$(SOUND_MODULE)" "1"
                EXTRA_CFLAGS+=-DREGISTER_DSP
        endif
endif

ifeq "$(I2CALGO_BUILTIN)" "1"
	EXTRA_CFLAGS+=-DI2C_BITBANGING
else
	ifeq "$(I2CALGO_MODULE)" "1"
                EXTRA_CFLAGS+=-DI2C_BITBANGING
	else
                ifeq "$(IS22)" "YES"
                        ifeq "$(I2C_LOCATION)" ""
                                EXTRA_CFLAGS+=-DI2C_BITBANGING
                        else
                                EXTRA_CFLAGS+=-I$(I2C_LOCATION) -DI2C_BITBANGING
                        endif
                endif
	endif
endif


here:   
	DIR=`pwd`; (cd $(KERNEL_LOCATION); make SUBDIRS=$$DIR modules)
	@echo


insmod: $(MODULES)
	(\
	insmod $(DVBDRIV)/dvbdev.o;\
	insmod i2c-core;\
	insmod i2c-algo-bit;\
	insmod adv717x_eeprom.o;\
	insmod adv717x.o pixelport_16bit=0 swap_redblue_pal=1;\
	insmod bt865.o;\
	insmod em8300.o use_bt865=1;\
	./microcode_upload.pl microcode/microcode0.bin;\
	)
#	insmod $(DXRDRIV)/adv717x.o pixelport_16bit=1
#	insmod em8300.o use_bt865=1 dicom_fix=0 dicom_control=0

rmmod:
	(\
	rmmod adv717x_eeprom; \
	rmmod em8300; \
	rmmod adv717x;\
	rmmod bt865; \
        rmmod i2c-algo-bit; \
        rmmod i2c-core;         \
	)

em8300.o : $(EM8300_OBJS)
	$(LD) -r -o $@ $(EM8300_OBJS)

clean:
	(cd $(DXRDRIV); make clean)
	rm -f $(M_OBJS) *.o .*.o.flags *~

include $(TOPDIR)/Rules.make

