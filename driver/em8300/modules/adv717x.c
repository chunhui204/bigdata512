/* 
   ADV7175A - Analog Devices ADV7175A video encoder driver version 0.0.3

   Copyright (C) 2000 Henrik Johannson <henrikjo@post.utfors.se>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/pci.h>
#include <linux/signal.h>
#include <asm/io.h>
#include <asm/pgtable.h>
#include <asm/page.h>
#include <linux/sched.h>
#include <asm/segment.h>
#include <linux/types.h>
#include <linux/wrapper.h>

#include <linux/videodev.h>
#include <linux/version.h>
#include <asm/uaccess.h>

#include <linux/i2c-algo-bit.h>
#include <linux/video_encoder.h>

#include "em8300_reg.h"
#include <linux/em8300.h>

#include "adv717x.h"

#include "encoder.h"

#ifdef MODULE
int pixelport_16bit = 1;

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,4,9)
MODULE_LICENSE("GPL");
#endif

MODULE_PARM(pixelport_16bit, "i");

int pixelport_other_pal = 1;
MODULE_PARM(pixelport_other_pal, "i");

int swap_redblue_pal = 0;
MODULE_PARM(swap_redblue_pal, "i");

static int color_bars = 0;
MODULE_PARM(color_bars, "i");
#endif

#define i2c_is_isa_client(clientptr) \
		((clientptr)->adapter->algo->id == I2C_ALGO_ISA)
#define i2c_is_isa_adapter(adapptr) \
		((adapptr)->algo->id == I2C_ALGO_ISA)


#define ADV7175_REG_MR0 0
#define ADV7175_REG_MR1 1
#define ADV7175_REG_TTXRQ_CTRL 0x24

static int adv717x_attach_adapter(struct i2c_adapter *adapter);
int adv717x_detach_client(struct i2c_client *client);
int adv717x_command(struct i2c_client *client, unsigned int cmd, void *arg);
void adv717x_inc_use (struct i2c_client *client);
void adv717x_dec_use (struct i2c_client *client);

#define CHIP_ADV7175A 1
#define CHIP_ADV7170  2

struct adv717x_data_s {
	int chiptype;
	int mode;
	int bars;
	int rgbmode;
	int enableoutput;

	unsigned char config[32];
	int configlen;
};

/* This is the driver that will be inserted */
static struct i2c_driver adv717x_driver = {
  /* name */		"ADV717X video encoder driver",
  /* id */		I2C_DRIVERID_ADV717X,
  /* flags */		I2C_DF_NOTIFY,
  /* attach_adapter */  &adv717x_attach_adapter,
  /* detach_client */   &adv717x_detach_client,
  /* command */		&adv717x_command,
  /* inc_use */		&adv717x_inc_use,
  /* dec_use */		&adv717x_dec_use
};

int adv717x_id = 0;

static unsigned char PAL_config_7170[27] = {
	0x05,   // Mode Register 0
	0x07,   // Mode Register 1
	0x48,   // Mode Register 2
	0x00,   // Mode Register 3
	0x00,   // Mode Register 4
	0x00,   // Reserved
	0x00,   // Reserved
	0x0d,   // Timing Register 0
	0x77,   // Timing Register 1
	0xcb,   // Subcarrier Frequency Register 0
	0x8a,   // Subcarrier Frequency Register 1
	0x09,   // Subcarrier Frequency Register 2
	0x2a,   // Subcarrier Frequency Register 3
	0x00,   // Subcarrier Phase Register 
	0x00,   // Closed Captioning Ext Register 0
	0x00,   // Closed Captioning Ext Register 1
	0x00,   // Closed Captioning Register 0 
	0x00,   // Closed Captioning Register 1 
	0x00,   // Pedestal Control Register 0
	0x00,   // Pedestal Control Register 1
	0x00,   // Pedestal Control Register 2
	0x00,   // Pedestal Control Register 3
	0x00,	// CGMS_WSS Reg 0
	0x00,	// CGMS_WSS Reg 1
	0x00,	// CGMS_WSS Reg 2
	0x00	// TeleText Control Register
};

