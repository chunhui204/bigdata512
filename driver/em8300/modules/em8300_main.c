/*
	em8300.c - EM8300 MPEG-2 decoder device driver

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
#include <linux/sound.h>
#include <linux/signal.h>
#include <linux/string.h>
#include <linux/time.h>
#include <asm/io.h>
#include <asm/pgtable.h>
#include <asm/page.h>
#include <linux/sched.h>
#include <asm/segment.h>
#include <asm/mtrr.h>

#include <linux/version.h>
#include <asm/uaccess.h>

#include <linux/i2c-algo-bit.h>

#include "encoder.h"

#include "em8300_reg.h"
#include <linux/em8300.h>
#include "em8300_fifo.h"

#ifndef I2C_BITBANGING
#error "This needs the I2C Bit Banging Interface in your Kernel"
#endif

MODULE_AUTHOR("Henrik Johansson <henrikjo@post.utfors.se>");
MODULE_DESCRIPTION("EM8300 MPEG-2 decoder");
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,3,0)
MODULE_PARM(remap,"1-" __MODULE_STRING(EM8300_MAX) "i");
#endif

#ifdef MODULE
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,4,9)
MODULE_LICENSE("GPL");
#endif
#endif

static unsigned int use_bt865[EM8300_MAX]={};
MODULE_PARM(use_bt865,"1-" __MODULE_STRING(EM8300_MAX) "i");

/*
 * Module params by Jonas Birmé (birme@jpl.nu)
 */
int dicom_other_pal = 1;
MODULE_PARM(dicom_other_pal, "i");

int dicom_fix = 1;
MODULE_PARM(dicom_fix, "i");

int dicom_control = 1;
MODULE_PARM(dicom_control, "i");

int bt865_ucode_timeout = 0;
MODULE_PARM(bt865_ucode_timeout, "i");

int activate_loopback = 0;
MODULE_PARM(activate_loopback, "i");

static int em8300_cards,clients;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,3,0)
static unsigned int remap[EM8300_MAX]={};
#endif
static struct em8300_s em8300[EM8300_MAX];
#ifdef REGISTER_DSP
static int dsp_num_table[16];
#endif

static void em8300_irq(int irq, void *dev_id, struct pt_regs * regs)
{
	struct em8300_s *em = (struct em8300_s *)dev_id;
	int irqstatus;
	struct timeval tv;

	irqstatus = read_ucregister(Q_IrqStatus);

	if (irqstatus & 0x8000) {
		write_ucregister(Q_IrqMask, 0x0);
		em->mem[EM8300_INTERRUPT_ACK] = 2;

		write_ucregister(Q_IrqStatus, 0x8000);

		if (irqstatus & IRQSTATUS_VIDEO_FIFO) {
			em8300_fifo_check(em->mvfifo);
		}
	
		if (irqstatus & IRQSTATUS_AUDIO_FIFO) {
			em8300_fifo_check(em->mafifo);
		}

		if (irqstatus & IRQSTATUS_VIDEO_VBL) {
			em8300_video_check_ptsfifo(em);
			em8300_spu_check_ptsfifo(em);

			do_gettimeofday(&tv);
			em->irqtimediff = TIMEDIFF(tv, em->tv);
			em->tv = tv;
			em->irqcount++;
		}
	
		write_ucregister(Q_IrqMask, em->irqmask);
		write_ucregister(Q_IrqStatus, 0x0000);
	}
}

static void release_em8300(int max)
{
	struct em8300_s *em;
	int i;

	for (i=0; i<max; i++) {
		em = &em8300[i];

		if(em->encoder) {
			em->encoder->driver->command(em->encoder, ENCODER_CMD_ENABLEOUTPUT, (void *)0);
		}
	
		if (em->mtrr_reg) {
			mtrr_del(em->mtrr_reg,em->adr, em->memsize);
		}
		
		em8300_i2c_exit(em);

		write_ucregister(Q_IrqMask, 0);
		write_ucregister(Q_IrqStatus, 0);
		em->mem[0x2000]=0;
	
		em8300_fifo_free(em->mvfifo);
		em8300_fifo_free(em->mafifo);
		em8300_fifo_free(em->spfifo);
	
		/* free it */
		free_irq(em->dev->irq, em);
	
		/* unmap and free memory */
		if (em->mem) {
			iounmap((unsigned *)em->mem);
		}
	}
}

