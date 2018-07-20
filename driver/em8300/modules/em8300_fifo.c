/*
	Copyright (C) 2000 Henrik Johansson <henrikjo@post.utfors.se>

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

#define __NO_VERSION__

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include <linux/pci.h>
#include <linux/signal.h>
#include <linux/string.h>
#include <linux/time.h>
#include <asm/io.h>
#include <asm/pgtable.h>
#include <asm/page.h>
#include <linux/sched.h>
#include <asm/segment.h>

#include <linux/version.h>
#include <asm/uaccess.h>

#include <linux/i2c-algo-bit.h>

#include "em8300_reg.h"
#include <linux/em8300.h>
#include "em8300_fifo.h"



int em8300_fifo_init(struct em8300_s *em, struct fifo_s *f, int start, int wrptr, int rdptr, int pcisize, int slotsize, int fifotype)
{
	int i;
	unsigned phys;

	f->em = em;
	f->preprocess_ratio = 1;
	f->preprocess_cb = NULL;
	f->preprocess_maxbufsize = -1;
	
	f->type = fifotype;
	
	f->writeptr = (unsigned *)ucregister_ptr(wrptr);
	f->readptr = (unsigned *)ucregister_ptr(rdptr);

	switch (f->type) {
	case FIFOTYPE_AUDIO:
		f->slotptrsize = 3;
		f->slots.a = (struct audio_fifoslot_s *) ucregister_ptr(start);
		f->nslots = read_ucregister(pcisize) / 3;
		break;
	case FIFOTYPE_VIDEO:
		f->slotptrsize = 4;
		f->slots.v = (struct video_fifoslot_s *) ucregister_ptr(start);
		f->nslots = read_ucregister(pcisize) / 4;
		break;
	}
	
	f->slotsize = slotsize;
	f->start = ucregister(start) - 0x1000;
	f->threshold = f->nslots / 2;
	f->waiting=0;

	f->bytes = 0;

	if (f->fifobuffer) {
		kfree(f->fifobuffer);
	}
	
	f->fifobuffer = kmalloc(f->nslots * f->slotsize, GFP_KERNEL);
	if (f->fifobuffer == NULL) {
		return -ENOMEM;
	}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,3,0)
	init_waitqueue(&f->wait);
#else
	init_waitqueue_head(&f->wait);
#endif	
	
	for (i=0; i < f->nslots; i++) {
		phys = virt_to_phys(f->fifobuffer + i*f->slotsize);
		switch (f->type) {
		case FIFOTYPE_AUDIO:	
			f->slots.a[i].physaddress_hi = phys >> 16;
			f->slots.a[i].physaddress_lo = phys & 0xffff;
			f->slots.a[i].slotsize = f->slotsize;
			break;
		case FIFOTYPE_VIDEO:
			f->slots.v[i].flags = 0;
			f->slots.v[i].physaddress_hi = phys >> 16;
			f->slots.v[i].physaddress_lo = phys & 0xffff;
			f->slots.v[i].slotsize = f->slotsize;
		}
	}

	f->valid = 1;
	
	return 0;
}

void em8300_fifo_free(struct fifo_s *f)
{
	if (f) {
		if(f->valid && f->fifobuffer) {
			kfree(f->fifobuffer);
		}
		kfree(f);
	}
}

struct fifo_s *em8300_fifo_alloc()
{
	struct fifo_s *f = kmalloc(sizeof(struct fifo_s), GFP_KERNEL);
	if (f) {
		memset(f, 0, sizeof(struct fifo_s));
	}
	return f;
}

int em8300_fifo_check(struct fifo_s *fifo)
{
	int freeslots;
	
	if (!fifo || !fifo->valid) {
		return -1;
	}
	
	freeslots = ((*fifo->readptr - *fifo->writeptr) / 4 + fifo->nslots - 1) % fifo->nslots;

	if ((freeslots > fifo->threshold) && fifo->waiting) {
		fifo->waiting=0;
		wake_up_interruptible(&fifo->wait);
	}

	return 0;
}

int em8300_fifo_sync(struct fifo_s *fifo)
{
	while (*fifo->writeptr != *fifo->readptr) {
		fifo->waiting=1;	
		interruptible_sleep_on(&fifo->wait);
		fifo->waiting=0;

		if (signal_pending(current)) {
			printk(KERN_ERR "em8300.o: FIFO sync interrupted\n");		
			return -EINTR;
		}
	}

	return 0;
}

int em8300_fifo_write(struct fifo_s *fifo, int n, const char *userbuffer,
			  int flags)
{
	int freeslots,writeindex,i,bytes_transferred=0,copysize;

	if (!fifo || !fifo->valid) {
		return -1;
	}
	
	freeslots = ((*fifo->readptr - *fifo->writeptr) / fifo->slotptrsize + fifo->nslots - 1) % fifo->nslots;

	writeindex = (*fifo->writeptr - fifo->start) / fifo->slotptrsize;
	for (i=0; i < freeslots && n; i++) {
		copysize = n < fifo->slotsize / fifo->preprocess_ratio ? n : fifo->slotsize / fifo->preprocess_ratio;

		if ((fifo->preprocess_maxbufsize > 0) && (copysize > fifo->preprocess_maxbufsize)) {
			copysize = fifo->preprocess_maxbufsize;
		}

		switch (fifo->type) {
		case FIFOTYPE_AUDIO:
			fifo->slots.a[writeindex].slotsize = copysize*fifo->preprocess_ratio;
			break;
		case FIFOTYPE_VIDEO:
			fifo->slots.v[writeindex].flags = flags;
			fifo->slots.v[writeindex].slotsize = copysize*fifo->preprocess_ratio;
			break;
		}	

		if (fifo->preprocess_cb) {
			fifo->preprocess_cb(fifo->em, fifo->fifobuffer + writeindex*fifo->slotsize, userbuffer, copysize);
		} else {
			copy_from_user(fifo->fifobuffer + writeindex*fifo->slotsize, userbuffer, copysize);
		}
	
		writeindex++;
		writeindex %= fifo->nslots;
		n -= copysize;
		userbuffer += copysize;
		bytes_transferred += copysize;
		fifo->bytes += copysize;
	}
	*fifo->writeptr = fifo->start + writeindex*fifo->slotptrsize;

	return bytes_transferred;
}


int em8300_fifo_writeblocking(struct fifo_s *fifo, int n, const char *userbuffer, int flags)
{
	int total_bytes_written=0,copy_size;

	if(!fifo->valid) {
		return -EPERM;
	}
	
	while (n) {
		copy_size = em8300_fifo_write(fifo, n, userbuffer, flags);

		if (!copy_size) {
			fifo->waiting=1;
			interruptible_sleep_on(&fifo->wait);
		}
	
		if (signal_pending(current)) {
			if (total_bytes_written) {
				return total_bytes_written;
			} else {
				return -EINTR;
			}
		}

		if (copy_size < 0) {
			return -EIO;
		}
	
		n -= copy_size;
		userbuffer += copy_size;
		total_bytes_written += copy_size;
	}

	return total_bytes_written;
}

int em8300_fifo_freeslots(struct fifo_s *fifo)
{
	return ((*fifo->readptr - *fifo->writeptr) / fifo->slotptrsize + fifo->nslots - 1) % fifo->nslots;
}

void em8300_fifo_statusmsg(struct fifo_s *fifo, char *str)
{
	int freeslots = em8300_fifo_freeslots(fifo);
	sprintf(str,"Free slots: %d/%d", freeslots, fifo->nslots);
}

int em8300_fifo_calcbuffered(struct fifo_s *fifo)
{
	int readindex,writeindex,i,n;

	writeindex = (*fifo->writeptr - fifo->start) / fifo->slotptrsize;	
	readindex = (*fifo->readptr - fifo->start) / fifo->slotptrsize;
	n=0;
	i=readindex;
	while (i != writeindex) {
		switch (fifo->type) {
		case FIFOTYPE_AUDIO:
			n += fifo->slots.a[i].slotsize;
			break;
		case FIFOTYPE_VIDEO:
			n += fifo->slots.v[i].slotsize;
			break;
		}
		i++;
		i &= fifo->nslots-1;
	}

	return n;
}

int em8300_fifo_isempty(struct fifo_s *fifo)
{
	return !(*fifo->writeptr - *fifo->readptr);
}