static unsigned char NTSC_config_7170[27] = {
	0x10,   // Mode Register 0
	0x06,   // Mode Register 1
	0x08,   // Mode Register 2
	0x00,   // Mode Register 3
	0x00,   // Mode Register 4
	0x00,   // Reserved
	0x00,   // Reserved
	0x0d,   // Timing Register 0
	0x77,   // Timing Register 1
	0x16,   // Subcarrier Frequency Register 0
	0x7c,   // Subcarrier Frequency Register 1
	0xf0,   // Subcarrier Frequency Register 2
	0x21,   // Subcarrier Frequency Register 3
	0x00,   // Subcarrier Phase Register 
	0x00,   // Closed Captioning Ext Register 0
	0x00,   // Closed Captioning Ext Register 1
	0x00,   // Closed Captioning Register 0 
	0x00,   // Closed Captioning Register 1 
	0x00,   // Pedestal Control Register 0
	0x00,   // Pedestal Control Register 1
	0x00,   // Pedestal Control Register 2
	0x00,   // Pedestal Control Register 3
	0x00,	// CGMS_WSS Reg 0
	0x00,	// CGMS_WSS Reg 1
	0x00,	// CGMS_WSS Reg 2
	0x00	// TeleText Control Register
};

static unsigned char PAL_M_config_7175[16] = {   //These need to be tested
	0x06,   // Mode Register 0
	0x00,   // Mode Register 1
	0xa3,   // Subcarrier Frequency Register 0
	0xef, 	// Subcarrier Frequency Register 1
	0xe6, 	// Subcarrier Frequency Register 2
	0x21,  	// Subcarrier Frequency Register 3
	0x00,   // Subcarrier Phase Register
	0x0d,   // Timing Register 0
	0x00,   // Closed Captioning Ext Register 0
	0x00,   // Closed Captioning Ext Register 1
	0x00,   // Closed Captioning Register 0
	0x00,   // Closed Captioning Register 1
	0xc0, 	// Timing Register 1
	0x73,  	// Mode Register 2
	0x00,   // Pedestal Control Register 0
	0x00,   // Pedestal Control Register 1
};

static unsigned char PAL_config_7175[17] = {
	0x01,   // Mode Register 0
	0x06,   // Mode Register 1
	0xcb,   // Subcarrier Frequency Register 0
	0x8a,   // Subcarrier Frequency Register 1
	0x09,   // Subcarrier Frequency Register 2
	0x2a,   // Subcarrier Frequency Register 3
	0x00,   // Subcarrier Phase Register 
	0x0d,   // Timing Register 0
	0x00,   // Closed Captioning Ext Register 0
	0x00,   // Closed Captioning Ext Register 1
	0x00,   // Closed Captioning Register 0 
	0x00,   // Closed Captioning Register 1 
	0x73,   // Timing Register 1
	0x08,   // Mode Register 2
	0x00,   // Pedestal Control Register 0
	0x00,   // Pedestal Control Register 1
	0x00,   // Mode Register 3
};

static unsigned char PAL60_config_7175[16] = {
	0x12,   // Mode Register 0
	0x0,	// Mode Register 1
	0xcb,   // Subcarrier Frequency Register 0
	0x8a,   // Subcarrier Frequency Register 1
	0x09,   // Subcarrier Frequency Register 2
	0x2a,   // Subcarrier Frequency Register 3
	0x00,   // Subcarrier Phase Register 
	0x0d,   // Timing Register 0
	0x00,   // Closed Captioning Ext Register 0
	0x00,   // Closed Captioning Ext Register 1
	0x00,   // Closed Captioning Register 0 
	0x00,   // Closed Captioning Register 1 
	0x73,   // Timing Register 1
	0x08,   // Mode Register 2
	0x00,   // Pedestal Control Register 0
	0x00,   // Pedestal Control Register 1
};

static unsigned char NTSC_config_7175[16] = {
	0x04,   // Mode Register 0
	0x00,   // Mode Register 1
	0x16,   // Subcarrier Frequency Register 0
	0x7c, 	// Subcarrier Frequency Register 1
	0xf0, 	// Subcarrier Frequency Register 2
	0x21,  	// Subcarrier Frequency Register 3
	0x00,   // Subcarrier Phase Register
	0x0d,   // Timing Register 0
	0x00,   // Closed Captioning Ext Register 0
	0x00,   // Closed Captioning Ext Register 1
	0x00,   // Closed Captioning Register 0
	0x00,   // Closed Captioning Register 1
	0x77,  	// Timing Register 1
	0x00, 	// Mode Register 2
	0x00,   // Pedestal Control Register 0
	0x00,   // Pedestal Control Register 1
};

static int adv717x_update(struct i2c_client *client)
{
	struct adv717x_data_s *data = client->data ;
	char tmpconfig[32];
	int n,i;

	memcpy(tmpconfig, data->config, data->configlen);

	tmpconfig[1] |= data->bars ? 0x80:0;

	switch(data->chiptype) {
	case CHIP_ADV7175A:
		if (data->rgbmode) {
			tmpconfig[0] |= 0x40;
		}
		break;
	case CHIP_ADV7170:
		if (data->rgbmode) {
			tmpconfig[3] |= 0x10;
		}
		break;
	}
	
	if (!data->enableoutput) {
		tmpconfig[1] |= 0x7f;
	}
	
	for(i=0; i < data->configlen; i++) {
		n=i2c_smbus_write_byte_data(client,i,tmpconfig[i]);
	}

	i2c_smbus_write_byte_data(client,7, tmpconfig[7] & 0x7f);
	i2c_smbus_write_byte_data(client,7, tmpconfig[7] | 0x80);
	i2c_smbus_write_byte_data(client,7, tmpconfig[7] & 0x7f);

	return 0;
}

