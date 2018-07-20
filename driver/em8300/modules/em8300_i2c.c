#define __NO_VERSION__

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/string.h>
#include <linux/pci.h>
#include <asm/io.h>
#include <asm/uaccess.h>

#include <linux/i2c-algo-bit.h>

#include "em8300_reg.h"
#include <linux/em8300.h>

#include "adv717x.h"
#include "bt865.h"
//#include <linux/sensors.h>

#define I2C_HW_B_EM8300 0xa

struct private_data_s {
	int clk;
	int data;
	struct em8300_s *em;
};

/* ----------------------------------------------------------------------- */
/* I2C bitbanger functions						   */
/* ----------------------------------------------------------------------- */

/* software I2C functions */

static void em8300_setscl(void *data,int state)
{
	struct private_data_s *p = (struct private_data_s *)data;
	int sel = p->clk << 8;

	p->em->mem[p->em->i2c_oe_reg] = sel | p->clk;
	p->em->mem[p->em->i2c_pin_reg] = sel | (state ? p->clk : 0);
}

static void em8300_setsda(void *data,int state)
{
	struct private_data_s *p = (struct private_data_s *)data;
	struct em8300_s *em = p->em;
	int sel = p->data << 8;

	em->mem[em->i2c_oe_reg] = sel | p->data;
	em->mem[em->i2c_pin_reg] = sel | (state ? p->data : 0);
}

static int em8300_getscl(void *data)
{
	struct private_data_s *p = (struct private_data_s *)data;
	struct em8300_s *em = p->em;

	return em->mem[em->i2c_pin_reg] & (p->clk << 8);
}

static int em8300_getsda(void *data)
{
	struct private_data_s *p = (struct private_data_s *)data;
	struct em8300_s *em = p->em;

	return em->mem[em->i2c_pin_reg] & (p->data << 8);
}


static int em8300_i2c_reg(struct i2c_client *client)
{
	struct em8300_s *em = client->adapter->data;

	switch (client->driver->id) {
	case I2C_DRIVERID_ADV717X:
		if (!strncmp(client->name, "ADV7175", 7)) {
			em->encoder_type = ENCODER_ADV7175;
		}
		if (!strncmp(client->name, "ADV7170", 7)) {
			em->encoder_type = ENCODER_ADV7170;
		}
		em->encoder = client;
		break;
	case I2C_DRIVERID_AT24Cxx:
		em->eeprom = client;
		break;
	case  I2C_DRIVERID_BT865:
		em->encoder_type = ENCODER_BT865;
		em->encoder = client;
		break;
	default:
		printk(KERN_ERR "em8300_i2c: unknown client id\n");
		return -ENODEV;
	}

	return 0;
}

static int em8300_i2c_unreg(struct i2c_client *client)
{
	struct em8300_s *em = client->adapter->data;

	switch (client->driver->id) {
	case I2C_DRIVERID_ADV717X:
		em->encoder = NULL;
		break;
	case  I2C_DRIVERID_BT865:
		em->encoder = NULL;
		break;
/*
	case I2C_DRIVERID_EEPROM:
	        em->eeprom = NULL;
	        break;
*/
	}

	return 0;
}

static void em8300_i2c_inc(struct i2c_adapter *adapter)
{	
}

static void em8300_i2c_dec(struct i2c_adapter *adapter)
{	
}

/* ----------------------------------------------------------------------- */
/* I2C functions							   */
/* ----------------------------------------------------------------------- */
int em8300_i2c_init(struct em8300_s *em)
{
	int ret;
	struct private_data_s *pdata;

	switch (em->chip_revision) {
	case 2:
		em->i2c_oe_reg = EM8300_I2C_OE;
		em->i2c_pin_reg = EM8300_I2C_PIN;
		break;
	case 1:
		em->i2c_oe_reg = 0x1f4f;
		em->i2c_pin_reg = EM8300_I2C_OE;
		break;
	}
	
	/*
	  Reset devices on I2C bus
	*/
	em->mem[em->i2c_pin_reg] = 0x3f3f;
	em->mem[em->i2c_oe_reg] = 0x3b3b;
	em->mem[em->i2c_pin_reg] = 0x0100;
	em->mem[em->i2c_pin_reg] = 0x0101;
	em->mem[em->i2c_pin_reg] = 0x0808;
	
	/*
	  Setup info structure for bus 1
	*/
	
	em->i2c_data_1.setsda = &em8300_setsda;
	em->i2c_data_1.setscl = &em8300_setscl;
	em->i2c_data_1.getsda = &em8300_getsda;
	em->i2c_data_1.getscl = &em8300_getscl;
	em->i2c_data_1.mdelay = 10;
	em->i2c_data_1.udelay = 10;
	em->i2c_data_1.timeout = 100;

	pdata = kmalloc(sizeof(struct private_data_s),GFP_KERNEL);
	pdata->clk = 0x10;
	pdata->data = 0x8;
	pdata->em = em;
	
	em->i2c_data_1.data = pdata;

	strcpy(em->i2c_ops_1.name, "EM8300 I2C bus 1");
	em->i2c_ops_1.id = I2C_HW_B_EM8300;
	em->i2c_ops_1.algo = NULL;
	em->i2c_ops_1.algo_data = &em->i2c_data_1;
	em->i2c_ops_1.inc_use = em8300_i2c_inc;
	em->i2c_ops_1.dec_use = em8300_i2c_dec;
	em->i2c_ops_1.client_register = em8300_i2c_reg;
	em->i2c_ops_1.client_unregister = em8300_i2c_unreg;
	em->i2c_ops_1.data = em;
	
	ret = i2c_bit_add_bus(&em->i2c_ops_1);

	if (ret) {
		return ret;
	}

	/*
	  Setup info structure for bus 2
	*/

	em->i2c_data_2.setsda = &em8300_setsda;
	em->i2c_data_2.setscl = &em8300_setscl;
	em->i2c_data_2.getsda = &em8300_getsda;
	em->i2c_data_2.getscl = &em8300_getscl;
	em->i2c_data_2.mdelay = 10;
	em->i2c_data_2.udelay = 10;
	em->i2c_data_2.timeout = 100;

	pdata = kmalloc(sizeof(struct private_data_s),GFP_KERNEL);
	pdata->clk = 0x4;
	pdata->data = 0x8;
	pdata->em = em;
	
	em->i2c_data_2.data = pdata;

	strcpy(em->i2c_ops_2.name, "EM8300 I2C bus 2");
	em->i2c_ops_2.id = I2C_HW_B_EM8300;
	em->i2c_ops_2.algo = NULL;
	em->i2c_ops_2.algo_data = &em->i2c_data_2;
	em->i2c_ops_2.inc_use = em8300_i2c_inc;
	em->i2c_ops_2.dec_use = em8300_i2c_dec;
	em->i2c_ops_2.client_register = em8300_i2c_reg;
	em->i2c_ops_2.client_unregister = em8300_i2c_unreg;
	em->i2c_ops_2.data = em;
	
	ret = i2c_bit_add_bus(&em->i2c_ops_2);
	return ret;
}

