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

#include "encoder.h"

int em8300_control_ioctl(struct em8300_s *em, int cmd, unsigned long arg)
{
	em8300_register_t reg;
	int val, len, err;
	em8300_bcs_t bcs;
	em8300_overlay_window_t ov_win;
	em8300_overlay_screen_t ov_scr;
	em8300_overlay_calibrate_t ov_cal;
	em8300_attribute_t attr;
	
	if (_IOC_DIR(cmd) != 0) {
		len = _IOC_SIZE(cmd);

		if (len < 1 || len > 65536 || arg == 0) {
			return -EFAULT;
		}
		if (_IOC_DIR(cmd) & _IOC_WRITE) {
			if ((err = verify_area(VERIFY_READ, (void *)arg, len)) < 0) {
				return err;
			}
		}
		if (_IOC_DIR(cmd) & _IOC_READ) {
			if ((err = verify_area(VERIFY_WRITE, (void *)arg, len)) < 0) {
				return err;
			}
		}
	}	

	switch(_IOC_NR(cmd)) {
	case _IOC_NR(EM8300_IOCTL_INIT):
		return em8300_ioctl_init(em, (em8300_microcode_t *)arg);\

	case _IOC_NR(EM8300_IOCTL_WRITEREG):
		copy_from_user(&reg, (void *)arg, sizeof(em8300_register_t));

		if (reg.microcode_register) {
			write_ucregister(reg.reg, reg.val);
		} else {
			write_register(reg.reg, reg.val);
		}
		break;

	case _IOC_NR(EM8300_IOCTL_READREG):
		copy_from_user(&reg, (void *)arg, sizeof(em8300_register_t));

		if (reg.microcode_register) {
			reg.val = read_ucregister(reg.reg);
			reg.reg = ucregister(reg.reg);
		} else {
			reg.val = read_register(reg.reg);
		}
		copy_to_user((void *)arg,&reg, sizeof(em8300_register_t));
		break;

	case _IOC_NR(EM8300_IOCTL_GETSTATUS):
		em8300_ioctl_getstatus(em, (char *)arg);
		return 0;

	case _IOC_NR(EM8300_IOCTL_GETBCS):
		if (_IOC_DIR(cmd) & _IOC_WRITE) {
			copy_from_user(&bcs, (void *)arg, sizeof(em8300_bcs_t));
			em8300_dicom_setBCS(em, bcs.brightness, bcs.contrast, bcs.saturation);
		}

		if (_IOC_DIR(cmd) & _IOC_READ) {
			bcs.brightness = em->dicom_brightness;
			bcs.contrast = em->dicom_contrast;
			bcs.saturation = em->dicom_saturation;
			copy_to_user((void *)arg, &bcs, sizeof(em8300_bcs_t));
		}
	break;

	case _IOC_NR(EM8300_IOCTL_SET_VIDEOMODE):
		if (_IOC_DIR(cmd) & _IOC_WRITE) {
			get_user(val, (int *)arg);
			em8300_ioctl_setvideomode(em, val);
		}
		break;

	case _IOC_NR(EM8300_IOCTL_SET_PLAYMODE):
		if (_IOC_DIR(cmd) & _IOC_WRITE) {
			get_user(val, (int *)arg);
			em8300_ioctl_setplaymode(em, val);
		}
		break;

	case _IOC_NR(EM8300_IOCTL_SET_ASPECTRATIO):
		if (_IOC_DIR(cmd) & _IOC_WRITE) {
			get_user(val, (int *)arg);
			em8300_ioctl_setaspectratio(em, val);
		}
		break;

	case _IOC_NR(EM8300_IOCTL_GET_AUDIOMODE):
		if (_IOC_DIR(cmd) & _IOC_WRITE) {
			get_user(val, (int *)arg);
			em8300_ioctl_setaudiomode(em,val);
		}
		if (_IOC_DIR(cmd) & _IOC_READ) {
			em8300_ioctl_getaudiomode(em, arg);
		}
		break;

	case _IOC_NR(EM8300_IOCTL_SET_SPUMODE):
		if (_IOC_DIR(cmd) & _IOC_WRITE) {
			get_user(val, (int *)arg);
			em8300_ioctl_setspumode(em,val);
		}
		break;

	case _IOC_NR(EM8300_IOCTL_OVERLAY_SETMODE):
		if (_IOC_DIR(cmd) & _IOC_WRITE) {
			get_user(val, (int *)arg);
			if (!em8300_ioctl_overlay_setmode(em, val)) {
				return -EINVAL;
			}
		}
		break;

	case _IOC_NR(EM8300_IOCTL_OVERLAY_SIGNALMODE):
		if (_IOC_DIR(cmd) & _IOC_WRITE) {
			get_user(val, (int *)arg);
			if (!em9010_overlay_set_signalmode(em, val)) {
				return -EINVAL;
			}
		}
		break;

	case _IOC_NR(EM8300_IOCTL_OVERLAY_SETWINDOW):
		if (_IOC_DIR(cmd) & _IOC_WRITE) {
			copy_from_user(&ov_win, (void *)arg, sizeof(em8300_overlay_window_t));
			if (!em8300_ioctl_overlay_setwindow(em, &ov_win)) {
				return -EINVAL;
			}
		}
		if (_IOC_DIR(cmd) & _IOC_READ) {
			copy_to_user((void *)arg, &ov_win, sizeof(em8300_overlay_window_t));
		}
		break;

	case _IOC_NR(EM8300_IOCTL_OVERLAY_SETSCREEN):
		if (_IOC_DIR(cmd) & _IOC_WRITE) {
			copy_from_user(&ov_scr, (void *)arg, sizeof(em8300_overlay_screen_t));
			if (!em8300_ioctl_overlay_setscreen(em, &ov_scr)) {
				return -EINVAL;
			}
		}
		if (_IOC_DIR(cmd) & _IOC_READ) {
			copy_to_user((void *)arg, &ov_scr, sizeof(em8300_overlay_screen_t));
		}
	break;

	case _IOC_NR(EM8300_IOCTL_OVERLAY_CALIBRATE):
		if (_IOC_DIR(cmd) & _IOC_WRITE) {
			copy_from_user(&ov_cal, (void *)arg, sizeof(em8300_overlay_calibrate_t));
			if(!em8300_ioctl_overlay_calibrate(em, &ov_cal)) {
				return -EIO;
			}
		}
	
		if (_IOC_DIR(cmd) & _IOC_READ) {
			copy_to_user((void *)arg, &ov_cal, sizeof(em8300_overlay_calibrate_t));
		}
	break;

	case _IOC_NR(EM8300_IOCTL_OVERLAY_GET_ATTRIBUTE):
		copy_from_user(&attr, (void *)arg, sizeof(em8300_attribute_t));		
		if (_IOC_DIR(cmd) & _IOC_WRITE) {
			em9010_set_attribute(em, attr.attribute, attr.value);
		}
		if (_IOC_DIR(cmd) & _IOC_READ) {
			attr.value = em9010_get_attribute(em, attr.attribute);
			copy_to_user((void *)arg, &attr, sizeof(em8300_attribute_t));
		}
	break;

	case _IOC_NR(EM8300_IOCTL_SCR_GET):
		if (_IOC_DIR(cmd) & _IOC_WRITE) {
			unsigned scr;
			if (get_user(val,(unsigned*)arg))
				return -EFAULT;
			scr = read_ucregister(MV_SCRlo) | (read_ucregister(MV_SCRhi) << 16);
			
			if (scr > val) scr = scr - val;
			else           scr = val - scr;
				
			if (scr > 2*1800) { /* Tolerance: 2 frames */
				pr_info("adjusting scr: %i\n", val);
				write_ucregister(MV_SCRlo, val & 0xffff);
				write_ucregister(MV_SCRhi, (val >> 16) & 0xffff);
			}
		}
		if (_IOC_DIR(cmd) & _IOC_READ) {
			val = read_ucregister(MV_SCRlo) | (read_ucregister(MV_SCRhi) << 16);
			copy_to_user((void *)arg, &val, sizeof(unsigned));
		}
	break;

	case _IOC_NR(EM8300_IOCTL_SCR_GETSPEED):
		if (_IOC_DIR(cmd) & _IOC_WRITE) {
			get_user(val, (int*)arg);
			val &= 0xFFFF;

			write_ucregister(MV_SCRSpeed,
			 read_ucregister(MicroCodeVersion) >= 0x29 ? val : val >> 8);
		}
		if (_IOC_DIR(cmd) & _IOC_READ) {
			val = read_ucregister(MV_SCRSpeed);
			if (! read_ucregister(MicroCodeVersion) >= 0x29)
				val <<= 8;

			copy_to_user((void *)arg, &val, sizeof(unsigned));
		}
	break;

	default:
		return -ETIME;
	}

	return 0;
}