static int adv717x_setmode(int mode, struct i2c_client *client) {
	struct adv717x_data_s *data = client->data ;
	unsigned char *config=NULL;

	pr_debug("adv717x_setmode(%d,%p)\n", mode, client);
	
	switch(mode) {
	case ENCODER_MODE_PAL:
		printk(KERN_NOTICE "adv717x.o: Configuring for PAL\n");
		switch (data->chiptype) {
		case CHIP_ADV7175A:
			config = PAL_config_7175;
			data->configlen = sizeof(PAL_config_7175);
			break;
		case CHIP_ADV7170:
			config = PAL_config_7170;
			data->configlen = sizeof(PAL_config_7170);
			break;
		}
		break;
	case ENCODER_MODE_PAL_M:
		printk(KERN_NOTICE "adv717x.o: Configuring for PALM\n");
		switch (data->chiptype) {
		case CHIP_ADV7175A:
			config = PAL_config_7175;
			data->configlen = sizeof(PAL_M_config_7175);
			break;
		case CHIP_ADV7170:
			config = PAL_config_7170;
			data->configlen = sizeof(PAL_config_7170);
			break;
		}
		break;
	case ENCODER_MODE_PAL60:
		printk(KERN_NOTICE "adv717x.o: Configuring for PAL 60\n");
		switch (data->chiptype) {
		case CHIP_ADV7175A:
			config = PAL60_config_7175;
			data->configlen = sizeof(PAL60_config_7175);
			break;
		case CHIP_ADV7170:
			config = PAL_config_7170;
			data->configlen = sizeof(PAL_config_7170);
			break;
		}
		break;
	case ENCODER_MODE_NTSC:
		printk(KERN_NOTICE "adv717x.o: Configuring for NTSC\n");
	 	switch (data->chiptype) {
		case CHIP_ADV7175A:
			config = NTSC_config_7175;
			data->configlen = sizeof(NTSC_config_7175);
			break;
		case CHIP_ADV7170:
			config = NTSC_config_7170;
			data->configlen = sizeof(NTSC_config_7170);
			break;
		}
		break;
	default:
		return -1;
	}

	data->mode = mode;

	if (config) {
		memcpy(data->config, config, data->configlen);
	}

	return 0;
}

static int adv717x_setup(struct i2c_client *client)
{
	struct adv717x_data_s *data = client->data ;
	
	memset(data->config, 0, sizeof(data->config));

	data->bars=0;
	data->rgbmode=0;
	data->enableoutput=0;

	adv717x_setmode(ENCODER_MODE_PAL60, client);
	
	adv717x_update(client);
	
	return 0;
}

static int adv717x_detect(struct i2c_adapter *adapter, int address)
{
	struct adv717x_data_s *data;
	struct i2c_client *new_client;
	int mr0,mr1;
	int err;

	if (i2c_is_isa_adapter(adapter)) {
		printk(KERN_ERR "adv717xa.o: called for an ISA bus adapter?!?\n");
		return 0;
	}

	if (!i2c_check_functionality(adapter,I2C_FUNC_SMBUS_READ_BYTE | I2C_FUNC_SMBUS_WRITE_BYTE_DATA)) {
		return 0;
	}

	 
	 
	if (!(new_client = kmalloc(sizeof(struct i2c_client) + sizeof(struct adv717x_data_s), GFP_KERNEL))) {
		return -ENOMEM;
	}

	data = (struct adv717x_data_s *) (((struct i2c_client *) new_client) + 1);
	new_client->addr = address;
	new_client->data = data;
	new_client->adapter = adapter;
	new_client->driver = &adv717x_driver;
	new_client->flags = 0;

	i2c_smbus_write_byte_data(new_client, ADV7175_REG_MR1,0x55);
	mr1=i2c_smbus_read_byte_data(new_client, ADV7175_REG_MR1);
	 
	if (mr1 == 0x55) {
		mr0=i2c_smbus_read_byte_data(new_client, ADV7175_REG_MR0);

		if (mr0 & 0x20) {
			strcpy(new_client->name, "ADV7175A chip");
			data->chiptype = CHIP_ADV7175A;
			printk(KERN_NOTICE "adv717x.o: ADV7175A chip detected\n");
		} else {
			strcpy(new_client->name, "ADV7170 chip");
			data->chiptype = CHIP_ADV7170;
			printk(KERN_NOTICE "adv717x.o: ADV7170 chip detected\n");
		}
	 
		new_client->id = adv717x_id++;

		if ((err = i2c_attach_client(new_client))) {
			kfree(new_client);
			return err;
		}

		adv717x_setup(new_client);

		return 0;
	} 
	kfree(new_client);

	return 0;			  
}

