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

int em8300_waitfor(struct em8300_s *em, int reg, int val, int mask)
{
	int tries;

	for (tries=0; tries < 100 ; tries++) {
		if ((em->mem[reg] & mask) == val) {
			return 0;
		}
		mdelay(10);
	}

	return -ETIME;
}

int em8300_waitfor_not(struct em8300_s *em, int reg, int val, int mask)
{
	int tries;

	for (tries = 0; tries < 100 ; tries++) {
		if ((em->mem[reg] & mask) != val) {
			return 0;
		}
		mdelay(10);
	}

	return -ETIME;
}

int em8300_setregblock(struct em8300_s *em, int offset, int val, int len)
{
	int i;

	for (i=1000; i; i--) {
		if (!read_register(0x1c1a)) {
			break;
		}
		if (!i) {
			return -ETIME;
		}
	}
#if 0 /* FIXME: was in the zeev01 branch, verify if it is necessary */
	val = val | (val << 8) | (val << 16) | (val << 24);
#endif

	em->mem[0x1c11] = offset & 0xffff;
	em->mem[0x1c12] = (offset >> 16) & 0xffff;
	em->mem[0x1c13] = len;
	em->mem[0x1c14] = len;
	em->mem[0x1c15] = 0;
	em->mem[0x1c16] = 1;
	em->mem[0x1c17] = 1;
	em->mem[0x1c18] = offset & 0xffff;
	em->mem[0x1c19] = 0;

	em->mem[0x1c1a] = 1;

	for (i = 0; i < len/4; i++) {
		em->mem[0x11800] = val;
	}

	switch (len % 4) {
	case 1:
		em->mem[0x10000] = val;
		break;
	case 2:
		em->mem[0x10800] = val;
		break;
	case 3:
		em->mem[0x11000] = val;
		break;
	}

	for (i=1000; i; i--) {
		if (!read_register(0x1c1a)) {
			break;
		}
		if (!i) {
			return -ETIME;
		}
	}

#if 0 /* FIXME: was in zeev01 branch, verify if it is necessary */	
	if (em8300_waitfor(em, 0x1c1a, 0, 1)) 
	        return -ETIME;  
#endif

	return 0;
}

int em8300_writeregblock(struct em8300_s *em, int offset, unsigned *buf, int len)
{
	int i;

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

	for (i = 0; i < len/4; i++) {
		em->mem[0x11800] = *buf++;
	}

	if (em8300_waitfor(em, 0x1c1a, 0, 1)) {
		return -ETIME;
	}
	return 0;
}
