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

#define i2c_is_isa_client(clientptr) \
		((clientptr)->adapter->algo->id == I2C_ALGO_ISA)
#define i2c_is_isa_adapter(adapptr) \
		((adapptr)->algo->id == I2C_ALGO_ISA)


#define EEPROM_REG_MR0 0
#define EEPROM_REG_TTXRQ_CTRL 0x24

#ifdef MODULE
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,4,9)
MODULE_LICENSE("GPL");
#endif
#endif

static int eeprom_attach_adapter(struct i2c_adapter *adapter);
int eeprom_detach_client(struct i2c_client *client);
int eeprom_command(struct i2c_client *client, unsigned int cmd, void *arg);
void eeprom_inc_use (struct i2c_client *client);
void eeprom_dec_use (struct i2c_client *client);

/* This is the driver that will be inserted */
static struct i2c_driver eeprom_driver = {
  /* name */		"EEPROM video encoder driver",
  /* id */		I2C_DRIVERID_AT24Cxx,
  /* flags */		I2C_DF_NOTIFY,
  /* attach_adapter */	&eeprom_attach_adapter,
  /* detach_client */	&eeprom_detach_client,
  /* command */		&eeprom_command,
  /* inc_use */		&eeprom_inc_use,
  /* dec_use */		&eeprom_dec_use
};

int eeprom_id = 0;

static int eeprom_detect(struct i2c_adapter *adapter, int address)
{
	struct i2c_client *new_client;
	int i,j,err;

	if (i2c_is_isa_adapter(adapter)) {
		printk(KERN_ERR "eeprom.o: called for an ISA bus adapter?!?\n");
		return 0;
	}

	if (! i2c_check_functionality(adapter,I2C_FUNC_SMBUS_READ_BYTE| I2C_FUNC_SMBUS_WRITE_BYTE_DATA)) {
		return 0;
	}

	 
	 
	if (! (new_client = kmalloc(sizeof(struct i2c_client), GFP_KERNEL))) {
		return -ENOMEM;
	}

	new_client->addr = address;
	new_client->adapter = adapter;
	new_client->driver = &eeprom_driver;
	new_client->flags = 0;

	if ((i2c_smbus_read_byte_data(new_client, 0x0) >= 0) && (i2c_smbus_read_byte_data(new_client, 0xff) >= 0) ) {
		strcpy(new_client->name, "256byte EEPROM chip");
		new_client->id = eeprom_id++;

		if ((err = i2c_attach_client(new_client))) {
			kfree(new_client);
			return err;
		}

		pr_debug("eeprom.o: EEPROM contents:\n");

		for (i = 0;i < 16; i++) {
			for (j = 0;j < 16; j++) {
				pr_debug("%02x ", i2c_smbus_read_byte_data(new_client, i*16+j));
			}
			pr_debug("\n");
		}

		return 0;
	}
	 
	kfree(new_client);
	return 0;			  
}

static int eeprom_attach_adapter(struct i2c_adapter *adapter)
{
	eeprom_detect(adapter,0x50);
	return 0;
}


int eeprom_detach_client(struct i2c_client *client)
{
	int err;

	if ((err = i2c_detach_client(client))) {
		printk(KERN_ERR "eeprom.o: Client deregistration failed, client not detached.\n");
		return err;
	}

	kfree(client);

	return 0;
}

/* No commands defined yet */
int eeprom_command(struct i2c_client *client, unsigned int cmd, void *arg)
{
	return 0;
}

/* Nothing here yet */
void eeprom_inc_use (struct i2c_client *client)
{
#ifdef MODULE
	MOD_INC_USE_COUNT;
#endif
}

/* Nothing here yet */
void eeprom_dec_use (struct i2c_client *client)
{
#ifdef MODULE
	MOD_DEC_USE_COUNT;
#endif
}

/* ----------------------------------------------------------------------- */


#ifdef MODULE
int init_module(void)
#else
int eeprom_init(void)
#endif
{
	return i2c_add_driver(&eeprom_driver);
}



#ifdef MODULE

void cleanup_module(void)
{
	i2c_del_driver(&eeprom_driver);
}

#endif