static int find_em8300(void)
{
	struct pci_dev *dev = NULL;
	unsigned char revision;
	struct em8300_s *em;
	int em8300_n=0;
	int result;
	 
	if (!pcibios_present()) {
		printk(KERN_ERR "em8300: PCI-BIOS not present or not accessible!\n");
		return -1;
	}
	
	while ((dev = pci_find_device(PCI_VENDOR_ID_SIGMADESIGNS, PCI_DEVICE_ID_SIGMADESIGNS_EM8300, dev))) {
		em = &em8300[em8300_n];
		em->dev= dev;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,3,0)
		em->adr = dev->base_address[0];
		if (remap[em8300_n]) {
			uint dw;

			if (remap[em8300_n] < 0x1000) {
				remap[em8300_n]<<=20;
			}
			remap[em8300_n] &= PCI_BASE_ADDRESS_MEM_MASK;
			pr_info("Remapping to : 0x%08x.\n", remap[em8300_n]);
			remap[em8300_n] |= em->adr&(~PCI_BASE_ADDRESS_MEM_MASK);
			pci_write_config_dword(dev, PCI_BASE_ADDRESS_0, remap[em8300_n]);
			/* commit to PCI bus */
			pci_read_config_dword(dev,  PCI_BASE_ADDRESS_0, &dw);
			dev->base_address[0] = em->adr;
		}
		em->adr &= PCI_BASE_ADDRESS_MEM_MASK;
#else
		em->adr = dev->resource[0].start;
#endif	
		em->memsize = 1024*1024;
	
		pci_read_config_byte(dev, PCI_CLASS_REVISION, &revision);
		em->pci_revision = revision;
		pr_info("em8300: EM8300 %x (rev %d) ", dev->device, revision);
		printk("bus: %d, devfn: %d, irq: %d, ", dev->bus->number, dev->devfn, dev->irq);
		printk("memory: 0x%08lx.\n", em->adr);
	
		em->mem = ioremap(em->adr, em->memsize);
		pr_info("em8300: mapped-memory at 0x%p\n",em->mem);
		em->mtrr_reg = mtrr_add( em->adr, em->memsize, MTRR_TYPE_UNCACHABLE, 1 );
		if( em->mtrr_reg ) pr_info("em8300: using MTRR\n");

		result = request_irq(dev->irq, em8300_irq, SA_SHIRQ|SA_INTERRUPT,"em8300",(void *)em);
	
		if (result==-EINVAL) {
			printk(KERN_ERR "em8300: Bad irq number or handler\n");
			return -EINVAL;
		}	
	
		pci_set_master(dev);
	
		em8300_n++;
	}

	if (em8300_n) {
		pr_info("em8300: %d EM8300 card(s) found.\n", em8300_n);
	}
  
	return em8300_n;
}

static int em8300_io_ioctl(struct inode* inode, struct file* filp, unsigned int cmd, unsigned long arg)
{
	struct em8300_s *em = filp->private_data;
	int subdevice = MINOR(inode->i_rdev) & 3;

	switch (subdevice) {
	case EM8300_SUBDEVICE_AUDIO:
		return em8300_audio_ioctl(em, cmd, arg);
	case EM8300_SUBDEVICE_VIDEO:
		return em8300_video_ioctl(em, cmd, arg);
	case EM8300_SUBDEVICE_SUBPICTURE:
		return em8300_spu_ioctl(em, cmd, arg);
	case EM8300_SUBDEVICE_CONTROL:
		return em8300_control_ioctl(em, cmd, arg);
	}

	return -EINVAL;
}

static int em8300_io_open(struct inode* inode, struct file* filp) 
{
	int card = MINOR(inode->i_rdev)/4;
	int subdevice = MINOR(inode->i_rdev) & 3;
	struct em8300_s *em = &em8300[card];
	int err=0;
  
	if (card >= em8300_cards) {
		return -ENODEV;
	}

	if (subdevice != EM8300_SUBDEVICE_CONTROL) {
		if (em8300[card].inuse[subdevice]) {
			return -EBUSY;
		}
	}
  
	filp->private_data = &em8300[card];

	switch (subdevice) {
	case EM8300_SUBDEVICE_CONTROL:
		break;
	case EM8300_SUBDEVICE_AUDIO:
		err =  em8300_audio_open(em);
		break;
	case EM8300_SUBDEVICE_VIDEO:
		if (!em->ucodeloaded) {
			return -ENODEV;
		}
		em8300_video_open(em);

		em8300_ioctl_enable_videoout(em, 1);

		em8300_video_setplaymode(em, EM8300_PLAYMODE_PLAY);
		break;
	case EM8300_SUBDEVICE_SUBPICTURE:
		if (!em->ucodeloaded) {
			return -ENODEV;
		}
		err = em8300_spu_open(em);
		break;
	default:
		return -ENODEV;
		break;
	}

	if (err) {
		return err;
	}
	
	em8300[card].inuse[subdevice]++;

	clients++;
	pr_info("em8300_main.o: Opening device %d, Clients:%d\n", subdevice, clients);
	MOD_INC_USE_COUNT;
  
	return(0);
}

