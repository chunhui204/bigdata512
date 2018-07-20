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
#include "em8300_fifo.h"

#include <linux/soundcard.h>

extern int bt865_ucode_timeout;

static int mpegvideo_command(struct em8300_s *em, int cmd)
{
	if (em8300_waitfor(em,ucregister(MV_Command), 0xffff, 0xffff)) {
		return -1;
	}

	write_ucregister(MV_Command, cmd);
	
	if ((cmd == MVCOMMAND_DISPLAYBUFINFO) || (cmd == 0x10)) {
		return em8300_waitfor_not(em, ucregister(DICOM_Display_Data), 0, 0xffff);
	} else {
		return em8300_waitfor(em,ucregister(MV_Command), 0xffff, 0xffff);
	}
}

int em8300_video_setplaymode(struct em8300_s *em, int mode)
{
	if (mode == EM8300_PLAYMODE_FRAMEBUF) {
		mpegvideo_command(em, MVCOMMAND_DISPLAYBUFINFO);
		em->video_playmode=mode;
		return 0;
	}
    
	if (em->video_playmode != mode) {
		switch(mode) {
		case EM8300_PLAYMODE_STOPPED:
			em->video_ptsfifo_ptr = 0;
			em->video_offset = 0;
			mpegvideo_command(em, MVCOMMAND_STOP);
			break;
		case EM8300_PLAYMODE_PLAY:
			em->video_pts = 0;
			em->video_lastpts = 0;
			if (em->video_playmode == EM8300_PLAYMODE_STOPPED) {
				write_ucregister(MV_FrameEventLo, 0xffff);
				write_ucregister(MV_FrameEventHi, 0x7fff);
			}
			mpegvideo_command(em, MVCOMMAND_START);
			break;
		case EM8300_PLAYMODE_PAUSED:
			mpegvideo_command(em, MVCOMMAND_PAUSE);
			break;
		default:
			return -1;
		}

		em->video_playmode=mode;

		return 0;
	}

	return -1;
}

int em8300_video_sync(struct em8300_s *em)
{
	int rdptr,wrptr,synctimeout;

	synctimeout = 0;
    
	do {
		wrptr = read_ucregister(MV_Wrptr_Lo) |
			(read_ucregister(MV_Wrptr_Hi)<<16);
		rdptr = read_ucregister(MV_RdPtr_Lo) |
			(read_ucregister(MV_RdPtr_Hi)<<16);

		if (rdptr != wrptr && ++synctimeout < 20) {
			schedule_timeout(HZ/10);
		}

		if (signal_pending(current)) {
			printk(KERN_ERR "em8300_video.o: Video sync interrupted\n");
			return -EINTR;
		}

	} while (rdptr != wrptr && synctimeout < 20);
	
	return 0;
}

void set_dicom_kmin (struct em8300_s *em)
{
	int kmin;

	kmin = (em->overlay_70 + 10) * 150645 / em->mystery_divisor;
	if (kmin > 0x900) {
		kmin = 0x900;
	}
	write_ucregister(DICOM_Kmin, kmin);
	pr_debug("em8300: register DICOM_Kmin = 0x%x\n", kmin);
}

int em8300_video_setup(struct em8300_s *em)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,3,0)
	init_waitqueue(&em->video_ptsfifo_wait);
#else
	init_waitqueue_head(&em->video_ptsfifo_wait);