int em8300_ioctl_init(struct em8300_s *em, em8300_microcode_t *useruc)
{
	em8300_microcode_t uc;
	int ret;

	copy_from_user(&uc, useruc, sizeof(em8300_microcode_t));

	if ((ret=em8300_ucode_upload(em, uc.ucode, uc.ucode_size))) {
		return ret;
	}

	em8300_dicom_init(em);
	
	if ((ret=em8300_video_setup(em))) {
		return ret;
	}

	if (em->mvfifo) em8300_fifo_free(em->mvfifo);
	if (em->mafifo) em8300_fifo_free(em->mafifo);
	if (em->spfifo) em8300_fifo_free(em->spfifo);
	
	if (!(em->mvfifo = em8300_fifo_alloc())) {
		return -ENOMEM;
	}
	
	if (!(em->mafifo = em8300_fifo_alloc())) {
		return -ENOMEM;
	}

	if (!(em->spfifo = em8300_fifo_alloc())) {
		return -ENOMEM;
	}

	em8300_fifo_init(em,em->mvfifo, MV_PCIStart, MV_PCIWrPtr, MV_PCIRdPtr, MV_PCISize, 0x900, FIFOTYPE_VIDEO);
	em8300_fifo_init(em,em->mafifo, MA_PCIStart, MA_PCIWrPtr, MA_PCIRdPtr, MA_PCISize, 0x1000, FIFOTYPE_AUDIO); 
	em8300_fifo_init(em,em->spfifo, SP_PCIStart, SP_PCIWrPtr, SP_PCIRdPtr, SP_PCISize, 0x1000, FIFOTYPE_VIDEO);
	em8300_spu_init(em);

	if ((ret=em8300_audio_setup(em))) {
		return ret;
	}

	em8300_ioctl_enable_videoout(em, 1);
	
	em->ucodeloaded = 1;

	printk(KERN_NOTICE "em8300: Microcode version 0x%02x loaded\n", read_ucregister(MicroCodeVersion));
	return 0;
}