static int em8300_io_write(struct file *file, const char * buf,	size_t count, loff_t *ppos)
{
	struct em8300_s *em = file->private_data;
	int subdevice = MINOR(file->f_dentry->d_inode->i_rdev) & 3;
	
	switch (subdevice) {
	case EM8300_SUBDEVICE_VIDEO:
		return em8300_video_write(em, buf, count, ppos);
		break;
	case EM8300_SUBDEVICE_AUDIO:
		return em8300_audio_write(em, buf, count, ppos);
		break;
	case EM8300_SUBDEVICE_SUBPICTURE:
		return em8300_spu_write(em, buf, count, ppos);
		break;
	default:
		return -EPERM;
	}

	
}

int em8300_io_mmap(struct file *file, struct vm_area_struct *vma)
{
	struct em8300_s *em = file->private_data;
	unsigned long size = vma->vm_end - vma->vm_start;
	int subdevice = MINOR(file->f_dentry->d_inode->i_rdev) & 3;

	if (subdevice != EM8300_SUBDEVICE_CONTROL) {
		return -EPERM;
	}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,3,0)
	switch ((unsigned long) vma->vm_offset) {
#else	
	switch (vma->vm_pgoff) {
#endif	
	case 0:
		if (size > em->memsize) {
			return -EINVAL;
		}
	
		remap_page_range(vma->vm_start, em->adr, vma->vm_end - vma->vm_start, vma->vm_page_prot);
		vma->vm_file = file;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0)
		file->f_dentry->d_inode->i_count++;
#else	
		atomic_inc(&file->f_dentry->d_inode->i_count);
#endif   
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

int em8300_io_release(struct inode* inode, struct file* filp) 
{
	struct em8300_s *em = filp->private_data;
	int subdevice = MINOR(inode->i_rdev) & 3;
	
	switch(subdevice) {
	case EM8300_SUBDEVICE_AUDIO:
		em8300_audio_release(em);
		break;
	case EM8300_SUBDEVICE_VIDEO:
		em8300_video_release(em);
		em8300_ioctl_enable_videoout(em, 0);	
		break;
	case EM8300_SUBDEVICE_SUBPICTURE:
		break;
	}
	
	em->inuse[subdevice]--;

	clients--;
	pr_info("em8300_main.o: Releasing device %d, clients:%d\n", subdevice, clients);
	MOD_DEC_USE_COUNT;

	return(0);
}
   
static struct file_operations em8300_fops = {
	write: em8300_io_write,
	ioctl: em8300_io_ioctl,
	mmap: em8300_io_mmap,
	open: em8300_io_open,
	release: em8300_io_release,
};

#ifdef REGISTER_DSP
static int em8300_dsp_ioctl(struct inode* inode, struct file* filp, unsigned int cmd, unsigned long arg)
{
	struct em8300_s *em = filp->private_data;
	return em8300_audio_ioctl(em, cmd, arg);
}

static int em8300_dsp_open(struct inode* inode, struct file* filp) 
{
	int dsp_num = ((MINOR(inode->i_rdev) >> 4) & 0x0f);
	int card = dsp_num_table[dsp_num]-1;
	int err=0;

        pr_debug("em8300: opening dsp %i for card %i\n", dsp_num, card);

	if (card < 0 || card >= em8300_cards) {
		return -ENODEV;
	}

        if (em8300[card].inuse[EM8300_SUBDEVICE_AUDIO]) {
		return -EBUSY;
	}
  
	filp->private_data = &em8300[card];

        err =  em8300_audio_open(&em8300[card]);

	if (err) {
		return err;
	}
	
	em8300[card].inuse[EM8300_SUBDEVICE_AUDIO]++;

	clients++;
	pr_info("em8300_main.o: Opening device %d, Clients:%d\n", EM8300_SUBDEVICE_AUDIO, clients);
	MOD_INC_USE_COUNT;
  
	return(0);
}

static int em8300_dsp_write(struct file *file, const char * buf, size_t count, loff_t *ppos)
{
	struct em8300_s *em = file->private_data;
	return em8300_audio_write(em, buf, count, ppos);
}

int em8300_dsp_release(struct inode* inode, struct file* filp) 
{
	struct em8300_s *em = filp->private_data;
	
	em8300_audio_release(em);
	
	em->inuse[EM8300_SUBDEVICE_AUDIO]--;

	clients--;
	pr_info("em8300_main.o: Releasing device %d, clients:%d\n", EM8300_SUBDEVICE_AUDIO, clients);
	MOD_DEC_USE_COUNT;

	return(0);
}

static struct file_operations em8300_dsp_audio_fops = {
	write: em8300_dsp_write,
	ioctl: em8300_dsp_ioctl,
	open: em8300_dsp_open,
	release: em8300_dsp_release,
};
#endif

void cleanup_module(void)
{
#ifdef REGISTER_DSP
	int card;
	for (card = 0; card < em8300_cards; card++) {
		unregister_sound_dsp(em8300[card].dsp_num);
	}
#endif
	unregister_chrdev(EM8300_MAJOR, EM8300_LOGNAME);
	release_em8300(em8300_cards);
}

int em8300_init(struct em8300_s *em) {
	/* Setup parameters */
	static unsigned int *bt = use_bt865; 
    
	write_register(0x30000, read_register(0x30000));

	write_register(0x1f50, 0x123);

	if (read_register(0x1f50) == 0x123) {
		em->chip_revision = 2;
		if (0x40 & read_register(0x1c08)) {
			em->var_video_value = 3375; /* was 0xd34 = 3380 */
			em->mystery_divisor = 0x107ac;
			em->var_ucode_reg2 = 0x272;
			em->var_ucode_reg3 = 0x8272;
			if (0x20 & read_register(0x1c08)) {
				if(*bt) {
					em->var_ucode_reg1 = 0x800;
				} else {
					em->var_ucode_reg1 = 0x818;
				}
			}
		} else {
			em->var_video_value = 0xce4;
			em->mystery_divisor = 0x101d0;
			em->var_ucode_reg2 = 0x25a;
			em->var_ucode_reg3 = 0x825a;
		}
	} else {
		em->chip_revision = 1;            
		em->var_ucode_reg1 = 0x80;
		em->var_video_value = 0xce4;
		em->mystery_divisor = 0x101d0;
		em->var_ucode_reg2 = 0xC7;
		em->var_ucode_reg3 = 0x8c7;
	}

	pr_info("em8300_main.o: Chip revision: %d\n", em->chip_revision);
	pr_debug("em8300_main.o: use_bt865: %d\n", *bt);
	em8300_i2c_init(em);

	bt++;

	if (activate_loopback==0) {
		em->clockgen_tvmode=CLOCKGEN_TVMODE_1;
		em->clockgen_overlaymode=CLOCKGEN_OVERLAYMODE_1;
	} else {
		em->clockgen_tvmode=CLOCKGEN_TVMODE_2;
		em->clockgen_overlaymode=CLOCKGEN_OVERLAYMODE_2;
	}

	pr_debug("em8300_main.o: activate_loopback: %d\n", activate_loopback);

	return 0;
}

int init_module(void)
{
	int card;
	struct em8300_s *em = NULL;

	memset(&em8300, 0, sizeof(em8300));
#ifdef REGISTER_DSP
	memset(&dsp_num_table, 0, sizeof(dsp_num_table));
#endif

	/* Find EM8300 cards */
	em8300_cards = find_em8300();

	/* Initialize EM8300 cards */
	for (card = 0; card < em8300_cards; card++) {
		em = &em8300[card];
		
		em->irqmask = 0;
		
		em->encoder = NULL;
		em->eeprom = NULL;
		
		em->linecounter=0;
		
		em8300_init(em);

#ifdef REGISTER_DSP
		if ((em8300[card].dsp_num = register_sound_dsp(&em8300_dsp_audio_fops, -1)) < 0) {
			printk(KERN_ERR "e8300: cannot register oss audio device!\n");
			goto err_audio_dsp;
		}
		dsp_num_table[em8300[card].dsp_num >> 4 & 0x0f] = card+1;
	        pr_debug("em8300: registered dsp %i for device %i\n", em8300[card].dsp_num >> 4 & 0x0f, card);
#endif
	}

	if (register_chrdev(EM8300_MAJOR, EM8300_LOGNAME, &em8300_fops)) {
		printk(KERN_ERR "em8300: unable to get major %d\n", EM8300_MAJOR);
		goto err_chrdev;
	}
    
	return 0;

#ifdef REGISTER_DSP
 err_audio_dsp:
#endif
 err_chrdev:
#ifdef REGISTER_DSP
	while (card-- > 0) {
		unregister_sound_dsp(em[card].dsp_num);
	}
#endif
	release_em8300(em8300_cards);
	return -ENODEV;
}
