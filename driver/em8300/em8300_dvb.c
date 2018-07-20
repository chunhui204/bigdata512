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

#include "modules/encoder.h"

#include "modules/em8300_reg.h"
#include <linux/em8300.h>
#include "modules/em8300_fifo.h"

#ifndef I2C_BITBANGING
#warning "This needs the I2C Bit Banging Interface in your Kernel"
#endif

MODULE_AUTHOR("author");
MODULE_DESCRIPTION("EM8300 MPEG-2 decoder");

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
static struct dvb_stuff dvbs[EM8300_MAX];
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
#ifdef USE_DVB
		dvb_unregister(&dvbs[i]);
#endif

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

		em->adr = dev->resource[0].start;
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
	pr_info("em8300_dvb.o: Opening device %d, Clients:%d\n", subdevice, clients);
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

	switch (vma->vm_pgoff) {
	case 0:
		if (size > em->memsize) {
			return -EINVAL;
		}
	
		remap_page_range(vma->vm_start, em->adr, vma->vm_end - vma->vm_start, vma->vm_page_prot);
		vma->vm_file = file;

		atomic_inc(&file->f_dentry->d_inode->i_count);
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
	pr_info("em8300_dvb.o: Releasing device %d, clients:%d\n", subdevice, clients);
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
	pr_info("em8300_dvb.o: Opening device %d, Clients:%d\n", EM8300_SUBDEVICE_AUDIO, clients);
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
	pr_info("em8300_dvb.o: Releasing device %d, clients:%d\n", EM8300_SUBDEVICE_AUDIO, clients);
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

	pr_info("em8300_dvb.o: Chip revision: %d\n", em->chip_revision);
	pr_debug("em8300_dvb.o: use_bt865: %d\n", *bt);
	em8300_i2c_init(em);

	bt++;

	if (activate_loopback==0) {
		em->clockgen_tvmode=CLOCKGEN_TVMODE_1;
		em->clockgen_overlaymode=CLOCKGEN_OVERLAYMODE_1;
	} else {
		em->clockgen_tvmode=CLOCKGEN_TVMODE_2;
		em->clockgen_overlaymode=CLOCKGEN_OVERLAYMODE_2;
	}

	pr_debug("em8300_dvb.o: activate_loopback: %d\n", activate_loopback);

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

#ifdef USE_DVB
		dvbs[card].em8300 = &em8300[card];
		dvb_register(&dvbs[card]);
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


#ifdef USE_DVB

static inline int

num2type(struct dvb_stuff *card, int num)
{
        if (!card->dvb_devs)
                return -2;
        if (num>=card->dvb_devs->num)
                return -2;
        return card->dvb_devs->tab[num];
}


void em_write (uint8_t *buf, int count, void *data)
{
	int written,c;
        struct dvb_stuff *card=(struct dvb_stuff *) data;
	struct em8300_s  *em = card->em8300;
	uint8_t *pts;
	uint8_t *b;


	switch(buf[3]){
	case PRIVATE_STREAM1:
		break;
	case AUDIO_STREAM_S ... AUDIO_STREAM_E:
		break;
	case VIDEO_STREAM_S ... VIDEO_STREAM_E:
		written = 0;
		b = buf;
		pts = skip_pes_header(&b);
		count -= b - buf;
		while (written < count){
			if ((c = em8300_video_write(em, b+written, 
						    count-written, 0)) > 0){
				written += c;
			}
			
		}
		break;
	default:
		break;
	}
}

static int dvb_video_open(struct inode *inode, struct file *file)
{
        dvb_device_t *dvbdev=*(dvb_device_t **) file->private_data;
        struct dvb_stuff *card=(struct dvb_stuff *) dvbdev->priv;
	struct em8300_s  *em = card->em8300;
	int err;
        int ret=0;
	int subdevice1 = -1;
	int subdevice2 = -1;
        if ((err=dvb_generic_open(inode, file))<0)
                return err;

	card->video_blank=true;
	card->audiostate.AV_sync_state=true;
	card->videostate.stream_source=VIDEO_SOURCE_DEMUX;
	if (!em->ucodeloaded) {
		return -ENODEV;
	}
	em8300_video_open(em);

	em8300_ioctl_enable_videoout(em, 1);

	em8300_video_setplaymode(em, EM8300_PLAYMODE_PLAY);
	ret = em8300_spu_open(em);
	subdevice1 = EM8300_SUBDEVICE_VIDEO;
	subdevice2 = EM8300_SUBDEVICE_SUBPICTURE;
	init_ipack(&card->pesv, IPACKS, em_write);
	card->pesv.data = (void *)card;
	printk(KERN_ERR "em8300_dvb: video open\n");

	clients++;
	if (subdevice1 >= 0) em->inuse[subdevice1]++;
	if (subdevice2 >= 0) em->inuse[subdevice2]++;

        return ret;
}


static int dvb_audio_open(struct inode *inode, struct file *file)
{
        dvb_device_t *dvbdev=*(dvb_device_t **) file->private_data;
        struct dvb_stuff *card=(struct dvb_stuff *) dvbdev->priv;
	struct em8300_s  *em = card->em8300;
	int err;	
        int ret=0;
	int subdevice1 = -1;
	int subdevice2 = -1;
        if ((err=dvb_generic_open(inode, file))<0)
                return err;

	card->audiostate.AV_sync_state=true;
	card->audiostate.stream_source=AUDIO_SOURCE_DEMUX;
	ret =  em8300_audio_open(em);
	subdevice1 = EM8300_SUBDEVICE_AUDIO;
	subdevice2 = EM8300_SUBDEVICE_SUBPICTURE;
	init_ipack(&card->pesa, IPACKS, em_write);
	card->pesa.data = (void *)card;

	clients++;
	if (subdevice1 >= 0) em->inuse[subdevice1]++;
	if (subdevice2 >= 0) em->inuse[subdevice2]++;

        return ret;
}

static int dvb_video_release(struct inode *inode, struct file *file)
{
        dvb_device_t *dvbdev=*(dvb_device_t **) file->private_data;
        struct dvb_stuff *card=(struct dvb_stuff *) dvbdev->priv;
	struct em8300_s  *em = card->em8300;
	int subdevice1 = -1;
	int subdevice2 = -1;

	em8300_video_release(em);
	subdevice1 = EM8300_SUBDEVICE_VIDEO;
	subdevice2 = EM8300_SUBDEVICE_SUBPICTURE;
	free_ipack(&card->pesv);
	if (subdevice1 >= 0) em->inuse[subdevice1]--;
	if (subdevice2 >= 0) em->inuse[subdevice2]--;

	clients--;

        return dvb_generic_release(inode, file);
}

static int dvb_audio_release(struct inode *inode, struct file *file)
{
        dvb_device_t *dvbdev=*(dvb_device_t **) file->private_data;
        struct dvb_stuff *card=(struct dvb_stuff *) dvbdev->priv;
	struct em8300_s  *em = card->em8300;
	int subdevice1 = -1;

	em8300_audio_release(em);
	subdevice1 = EM8300_SUBDEVICE_AUDIO;
	free_ipack(&card->pesa);

	if (subdevice1 >= 0) em->inuse[subdevice1]--;
	clients--;
	
        return dvb_generic_release(inode, file);
}


static ssize_t 
dvb_video_write(struct file *file, const char *buf, 
                size_t count, loff_t *ppos)
{
        dvb_device_t *dvbdev=*(dvb_device_t **) file->private_data;
        struct dvb_stuff *card=(struct dvb_stuff *) dvbdev->priv;

	if (card->videostate.stream_source!=VIDEO_SOURCE_MEMORY)
		return -EPERM;
	instant_repack( (char *)buf, count,&card->pesv);
	return count;
}

static ssize_t 
dvb_audio_write(struct file *file, const char *buf, 
                size_t count, loff_t *ppos)
{
        dvb_device_t *dvbdev=*(dvb_device_t **) file->private_data;
        struct dvb_stuff *card=(struct dvb_stuff *) dvbdev->priv;
  
	if (card->audiostate.stream_source!=AUDIO_SOURCE_MEMORY)
		return -EPERM;
	instant_repack( (char *)buf, count,&card->pesa);
	return count;
}


static int 
dvb_video_ioctl(struct inode *inode, struct file *file,
            unsigned int cmd, void *parg)
{
	dvb_device_t *dvbdev=*(dvb_device_t **) file->private_data;
        struct dvb_stuff *card=(struct dvb_stuff *) dvbdev->priv;
	struct em8300_s  *em = card->em8300;
        unsigned long arg=(unsigned long) parg;

	if (((file->f_flags&O_ACCMODE)==O_RDONLY) &&
	    (cmd!=VIDEO_GET_STATUS))
		return -EPERM;
	
	switch (cmd) {
	case VIDEO_STOP:
		card->videostate.play_state = VIDEO_STOPPED;
		return em8300_video_setplaymode(em, EM8300_PLAYMODE_STOPPED);
		break;
	
	case VIDEO_PLAY:
		if (card->videostate.stream_source==
		    VIDEO_SOURCE_MEMORY) {
			card->videostate.play_state=VIDEO_PLAYING;
			return em8300_video_setplaymode(em, EM8300_PLAYMODE_PLAY);
		}
		break;

	case VIDEO_FREEZE:
		card->videostate.play_state=VIDEO_FREEZED;
		return em8300_video_setplaymode(em, EM8300_PLAYMODE_PAUSED);
		break;

	case VIDEO_CONTINUE:
		if (card->videostate.play_state==VIDEO_FREEZED) {
			card->videostate.play_state=VIDEO_PLAYING;
			return em8300_video_setplaymode(em, EM8300_PLAYMODE_PLAY);
		} 
		break;

	case VIDEO_SELECT_SOURCE:
		card->videostate.stream_source=(video_stream_source_t) parg;
		break;

	case VIDEO_SET_BLANK:
		card->videostate.video_blank=(boolean) arg;
		break;

	case VIDEO_GET_STATUS:
		if(copy_to_user(parg, &card->videostate, 
				sizeof(struct video_status)))
			return -EFAULT;
		break;

	case VIDEO_GET_EVENT:
		return -EOPNOTSUPP;

	case VIDEO_SET_DISPLAY_FORMAT:
	{
		video_displayformat_t format=(video_displayformat_t) arg;
		uint16_t val=0;
		
		switch(format) {
		case VIDEO_PAN_SCAN:
			val=VID_PAN_SCAN_PREF;
			break;
			
		case VIDEO_LETTER_BOX:
			val=VID_VC_AND_PS_PREF;
			break;
			
		case VIDEO_CENTER_CUT_OUT:
			val=VID_CENTRE_CUT_PREF;
			break;
			
		default:
			return -EINVAL;
		}
		
		card->videostate.video_format=format;
		return 0;
	}
                
	case VIDEO_STILLPICTURE:
	{ 
		struct video_still_picture pic;
		
		if(copy_from_user(&pic, parg, 
				  sizeof(struct video_still_picture)))
			return -EFAULT;
		
		break;
	}

	case VIDEO_FAST_FORWARD:
		if (card->videostate.stream_source !=
		    VIDEO_SOURCE_MEMORY)
			return -EPERM;
		
		break;
		
	case VIDEO_SLOWMOTION:
		if (card->videostate.stream_source!=VIDEO_SOURCE_MEMORY)
			return -EPERM;
		
		break;
		
	case VIDEO_GET_CAPABILITIES:
	{
		int cap=VIDEO_CAP_MPEG1|
			VIDEO_CAP_MPEG2|
			VIDEO_CAP_SPU|
			VIDEO_CAP_NAVI;
		
                        
		if (copy_to_user(parg, &cap, 
				 sizeof(cap)))
			return -EFAULT;
		break;
	}

	case VIDEO_SET_STREAMTYPE:
	{
		switch(arg){
		case VIDEO_CAP_MPEG1:
		case VIDEO_CAP_MPEG2:
		case VIDEO_CAP_SYS:
		case VIDEO_CAP_PROG:
		case VIDEO_CAP_SPU:
		case VIDEO_CAP_NAVI:
		case VIDEO_CAP_CSS:
		}   
	}			
	break;

	case VIDEO_SET_ID:
		break;
		
	case VIDEO_SET_SYSTEM:
		break;
		
	case VIDEO_SET_HIGHLIGHT:
		
	case VIDEO_SET_SPU:
		
	case VIDEO_SET_SPU_PALETTE:
		
	case VIDEO_GET_NAVI:
		
	case VIDEO_SET_ATTRIBUTES:

	default:
		return -ENOIOCTLCMD;
	}
	return 0;
}

static int 
dvb_audio_ioctl(struct inode *inode, struct file *file,
            unsigned int cmd, void *parg)
{
	dvb_device_t *dvbdev=*(dvb_device_t **) file->private_data;
	struct dvb_stuff *card=(struct dvb_stuff *) dvbdev->priv;
        unsigned long arg=(unsigned long) parg;

	if (((file->f_flags&O_ACCMODE)==O_RDONLY) &&
	    (cmd!=AUDIO_GET_STATUS))
		return -EPERM;

	switch (cmd) {

	case AUDIO_STOP:
		if (card->audiostate.stream_source!=AUDIO_SOURCE_MEMORY)
			break;
		card->audiostate.play_state=AUDIO_STOPPED;
		break;
		
	case AUDIO_PLAY:
		if (card->audiostate.stream_source!=AUDIO_SOURCE_MEMORY)
			break;
		card->audiostate.play_state=AUDIO_PLAYING;
		break;
		
	case AUDIO_PAUSE:
		card->audiostate.play_state=AUDIO_PAUSED;
		break;
		
	case AUDIO_CONTINUE:
		if (card->audiostate.play_state==AUDIO_PAUSED) {
			card->audiostate.play_state=AUDIO_PLAYING;
		} 
		break;
		
	case AUDIO_SELECT_SOURCE:
		card->audiostate.stream_source=
			(audio_stream_source_t) arg;
		break;
		
	case AUDIO_SET_MUTE:
	{
		card->audiostate.mute_state=(boolean) arg;
		break;
	}
	
	case AUDIO_SET_AV_SYNC:
		card->audiostate.AV_sync_state=(boolean) arg;
		break;

	case AUDIO_SET_BYPASS_MODE:
		return -EINVAL;
		
	case AUDIO_CHANNEL_SELECT:
		card->audiostate.channel_select=(audio_channel_select_t) arg;
		
		switch(card->audiostate.channel_select) {
		case AUDIO_STEREO:
			break;
			
		case AUDIO_MONO_LEFT:
			break;
			
		case AUDIO_MONO_RIGHT:
			break;
			
		default:
			return -EINVAL;
		}
		return 0;
		
	case AUDIO_GET_STATUS:
		if(copy_to_user(parg, &card->audiostate, 
				sizeof(struct audio_status)))
			return -EFAULT;
		break;
		
	case AUDIO_GET_CAPABILITIES:
	{
		int cap=AUDIO_CAP_LPCM|
			AUDIO_CAP_MP1|
			AUDIO_CAP_MP2|
			AUDIO_CAP_AC3;
		
		if (copy_to_user(parg, &cap, 
				 sizeof(cap)))
			return -EFAULT;
	}
	break;


	case AUDIO_SET_STREAMTYPE:
	{
		switch(arg){
		case AUDIO_CAP_DTS:
		case AUDIO_CAP_MP3:
		case AUDIO_CAP_AAC:
		case AUDIO_CAP_SDDS:
		case AUDIO_CAP_OGG:
		case AUDIO_CAP_LPCM:
		case AUDIO_CAP_MP1:
		case AUDIO_CAP_MP2:
		case AUDIO_CAP_AC3:
		}
			
		break;
	}

	case AUDIO_SET_ID:
		if (arg < 0 || arg >32) arg = 0;
	case AUDIO_SET_EXT_ID:
		if (arg < 0 || arg >32) arg = 0;
		break;
		
	case AUDIO_SET_MIXER:
		return -EINVAL;
		
	case AUDIO_SET_ATTRIBUTES:
		break;
		
		
	case AUDIO_SET_KARAOKE:
	{
		break;
	}
	
	default:
		return -ENOIOCTLCMD;
	}
	return 0;
}       

static unsigned int dvb_video_poll(struct file *file, poll_table *wait)
{
        dvb_device_t *dvbdev=*(dvb_device_t **) file->private_data;
	struct dvb_stuff *dvb=(struct dvb_stuff *) dvbdev->priv;
        unsigned int mask=0;

        return mask;
}

static unsigned int dvb_audio_poll(struct file *file, poll_table *wait)
{
        dvb_device_t *dvbdev=*(dvb_device_t **) file->private_data;
	struct dvb_stuff *dvb=(struct dvb_stuff *) dvbdev->priv;
        unsigned int mask=0;
                
        return mask;
}

/******************************************************************************
 * driver registration 
 ******************************************************************************/
static struct file_operations dvb_video_fops = {
        owner:          THIS_MODULE,
        read:           0,
        write:          dvb_video_write,
        ioctl:          dvb_generic_ioctl,
        open:           dvb_video_open,
        release:        dvb_video_release,
        poll:           dvb_video_poll,
};

static dvb_device_t dvbdev_video = {
        priv: 0,
        users: 1,
        writers: 1,
        fops: &dvb_video_fops,
        kernel_ioctl: dvb_video_ioctl,
};

static struct file_operations dvb_audio_fops = {
        owner:          THIS_MODULE,
        read:           0,
        write:          dvb_audio_write,
        ioctl:          dvb_generic_ioctl,
        open:           dvb_audio_open,
        release:        dvb_audio_release,
        poll:           dvb_audio_poll,
};

static dvb_device_t dvbdev_audio = {
        priv: 0,
        users: 1,
        writers: 1,
        fops: &dvb_audio_fops,
        kernel_ioctl: dvb_audio_ioctl,
};



void pes_write(uint8_t *buf, int count, void *priv)
{
	loff_t *ppos=0;
	uint8_t *pts;
	uint8_t *bufp = buf;

        struct dvb_stuff *card=(struct dvb_stuff *) priv;
	struct em8300_s  *em = card->em8300;

	switch(buf[3]){
        case VIDEO_STREAM_S ... VIDEO_STREAM_E:
		pts = skip_pes_header(&bufp);
		em8300_video_write(em, buf, count-(bufp-buf), ppos);
                break;
        case AUDIO_STREAM_S ... AUDIO_STREAM_E:                 
		pts = skip_pes_header(&bufp);
		em8300_video_write(em, buf, count-(bufp-buf), ppos);
                break;

        case PRIVATE_STREAM1:
	}
}



static int 
dvb_start_feed(dvb_demux_feed_t *dvbdmxfeed)
{
        dvb_demux_t *dvbdmx=dvbdmxfeed->demux;
        struct dvb_stuff *card=(struct dvb_stuff *) dvbdmx->priv;
 
        if (!dvbdmx->dmx.frontend || !card)
                return -EINVAL;
	
        if (dvbdmxfeed->type == DMX_TYPE_TS) {
	        if ((dvbdmxfeed->ts_type & TS_DECODER) 
		    && (dvbdmxfeed->pes_type<DMX_TS_PES_OTHER)) {
		        switch (dvbdmx->dmx.frontend->source) {
			case DMX_MEMORY_FE: 
			        if (dvbdmxfeed->ts_type & TS_DECODER)
				       if (dvbdmxfeed->pes_type<2 && 
                                           dvbdmx->pids[0]!=0xffff &&
					    dvbdmx->pids[1]!=0xffff) {
					       
					       setup_ts2pes( &card->tsa, 
							     &card->tsv,
							     dvbdmx->pids,
							     dvbdmx->pids+1, 
							     pes_write,
							     (void *)card);

					       dvbdmx->playing=1;
				       }
				break;
			default:
				return -EINVAL;
				break;
			}
		} 
        }
        
        if (dvbdmxfeed->type == DMX_TYPE_SEC) {
                int i;

	        for (i=0; i<dvbdmx->filternum; i++) {
		        if (dvbdmx->filter[i].state!=DMX_STATE_READY)
			        continue;
			if (dvbdmx->filter[i].type!=DMX_TYPE_SEC)
			        continue;
			if (dvbdmx->filter[i].filter.parent!=
			    &dvbdmxfeed->feed.sec)
			        continue;

			dvbdmxfeed->feed.sec.is_filtering=1;
			dvbdmx->filter[i].state=DMX_STATE_GO;
                }
	}

        return 0;
}


static int 
dvb_stop_feed(dvb_demux_feed_t *dvbdmxfeed)
{
        dvb_demux_t *dvbdmx=dvbdmxfeed->demux;
        struct dvb_stuff *card=(struct dvb_stuff *) dvbdmx->priv;

        if (!card)
                return -EINVAL;

        if (dvbdmxfeed->type == DMX_TYPE_TS) {
		if ((dvbdmxfeed->ts_type & TS_DECODER) 
		    && (dvbdmxfeed->pes_type<=1)) {
			if (dvbdmx->playing) {
				free_ipack(&card->tsa);
				free_ipack(&card->tsv);
				dvbdmx->playing=0;
			}
		} 

	}
        if (dvbdmxfeed->type == DMX_TYPE_SEC) {
                int i;
		
	        for (i=0; i<dvbdmx->filternum; i++)
		        if (dvbdmx->filter[i].state==DMX_STATE_GO && 
			    dvbdmx->filter[i].filter.parent==
			    &dvbdmxfeed->feed.sec) {
			        dvbdmx->filter[i].state=DMX_STATE_READY;
                }
		
	}
        return 0;
}


static int 
dvb_write_to_decoder(dvb_demux_feed_t *dvbdmxfeed, uint8_t *buf, size_t count)
{
        dvb_demux_t *dvbdmx=dvbdmxfeed->demux;
        struct dvb_stuff *card=(struct dvb_stuff *) dvbdmx->priv;

	uint16_t pid = 0;
	int off = 0;

	ipack *p;

        if (!card)
                return -EINVAL;
	
//	pid = get_pid(buf+1);
			
	if (pid == *(card->tsa.pid)) p = &(card->tsa);
	else if (pid == *(card->tsv.pid)) p = &(card->tsv);
	else return 0;

        if (dvbdmxfeed->pes_type>1)
                return -1;
        if (!(buf[3]&0x10)) // no payload?
                return -1;

	if (count != TS_SIZE) return -1;

	if ( buf[3] & ADAPT_FIELD) {  // adaptation field?
		off = buf[4] + 1;
	}
	

	if (pid == *(card->tsa.pid)){
		/* FIX ME */
	} else {
		/* FIX ME */
	}

        return 0;
}

int dvb_register(struct dvb_stuff *card)
{
        int ret;
        
        struct dvb_device *dvbd=&card->dvb_dev;
	
	dvb_demux_t *dvbdemux = (dvb_demux_t *)&card->demux;

        if (card->dvb_registered)
                return -1;

	if (dvb_register_adapter(&card->dvb_adapter, "EM8300")<0)
		return -1;

	card->dvb_registered=1;

        card->audiostate.AV_sync_state=0;
        card->audiostate.mute_state=0;
        card->audiostate.play_state=AUDIO_STOPPED;
        card->audiostate.stream_source=AUDIO_SOURCE_MEMORY;
        card->audiostate.channel_select=AUDIO_STEREO;
        card->audiostate.bypass_mode=0;

        card->videostate.video_blank=0;
        card->videostate.play_state=VIDEO_STOPPED;
        card->videostate.stream_source=VIDEO_SOURCE_MEMORY;
        card->videostate.video_format=VIDEO_FORMAT_4_3;
        card->videostate.display_format=VIDEO_CENTER_CUT_OUT;

        // init and register demuxes
	memcpy(card->demux_id, "demux0_0", 9);
        card->demux_id[7] = 1+0x30;
        dvbdemux->priv = (void *) card;
	dvbdemux->filternum = 256;
	dvbdemux->feednum = 256;
	dvbdemux->start_feed = dvb_start_feed;
	dvbdemux->stop_feed = dvb_stop_feed;
	dvbdemux->write_to_decoder = dvb_write_to_decoder;
                
	dvbdemux->dmx.vendor="CIM";
	dvbdemux->dmx.model="sw";
	dvbdemux->dmx.id=card->demux_id;
	dvbdemux->dmx.capabilities=(DMX_TS_FILTERING|
				    DMX_SECTION_FILTERING|
				    DMX_MEMORY_BASED_FILTERING);
	
	DvbDmxInit(&card->demux);

	card->dmxdev.filternum=256;
	card->dmxdev.demux=&dvbdemux->dmx;
	card->dmxdev.capabilities=0;

	DmxDevInit(&card->dmxdev, card->dvb_adapter);
        
        card->mem_frontend.id="mem_frontend";
        card->mem_frontend.vendor="memory";
        card->mem_frontend.model="sw";
        card->mem_frontend.source=DMX_MEMORY_FE;
        ret=dvbdemux->dmx.add_frontend(&dvbdemux->dmx, 
                                        &card->mem_frontend);
        if (ret<0)
                return ret;
	ret=dvbdemux->dmx.connect_frontend(&dvbdemux->dmx, 
					   &card->mem_frontend);
        if (ret<0)
                return ret;


	dvb_register_device(card->dvb_adapter, &card->video_dev, 
			    &dvbdev_video, card, DVB_DEVICE_VIDEO);
	dvb_register_device(card->dvb_adapter, &card->audio_dev, 
			    &dvbdev_audio, card, DVB_DEVICE_AUDIO);

        // init and register dvb device structure
        dvbd->priv=(void *) card;
	return 0;
}

static void 
dvb_unregister(struct dvb_stuff *dvb)
{
        dvb_demux_t *dvbdemux=&dvb->demux;

        if (!dvb->dvb_registered)
                return;
        dvbdemux->dmx.close(&dvbdemux->dmx);
        dvbdemux->dmx.remove_frontend(&dvbdemux->dmx, &dvb->mem_frontend);

        DmxDevRelease(&dvb->dmxdev);
        DvbDmxRelease(&dvb->demux);

	dvb_unregister_device(dvb->audio_dev);
	dvb_unregister_device(dvb->video_dev);
        dvb_unregister_adapter(dvb->dvb_adapter);
}

#endif
