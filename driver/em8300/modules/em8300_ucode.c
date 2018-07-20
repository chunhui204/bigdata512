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

#include <linux/soundcard.h>

#include "em8300_reg.c"

static
int upload_block(struct em8300_s *em, int blocktype, int offset, int len, unsigned char *buf)
{
	int i, val;

	switch (blocktype) {
	case 4:
		offset *= 2;
		em->mem[0x1c11] = offset & 0xffff;
		em->mem[0x1c12] = (offset >> 16) & 0xffff;
		em->mem[0x1c13] = len;
		em->mem[0x1c14] = len;
		em->mem[0x1c15] = 0;
		em->mem[0x1c16] = 1;
		em->mem[0x1c17] = 1;
		em->mem[0x1c18] = offset & 0xffff;
		em->mem[0x1c19] = (offset >> 16) & 0xffff;

		em->mem[0x1c1a] = 1;

		for (i=0; i < len; i+=4) {
			val = (buf[i+2] << 24) | (buf[i+3] << 16) | (buf[i] << 8) | buf[i+1];
			em->mem[0x11800] = val;
		}
	
		if (em8300_waitfor(em, 0x1c1a, 0, 1)) {
			return -ETIME;
		}
		break;
	case 1:
		for (i=0; i < len; i+=4) {
			val = (buf[i+1] << 24) | (buf[i] << 16) | (buf[i+3] << 8) | buf[i+2];
			em->mem[offset/2 + i/4] = val;
		}
		break;
	case 2:
		for (i=0; i < len; i+=2) {
			val = (buf[i+1] << 8) | buf[i];
			em->mem[0x1000 + offset + i/2] = val;
		}
		break;
	}

	return 0;
}

static
int upload_prepare(struct em8300_s *em)
{
	em->mem[0x30000] = 0x1ff00;
	em->mem[0x1f50] = 0x123;

	em->mem[0x20001] = 0x0;
	em->mem[0x2000] = 0x2;
	em->mem[0x2000] = 0x0;
	em->mem[0x1ff8] = 0xffff;
	em->mem[0x1ff9] = 0xffff;
	em->mem[0x1ff8] = 0xff00;
	em->mem[0x1ff9] = 0xff00;

	if (em->chip_revision == 1) {
		em->mem[0x1c04] = 0x8c7;
		em->mem[0x1c00] = 0x80;
		em->mem[0x1c04] = 0xc7;
	}
	em->mem[0x1c04] = em->var_ucode_reg3;
	em->mem[0x1c00] = em->var_ucode_reg1;
	em->mem[0x1c04] = em->var_ucode_reg2;
	 
	em->mem[0x1c08];
	em->mem[0x1c10] = 0x8;
	em->mem[0x1c20] = 0x8;
	em->mem[0x1c30] = 0x8;
	em->mem[0x1c40] = 0x8;
	em->mem[0x1c50] = 0x8;
	em->mem[0x1c60] = 0x8;
	em->mem[0x1c70] = 0x8;
	em->mem[0x1c80] = 0x8;
	em->mem[0x1c90] = 0x10;
	em->mem[0x1ca0] = 0x10;
	em->mem[0x1cb0] = 0x8;
	em->mem[0x1cc0] = 0x8;
	em->mem[0x1cd0] = 0x8;
	em->mem[0x1ce0] = 0x8;
	em->mem[0x1c01] = 0x5555;
	em->mem[0x1c02] = 0x55a;
	em->mem[0x1c03] = 0x0;
	
	return 0;
}

int em8300_ucode_upload(struct em8300_s *em, void *ucode_user, int ucode_size)
{
	int flags,offset,len;
	unsigned char *ucode,*p;
	int memcount,i;
	char regname[128];

	ucode = kmalloc(ucode_size, GFP_KERNEL);
	if (!ucode) {
		return -ENOMEM;
	}

	upload_prepare(em);
	
	copy_from_user(ucode, ucode_user, ucode_size);

	memcount=0;

	p = ucode;
	while (memcount < ucode_size) {
		flags =  p[0] | (p[1] << 8); p+=2;
		offset = p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24); p+=4; 
		len =	p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24); p+=4;
		memcount+=10;
		len *= 2;

		if (!flags)
			break;
	
		switch (flags & 0xf00) {
		case 0:
			upload_block(em, flags, offset, len, p);
			break;
		case 0x200:
			for (i=0;i < len; i++) {
				if(p[i]) {
					regname[i] = p[i] ^ 0xff;
				} else {
					break;
				}
			}
			regname[i]=0;

			for (i=0; i < MAX_UCODE_REGISTER; i++) {
				if (!strcmp(ucodereg_names[i], regname)) {
					em->ucode_regs[i] = 0x1000 + offset;
					break;
				}
			}
			break;
		}
		memcount += len;
		p += len;
	}
	
	kfree(ucode);
	return 0;
}