void em8300_ioctl_getstatus(struct em8300_s *em, char *usermsg) 
{
	char tmpstr[1024];
	struct timeval tv;
	long tdiff,frames,scr,picpts;
	char mvfstatus[128];
	char mafstatus[128];
	char spfstatus[128];

	em8300_fifo_statusmsg(em->mvfifo, mvfstatus);
	em8300_fifo_statusmsg(em->mafifo, mafstatus);
	em8300_fifo_statusmsg(em->spfifo, spfstatus);
		
	frames = (read_ucregister(MV_FrameCntHi) << 16) | read_ucregister(MV_FrameCntLo);
	picpts = (read_ucregister(PicPTSHi) << 16) |
	read_ucregister(PicPTSLo);
	scr = (read_ucregister(MV_SCRhi) << 16) | read_ucregister(MV_SCRlo);
	
	do_gettimeofday(&tv);
	tdiff = TIMEDIFF(tv, em->last_status_time);
	em->last_status_time = tv;
	sprintf(tmpstr,"Time elapsed: %ld us\nIRQ time period: %ld\nInterrupts : %d\nFrames: %ld\nSCR: %ld\nPicture PTS: %ld\nSCR diff: %ld\nMV Fifo: %s\nMA Fifo: %s\nSP Fifo: %s\nAudio pts: %d\nAudio lag: %d\n", tdiff, em->irqtimediff,em->irqcount,frames-em->frames,scr,picpts,scr-em->scr,mvfstatus,mafstatus,spfstatus,em->audio_pts,em->audio_lag);
	em->irqcount = 0;
	em->frames = frames;
	em->scr = scr;
	copy_to_user((void *)usermsg, tmpstr, strlen(tmpstr)+1);
}


int em8300_ioctl_setvideomode(struct em8300_s *em, int mode)
{
	int encoder;

	switch (mode) {
	case EM8300_VIDEOMODE_PAL:
		encoder = ENCODER_MODE_PAL;
		break;
	case EM8300_VIDEOMODE_PAL60:
		encoder = ENCODER_MODE_PAL60;
		break;
	case EM8300_VIDEOMODE_NTSC:
		encoder = ENCODER_MODE_NTSC;
		break;
	default:
		return -EINVAL;
	}

	em->video_mode = mode;

	em8300_dicom_disable(em);

	if (em->encoder) {
		em->encoder->driver->command(em->encoder, ENCODER_CMD_SETMODE, (void *)encoder);
	}
	em8300_dicom_enable(em);
	em8300_dicom_update(em);

	return 0;
}