#endif	

	write_register(0x1f47, 0x0);

	if (em->encoder_type == ENCODER_BT865) {
		write_register(0x1f5e, 0x9efe);
		write_ucregister(DICOM_Control, 0x9efe);
	} else {
		write_register(0x1f5e, 0x9afe);
		write_ucregister(DICOM_Control, 0x9afe);
	}

	write_register(EM8300_I2C_PIN, 0x3c3c);
	write_register(EM8300_I2C_OE, 0x3c00);
	write_register(EM8300_I2C_OE, 0x3c3c);
	
	write_register(EM8300_I2C_PIN, 0x808);
	write_register(EM8300_I2C_PIN, 0x1010);
	
	em9010_init(em);
	
	em9010_write(em, 7, 0x40);
	em9010_write(em, 9, 0x4);
	
	em9010_read(em, 0);
	
	udelay(100);
	
	write_ucregister(DICOM_UpdateFlag, 0x0);
	
	write_ucregister(DICOM_VisibleLeft, 0x168);
	write_ucregister(DICOM_VisibleTop, 0x2e);
	write_ucregister(DICOM_VisibleRight, 0x36b);
	write_ucregister(DICOM_VisibleBottom, 0x11e);
	write_ucregister(DICOM_FrameLeft, 0x168);
	write_ucregister(DICOM_FrameTop, 0x2e);
	write_ucregister(DICOM_FrameRight, 0x36b);
	write_ucregister(DICOM_FrameBottom, 0x11e);
	em8300_dicom_enable(em);

	em9010_write16(em, 0x8, 0xff);
	em9010_write16(em, 0x10, 0xff);
	em9010_write16(em, 0x20, 0xff);

	em9010_write(em, 0xa, 0x0);

	if (em9010_cabledetect(em)) {
		pr_debug("em8300: overlay loop-back cable detected\n");
	}
    
	pr_debug("em8300: overlay reg 0x80 = %x \n", em9010_read16(em, 0x80));
	
	em9010_write(em, 0xb, 0xc8);
	
	pr_debug("em8300: register 0x1f4b = %x (0x138)\n", read_register(0x1f4b));

	em9010_write16(em, 1, 0x4fe);
	em9010_write(em, 1, 4);
	em9010_write(em, 5, 0);
	em9010_write(em, 6, 0);
	em9010_write(em, 7, 0x40);
	em9010_write(em, 8, 0x80);
	em9010_write(em, 0xc, 0x8c);
	em9010_write(em, 9, 0);

	set_dicom_kmin(em);

	em9010_write(em, 7, 0x80);
	em9010_write(em, 9, 0);

	if (bt865_ucode_timeout) {
		write_register(0x1f47, 0x18);
	}
	if (em->encoder_type == ENCODER_BT865) {
		write_register(0x1f5e, 0x9efe);
		write_ucregister(DICOM_Control, 0x9efe);
	} else {
		if (!bt865_ucode_timeout) {
			write_register(0x1f47, 0x18);
		}
		write_register(0x1f5e,0x9afe);
		write_ucregister(DICOM_Control, 0x9afe);
	}

	write_ucregister(DICOM_UpdateFlag, 0x1);
	
	udelay(100);
	
	write_ucregister(ForcedLeftParity, 0x2);

	write_ucregister(MV_Threshold, 0x90); // was 0x50 for BT865, but this works too

	write_register(EM8300_INTERRUPT_ACK, 0x2);
	write_ucregister(Q_IrqMask, 0x0);
	write_ucregister(Q_IrqStatus, 0x0);
	write_ucregister(Q_IntCnt, 0x64);

	write_register(0x1ffb, em->var_video_value);

	write_ucregister(MA_Threshold, 0x8);
	
	/* Release reset */
	write_register(0x2000, 0x1);
	
	if (mpegvideo_command(em, MVCOMMAND_DISPLAYBUFINFO)) {
		printk(KERN_ERR "em8300_video: mpegvideo_command(0x11) failed\n");
		return -ETIME;
	}
	em8300_dicom_get_dbufinfo(em);
        
	write_ucregister(SP_Status, 0x0);
	
	if(mpegvideo_command(em, 0x10)) {
		printk(KERN_ERR "em8300: mpegvideo_command(0x10) failed\n");
		return -ETIME;
	}

	em8300_video_setspeed(em, 0x900);

	write_ucregister(MV_FrameEventLo, 0xffff);
	write_ucregister(MV_FrameEventHi, 0x7fff);

	em8300_ioctl_setvideomode(em, EM8300_VIDEOMODE_DEFAULT);
	em8300_ioctl_setaspectratio(em, EM8300_ASPECTRATIO_4_3);

	em8300_dicom_setBCS(em, 500, 500, 500);

	if (em8300_dicom_update(em)) {
		printk(KERN_ERR "em8300: DICOM Update failed\n");
		return -ETIME;
	}

	em->video_playmode = -1;
	em8300_video_setplaymode(em, EM8300_PLAYMODE_STOPPED);

	return 0;
}

void em8300_video_setspeed(struct em8300_s *em, int speed)
{
	if (read_ucregister(MicroCodeVersion) >= 0x29) {
		write_ucregister(MV_SCRSpeed, speed);
	} else {
		write_ucregister(MV_SCRSpeed, speed >> 8);
	}
}