static int adv717x_attach_adapter(struct i2c_adapter *adapter)
{
	adv717x_detect(adapter,0x6a);
	adv717x_detect(adapter,0x7a);
	adv717x_detect(adapter,0xa);
	return 0;
}


int adv717x_detach_client(struct i2c_client *client)
{
	int err;

	if ((err = i2c_detach_client(client))) {
		printk(KERN_ERR "adv717x.o: Client deregistration failed, client not detached.\n");
		return err;
	}

	kfree(client);

	return 0;
}

int adv717x_command(struct i2c_client *client, unsigned int cmd, void *arg)
{
	struct adv717x_data_s *data = client->data ;

	switch(cmd) {
	case ENCODER_CMD_SETMODE:
		adv717x_setmode((int)arg, client);
		adv717x_update(client);
		break;
	case ENCODER_CMD_ENABLEOUTPUT:
		data->enableoutput = (int)arg;
		adv717x_update(client);
		break;
	default:
	}

	return 0;
}

/* Nothing here yet */
void adv717x_inc_use (struct i2c_client *client)
{
#ifdef MODULE
	MOD_INC_USE_COUNT;
#endif
}

/* Nothing here yet */
void adv717x_dec_use (struct i2c_client *client)
{
#ifdef MODULE
	MOD_DEC_USE_COUNT;
#endif
}

/* ----------------------------------------------------------------------- */


#ifdef MODULE
int init_module(void)
#else
int adv717x_init(void)
#endif
{
	int pp_ntsc;
	int pp_pal;
	int rb_pal;
	int bars;

	if (pixelport_16bit) {
		pp_ntsc = pp_pal = 0x40;
		if (pixelport_other_pal) {
			pp_pal = 0x00;
		}
	} else {
		pp_ntsc = pp_pal = 0x00;
		if (pixelport_other_pal) {
			pp_pal = 0x40;
		}
	}

	if (swap_redblue_pal) {
		rb_pal = 0xA0;
	} else {
		rb_pal = 0x70;
	}

	if( color_bars ) {
		bars = 0x80;
	} else {
		bars = 0x00;
	}

	pr_debug("adv717x.o: pixelport_16bit: %d\n", pixelport_16bit);
	pr_debug("adv717x.o: pixelport_other_pal: %d\n", pixelport_other_pal);
	pr_debug("adv717x.o: color_bars: %d\n", color_bars);

	PAL_config_7170[7] = (PAL_config_7170[7] & ~0x40) | pp_pal;
	NTSC_config_7170[7] = (NTSC_config_7170[7] & ~0x40) | pp_ntsc;
	PAL_M_config_7175[7] = (PAL_M_config_7175[7] & ~0x40) | pp_pal;
	PAL_config_7175[7] = (PAL_config_7175[7] & ~0x40) | pp_pal;
	PAL60_config_7175[7] = (PAL60_config_7175[7] & ~0x40) | pp_pal;
	NTSC_config_7175[7] = (NTSC_config_7175[7] & ~0x40) | pp_ntsc;

	PAL_config_7175[12] = (PAL_config_7175[12] & ~0xF0) | rb_pal;
	PAL60_config_7175[12] = (PAL60_config_7175[12] & ~0xF0) | rb_pal;
	NTSC_config_7175[12] = (NTSC_config_7175[12] & ~0xF0) | rb_pal;

	PAL_config_7170[1] = (PAL_config_7170[1] & ~0x80) | bars;
	NTSC_config_7170[1] = (NTSC_config_7170[1] & ~0x80) | bars;
	PAL_M_config_7175[1] = (PAL_M_config_7175[1] & ~0x80) | bars;
	PAL_config_7175[1] = (PAL_config_7175[1] & ~0x80) | bars;
	PAL60_config_7175[1] = (PAL60_config_7175[1] & ~0x80) | bars;
	NTSC_config_7175[1] = (NTSC_config_7175[1] & ~0x80) | bars;

	return i2c_add_driver(&adv717x_driver);
}



#ifdef MODULE

void cleanup_module(void)
{
	i2c_del_driver(&adv717x_driver);
}

#endif