void em8300_ioctl_enable_videoout(struct em8300_s *em, int mode)
{
	em8300_dicom_disable(em);
	
	if (em->encoder) {
		em->encoder->driver->command(em->encoder, ENCODER_CMD_ENABLEOUTPUT, (void *)mode);
	}
	em8300_dicom_enable(em);
}


int em8300_ioctl_setaspectratio(struct em8300_s *em, int ratio)
{
	em->aspect_ratio = ratio;	
	em8300_dicom_update(em);

	return 0;
}

int em8300_ioctl_setplaymode(struct em8300_s *em, int mode) {
	switch (mode) {
	case EM8300_PLAYMODE_PLAY:
		mpegaudio_command(em, MACOMMAND_PLAY);
		if (em->playmode == EM8300_PLAYMODE_STOPPED) {
			em8300_ioctl_enable_videoout(em, 1);
		}
		em8300_video_setplaymode(em,mode);
		break;
	case EM8300_PLAYMODE_STOPPED:
		em8300_ioctl_enable_videoout(em, 0);
		em8300_video_setplaymode(em, mode);
		break;
	case EM8300_PLAYMODE_PAUSED:
		mpegaudio_command(em, MACOMMAND_PAUSE);
		em8300_video_setplaymode(em, mode);
		break;
	default:
		return -1;
	}
	em->playmode = mode;

	return 0;
}

int em8300_ioctl_setspumode(struct em8300_s *em, int mode) {
	em->sp_mode=mode;
	return 0;
}

int em8300_ioctl_overlay_setmode(struct em8300_s *em, int val) {
	switch (val) {
	case EM8300_OVERLAY_MODE_OFF:
		if (em->overlay_enabled) {
		        em->clockgen = (em->clockgen & ~CLOCKGEN_MODEMASK) | em->clockgen_tvmode;
			em8300_clockgen_write(em, em->clockgen);
			em->overlay_enabled=0;
			em->overlay_mode=val;
			em8300_ioctl_setvideomode(em, em->video_mode);
			em9010_overlay_update(em);
		}
		break;
	case EM8300_OVERLAY_MODE_RECTANGLE:
	case EM8300_OVERLAY_MODE_OVERLAY:
		if (!em->overlay_enabled) {
		        em->clockgen = (em->clockgen & ~CLOCKGEN_MODEMASK) | em->clockgen_overlaymode;
			em8300_clockgen_write(em, em->clockgen);
			em->overlay_enabled=1;
			em->overlay_mode=val;
			em8300_dicom_disable(em);
			em8300_dicom_enable(em);
			em8300_dicom_update(em);
			em9010_overlay_update(em);
		} else {
			em->overlay_mode=val;
			em9010_overlay_update(em);
		}
		break;
	default:
		return 0;
	}

	return 1;
}

int em8300_ioctl_overlay_setwindow(struct em8300_s *em, em8300_overlay_window_t *w)
{
	if (w->xpos < -2000 || w->xpos > 2000) {
		return 0;
	}
	if (w->ypos < -2000 || w->ypos > 2000) {
		return 0;
	}
	if (w->width <= 0 || w->width > 2000) {
		return 0;
	}
	if (w->height <= 0 || w->height > 2000) {
		return 0;
	}
	em->overlay_frame_xpos = w->xpos;
	em->overlay_frame_ypos = w->ypos;
	em->overlay_frame_width = w->width;
	em->overlay_frame_height = w->height;

	if (em->overlay_enabled) {
		sub_4288c(em, em->overlay_frame_xpos, em->overlay_frame_ypos, em->overlay_frame_width, em->overlay_frame_height, em->overlay_a[EM9010_ATTRIBUTE_XOFFSET], em->overlay_a[EM9010_ATTRIBUTE_YOFFSET], em->overlay_a[EM9010_ATTRIBUTE_XCORR], em->overlay_double_y);
	} else {
		em8300_dicom_update(em);
	}

	return 1;
}

int em8300_ioctl_overlay_setscreen(struct em8300_s *em, em8300_overlay_screen_t *s)
{
	if (s->xsize < 0 || s->xsize > 2000) {
		return 0;
	}
	if (s->ysize < 0 || s->ysize > 2000) {
		return 0;
	}

	em9010_overlay_set_res(em, s->xsize, s->ysize);
	return 1;
}