void em8300_i2c_exit(struct em8300_s *em)
{
	/* unregister i2c_bus */
	kfree(em->i2c_data_1.data);
	kfree(em->i2c_data_2.data);
	i2c_bit_del_bus(&em->i2c_ops_1);
	i2c_bit_del_bus(&em->i2c_ops_2);
}

void em8300_clockgen_write(struct em8300_s *em, int abyte) {
	int i;

	em->mem[em->i2c_pin_reg] = 0x808;
	for (i=0; i < 8; i++) {
		em->mem[em->i2c_pin_reg] = 0x2000;
		em->mem[em->i2c_pin_reg] = 0x800 | ((abyte & 1) ? 8:0);
		em->mem[em->i2c_pin_reg] = 0x2020;
		abyte >>= 1;
	}

	em->mem[em->i2c_pin_reg] = 0x200;
	udelay(10);
	em->mem[em->i2c_pin_reg] = 0x202;
}	

static void I2C_clk(struct em8300_s *em, int level)
{
	em->mem[em->i2c_pin_reg] = 0x1000 | (level ? 0x10 : 0);
	udelay(1);
}

static void I2C_data(struct em8300_s *em, int level)
{
	em->mem[em->i2c_pin_reg] = 0x800 | (level ? 0x8 : 0);
	udelay(1);
}

static void I2C_drivedata(struct em8300_s *em, int level)
{
	em->mem[em->i2c_oe_reg] = 0x800 | (level ? 0x8 : 0);
	udelay(1);
}

#define I2C_read_data ((em->mem[em->i2c_pin_reg] & 0x800) ? 1 : 0)

static void I2C_out(struct em8300_s *em, int data, int bits)
{
	int i;
	for (i = bits-1; i >= 0; i--) {
		I2C_data(em, data & (1<<i));
		I2C_clk(em, 1);
		I2C_clk(em, 0);
	}
}

static int I2C_in(struct em8300_s *em, int bits)
{
	int i,data=0;
	
	for(i = bits-1; i >= 0; i--) {
		data |= I2C_read_data << i;
		I2C_clk(em, 0);
		I2C_clk(em, 1);
	}
	return data;
}

static void sub_23660(struct em8300_s *em, int arg1, int arg2)
{
	I2C_clk(em, 0);
	I2C_out(em, arg1, 8);
	I2C_data(em, arg2);  
	I2C_clk(em, 1);
}


static void sub_236f0 (struct em8300_s *em,int arg1, int arg2, int arg3)
{
	I2C_clk(em, 1);
	I2C_data(em, 1);
	I2C_clk(em, 0);
	I2C_data(em, 1);
	I2C_clk(em, 1);
	I2C_clk(em, 0);

	sub_23660(em, 1, arg2);

	sub_23660(em, arg1, arg3);
}

void em9010_write(struct em8300_s *em, int reg, int data)
{
	sub_236f0(em, reg, 1, 0);
	sub_23660(em, data, 1);
}

int em9010_read(struct em8300_s *em, int reg) {
	int val;

	sub_236f0(em, reg, 0, 0);
	I2C_drivedata(em, 0);
	val = I2C_in(em, 8);
	I2C_drivedata(em, 1);
	I2C_clk(em, 0);
	I2C_data(em, 1);
	I2C_clk(em, 1);

	return val;
}

/* loc_2A5d8 in cl.asm
   call dword ptr [exx+0x14]
*/
int em9010_read16(struct em8300_s *em, int reg)
{
	if (reg > 128) {
		em9010_write(em, 3, 0);
		em9010_write(em, 4, reg);
	} else {
		em9010_write(em, 4, 0);
		em9010_write(em, 3, reg);
	}

	return em9010_read(em, 2) | (em9010_read(em, 1) << 8);
}

/* loc_2A558 in cl.asm
   call dword ptr [exx+0x10]
*/
void em9010_write16(struct em8300_s *em, int reg, int value)
{
	if (reg > 128) {
		em9010_write(em, 3, 0);
		em9010_write(em, 4, reg);
	} else {
		em9010_write(em, 4, 0);
		em9010_write(em, 3, reg);
	}
	em9010_write(em, 2, value & 0xff);
	em9010_write(em, 1, value >> 8);
}