void em8300_video_check_ptsfifo(struct em8300_s *em)
{
	int ptsfifoptr;
    
	if (em->video_ptsfifo_waiting) {

		ptsfifoptr = ucregister(MV_PTSFifo) + 4*em->video_ptsfifo_ptr;

		if (!(read_register(ptsfifoptr+1) & 1)) {
			em->video_ptsfifo_waiting=0;
			wake_up_interruptible(&em->video_ptsfifo_wait);
		}
	}
}

int em8300_video_write(struct em8300_s *em, const char * buf,
		       size_t count, loff_t *ppos)
{
	unsigned flags=0;
	
	if (em->video_ptsvalid) {
		int ptsfifoptr=0;
		em->video_lastpts = em->video_pts;
		em->video_pts >>= 1;

		flags = 0x40000000;
		ptsfifoptr = ucregister(MV_PTSFifo) + 4*em->video_ptsfifo_ptr;
		
		if (read_register(ptsfifoptr+3) & 1) {
			em->video_ptsfifo_waiting=1;
			interruptible_sleep_on(&em->video_ptsfifo_wait);
			em->video_ptsfifo_waiting=0;
			if (signal_pending(current)) {
				return -EINTR;
			}
		}	

#ifdef DEBUG_SYNC
		pr_debug("em8300_video.o: pts: %u\n", pts>>1);
#endif

		write_register(ptsfifoptr, em->video_offset >> 16);
		write_register(ptsfifoptr+1, em->video_offset & 0xffff);
		write_register(ptsfifoptr+2, em->video_pts >> 16);
		write_register(ptsfifoptr+3, (em->video_pts & 0xffff) | 1);
	
		em->video_ptsfifo_ptr++;
		em->video_ptsfifo_ptr %= read_ucregister(MV_PTSSize) / 4;

		em->video_ptsvalid=0;
	}

	em->video_offset += count;
	return em8300_fifo_writeblocking(em->mvfifo, count, buf, flags);
}

int em8300_video_ioctl(struct em8300_s *em, unsigned int cmd, unsigned long arg)
{
    unsigned scr,val;
	switch (_IOC_NR(cmd)) {
	case _IOC_NR(EM8300_IOCTL_VIDEO_SETPTS):
		if (get_user(em->video_pts, (int *)arg)) {
			return -EFAULT;
		}
		em->video_ptsvalid = 1;
		break;

	case _IOC_NR(EM8300_IOCTL_VIDEO_SETSCR):
		if (_IOC_DIR(cmd) & _IOC_WRITE) {
			if (get_user(val,(unsigned*)arg))
				return -EFAULT;
			val >>= 1;
			scr = read_ucregister(MV_SCRlo) | (read_ucregister(MV_SCRhi) << 16);
			scr -= val;
			if (scr < 0) scr = -scr;
			if (scr > 9000) {
				pr_info("setting scr: %i\n", val);
				write_ucregister(MV_SCRlo, val & 0xffff);
				write_ucregister(MV_SCRhi, (val >> 16) & 0xffff);
			}

		}
		if (_IOC_DIR(cmd) & _IOC_READ) {
			scr = read_ucregister(MV_SCRlo) | (read_ucregister(MV_SCRhi) << 16);
			copy_to_user((void *)arg, &scr, sizeof(unsigned));
		}
		break;

	default:
		return -EINVAL;
	}

	return 0;
}

void em8300_video_open(struct em8300_s *em)
{
	em->irqmask |= IRQSTATUS_VIDEO_FIFO | IRQSTATUS_VIDEO_VBL;
	write_ucregister(Q_IrqMask, em->irqmask);
}

int em8300_video_release(struct em8300_s *em)
{
	em->video_ptsfifo_ptr=0;
	em->video_offset=0;
	em->video_ptsvalid=0;
	em8300_fifo_sync(em->mvfifo);
	em8300_video_sync(em);

	em->irqmask &= ~(IRQSTATUS_VIDEO_FIFO | IRQSTATUS_VIDEO_VBL);
	write_ucregister(Q_IrqMask, em->irqmask);

	return em8300_video_setplaymode(em, EM8300_PLAYMODE_STOPPED);
}
