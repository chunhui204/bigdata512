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

extern int dicom_other_pal;
extern int dicom_fix;
extern int dicom_control;

struct dicom_tvmode {
	int vertsize;
	int horizsize;
	int vertoffset;
	int horizoffset;
};

struct dicom_tvmode tvmodematrix[EM8300_VIDEOMODE_LAST+1] = {
	{576, 720, 46, 130},     // PAL 4:3
	{480, 720, 46, 138},     // PAL60 4:3
	{482, 711, 34, 138},     // NTSC 4:3
};

/* C decompilation of the chromaluma code
*  by Anton Altaparmakov <antona@bigfoot.com>
*
*  This basically returns the result of calculating param1 / param2 and
*  depending on some weird rules either adds 1 to the result or not.
*  Returns 0 on error, ie. when param2 is 0.
*/
int sub_265C1 (int param1, unsigned int param2, short int param3)
{
	int local1;
	if (!param2) {
		return 0;
	}
	local1 = param1 / param2;
	if (param2 & 1) {
		param1 <<= 1;
		param2 <<= 1;
	}
	if (param3) {
		// Original was stupid, so there:
		if (param1 % param2 >= param2) {
			++local1;
		}
	} else {
		if (param1 % param2 >= param2 >> 1) {
			++local1;
		}
	}

	return local1;
}

/* sub_40137 calculates the contents of the dicom_bcsluma and dicom_bcschroma
*		 registers with brightness contrast and saturation values as inputs.
*/
int sub_40137(struct em8300_s *em)
{
	short int local1;
	int local2, local3;
	short int local4;
	int local5;
	short int local6, local7;
	int ir;
	register int eax;

	local3 = sub_265C1(em->dicom_contrast * 0x7f, 0x3e8, 0); // 0x3e8 = 1000
	local2 = sub_265C1(em->dicom_brightness * 0xff, 0x3e8, 0);
	local5 = sub_265C1(em->dicom_saturation * 0x1f, 0x3e8, 0);
	if (local3 >= 0x40) {
		local1 = 0x2000 / (0xc0 - local3);
		ir = local2 - 0x80 - ((local3 - 0x40) << 7) / (0xc0 - local3);
		if (ir > -128) {
			local4 = ir;
		} else {
			local4 = -128;
		}
		if ((eax = local5 << 7) < 0) {
			eax += 0xf;
		}
		local6 = local7 = ((eax & 0xfffffff0) << 2) / (0xc0 - local3);
	} else {
		register int eax;
		if ((eax = (local3 + 0x40) << 6) < 0) {
			eax += 0x7f;
		}
		local1 = eax >> 7;
		if ((eax = local2 - local3 - 0x40) < 0x7f) {
			local4 = eax;
		} else {
			local4 = 0x7f;
		}
		if ((eax = local5 * (local3 + 0x40)) < 0) {
			eax += 0xf;
		}
		if ((eax = (eax & 0xfffffff0) << 2) < 0) {
			eax += 0x7f;
		}
		local6 = local7 =  eax >> 7;
	}
#if 0
	if (em->encoder_type != ENCODER_BT865) {
#endif
		write_ucregister(DICOM_BCSLuma, (local1 << 8) | (local4 & 0xff));
		write_ucregister(DICOM_BCSChroma, local7 << 8 | local6);
#if 0
	}
#endif

	return 1;
}

void em8300_dicom_setBCS(struct em8300_s *em, int brightness, int contrast, int saturation) 
{
	em->dicom_brightness = brightness;
	em->dicom_contrast = contrast;
	em->dicom_saturation = saturation;

	if (read_ucregister(DICOM_UpdateFlag) == 1) {
		write_ucregister(DICOM_UpdateFlag, 0);
		udelay(1);
	}

	sub_40137(em); // Update brightness/contrast/saturation

	write_ucregister(DICOM_UpdateFlag, 1);
}

int em8300_dicom_update(struct em8300_s *em) 
{
	int ret;
	int vmode_ntsc = 1;
	
	if (dicom_other_pal) {
		vmode_ntsc = (em->video_mode == EM8300_VIDEOMODE_NTSC);
	}

	if ((ret=em8300_waitfor(em, ucregister(DICOM_UpdateFlag), 0, 1))) {
		return ret;
	}

	if (em->overlay_enabled) {
		sub_4288c(em, em->overlay_frame_xpos, em->overlay_frame_ypos, em->overlay_frame_width, em->overlay_frame_height, em->overlay_a[EM9010_ATTRIBUTE_XOFFSET], em->overlay_a[EM9010_ATTRIBUTE_YOFFSET], em->overlay_a[EM9010_ATTRIBUTE_XCORR], em->overlay_double_y);
	} else {
		write_ucregister(DICOM_FrameTop, tvmodematrix[em->video_mode].vertoffset);
		write_ucregister(DICOM_FrameBottom, tvmodematrix[em->video_mode].vertoffset + tvmodematrix[em->video_mode].vertsize-1);
		write_ucregister(DICOM_FrameLeft, tvmodematrix[em->video_mode].horizoffset);
		write_ucregister(DICOM_FrameRight, tvmodematrix[em->video_mode].horizoffset + tvmodematrix[em->video_mode].horizsize-1);
		write_ucregister(DICOM_VisibleTop, tvmodematrix[em->video_mode].vertoffset);
		write_ucregister(DICOM_VisibleBottom, tvmodematrix[em->video_mode].vertoffset + tvmodematrix[em->video_mode].vertsize-1);
		write_ucregister(DICOM_VisibleLeft, tvmodematrix[em->video_mode].horizoffset);
		write_ucregister(DICOM_VisibleRight, tvmodematrix[em->video_mode].horizoffset + tvmodematrix[em->video_mode].horizsize-1);
	}

	if (em->aspect_ratio == EM8300_ASPECTRATIO_16_9) {
		em->dicom_tvout |= 0x10;
	} else {
		em->dicom_tvout &= ~0x10;
	}

	write_ucregister(DICOM_TvOut, em->dicom_tvout);

	if (em->overlay_enabled) {
		write_register(0x1f47, 0x0);
		write_register(0x1f5e, 0x1afe);
		write_ucregister(DICOM_Control, 0x9afe);

#if 0 /* don't know if this is necessary yet */
#ifdef EM8300_DICOM_0x1f5e_0x1efe
		write_register(0x1f5e, 0x1efe);
#else
		write_register(0x1f5e, 0x1afe);
#endif
#ifdef EM8300_DICOM_CONTROL_0x9efe
		write_ucregister(DICOM_Control, 0x9efe);
#else
		write_ucregister(DICOM_Control, 0x9afe);
#endif
#endif
	} else {

		if (em->encoder_type == ENCODER_BT865) {
			write_register(0x1f47, 0x0);
			if (em->video_mode == EM8300_VIDEOMODE_NTSC) {
				write_register(EM8300_HSYNC_LO, 0x8a);	// 138
				write_register(EM8300_HSYNC_HI, 0x2c7);	// 711
			} else {
				write_register(EM8300_HSYNC_LO, 0x8c);
				write_register(EM8300_HSYNC_HI, 0x2d0);
			}
			if (vmode_ntsc) {
				write_register(EM8300_VSYNC_HI, 0x104);	// 260 ??
			} else {
				write_register(EM8300_VSYNC_HI, 0x136);	 
			}

			write_ucregister(DICOM_VSyncLo1, 0x1); 
			write_ucregister(DICOM_VSyncLo2, 0x0);
			write_ucregister(DICOM_VSyncDelay1, 0xd2); 
			write_ucregister(DICOM_VSyncDelay2, 0x00);  

			write_register(0x1f46, 0x00);
			write_register(0x1f47, 0x1c);

			write_register(0x1f5e, 0x9efe);
			write_ucregister(DICOM_Control, 0x9efe);		
		} else { /* ADV7170 or ADV7175A */
			write_register(0x1f47, 0x18);

			if (vmode_ntsc) {
				if (dicom_fix) {
					write_register(0x1f5e, 0x1efe);
				} else {
					write_register(0x1f5e, 0x1afe);
				}

				if (dicom_control) {
					write_ucregister(DICOM_Control, 0x9efe);
				} else {
					write_ucregister(DICOM_Control, 0x9afe);
				}
			} else {
				if (dicom_fix) {
					write_register(0x1f5e, 0x1afe);
				} else {
					write_register(0x1f5e, 0x1efe);
				}
	
				if (dicom_control) {
					write_ucregister(DICOM_Control, 0x9afe);
				} else {
					write_ucregister(DICOM_Control, 0x9efe);
				}
			}			 
		}
	}

	pr_debug("em8300_dicom.o: vmode_ntsc: %d\n", vmode_ntsc);
	pr_debug("em8300_dicom.o: dicom_other_pal: %d\n", dicom_other_pal);
	pr_debug("em8300_dicom.o: dicom_control: %d\n", dicom_control);
	pr_debug("em8300_dicom.o: dicom_fix: %d\n", dicom_fix);

	write_ucregister(DICOM_UpdateFlag, 1);
	
	return em8300_waitfor(em, ucregister(DICOM_UpdateFlag), 0, 1);
}



void em8300_dicom_disable(struct em8300_s *em)
{
	em->dicom_tvout=0x8000;
	write_ucregister(DICOM_TvOut, em->dicom_tvout);
}

void em8300_dicom_enable(struct em8300_s *em)
{
	if (em->overlay_enabled) {
		em->dicom_tvout=0x4000;
	} else {
		em->dicom_tvout=0x4001;
	}

	if (em->aspect_ratio == EM8300_ASPECTRATIO_16_9) {
	  em->dicom_tvout |= 0x10;
	} else {
	  em->dicom_tvout &= ~0x10;
	}

	write_ucregister(DICOM_TvOut, em->dicom_tvout);
}

int em8300_dicom_get_dbufinfo(struct em8300_s *em)
{
	int displaybuffer;
	struct displaybuffer_info_s *di=&em->dbuf_info;
	
	displaybuffer = read_ucregister(DICOM_DisplayBuffer)+0x1000;

	di->xsize = read_register(displaybuffer);
	di->ysize = read_register(displaybuffer+1);
	di->xsize2 = read_register(displaybuffer+2) & 0xfff;
	di->flag1 = read_register(displaybuffer+2) & 0x8000;
	di->flag2 = read_ucregister(Vsync_DBuf) & 0x4000;

	if(read_ucregister(MicroCodeVersion) <= 0xf) {
		di->buffer1 = (read_register(displaybuffer+3) |	(read_register(displaybuffer+4)<<16)) << 4 ;
		di->buffer2 = (read_register(displaybuffer+5) |	(read_register(displaybuffer+6)<<16)) << 4 ;
	} else {
		di->buffer1 = read_register(displaybuffer+3) << 6;
		di->buffer2 = read_register(displaybuffer+4) << 6;
	}

	if(displaybuffer == ucregister(Width_Buf3)) {
		di->unk_present = 1;
		if(read_ucregister(MicroCodeVersion) <= 0xf) {
			di->unknown1 = read_register(displaybuffer+7);
			di->unknown2 = (read_register(displaybuffer+8) | (read_register(displaybuffer+9)<<16)) << 4 ;
			di->unknown3 = (read_register(displaybuffer+0xa) | (read_register(displaybuffer+0xb)<<16)) << 4 ;
		} else {
			di->unknown2 = read_register(displaybuffer+6);
			di->unknown3 = read_register(displaybuffer+7);
		}
	} else {
		di->unk_present = 0;
	}
	
	pr_debug("DICOM buffer: xsize=0x%x(%d)\n", di->xsize, di->xsize);
	pr_debug("			  ysize=0x%x(%d)\n", di->ysize, di->ysize);
	pr_debug("			  xsize2=0x%x(%d)\n", di->xsize2, di->xsize2);
	pr_debug("			  flag1=%d, flag2=%d\n", di->flag1, di->flag2);
	pr_debug("			  buffer1=0x%x(%d)\n", di->buffer1, di->buffer1);
	pr_debug("			  buffer2=0x%x(%d)\n", di->buffer2, di->buffer2);
	if(di->unk_present) {
	pr_debug("			  unknown1=0x%x(%d)\n", di->unknown1, di->unknown1);
	pr_debug("			  unknown2=0x%x(%d)\n", di->unknown2, di->unknown2);
	pr_debug("			  unknown3=0x%x(%d)\n", di->unknown3, di->unknown3);
	}
	return 0;
}

/* sub_42A32
   Arguments 
   xoffset = ebp+0x8
   yoffset = ebp+0xc
   c = ebp+0x10
   lines = ebp+0x14
   pat1 = ebp+0x18
   pat2 = ebp+0x1c
 */
void em8300_dicom_fill_dispbuffers(struct em8300_s *em, int xpos, int ypos, int xsize, int ysize, unsigned int pat1, unsigned int pat2)
{
	int i;

	pr_debug("ysize: %d, xsize: %d\n", ysize, xsize);
	pr_debug("buffer1: %d, buffer2: %d\n", em->dbuf_info.buffer1, em->dbuf_info.buffer2);
	
	for(i=0; i < ysize; i++) {
		em8300_setregblock(em, em->dbuf_info.buffer1 + xpos + (ypos+i)*em->dbuf_info.xsize, pat1, xsize);
		em8300_setregblock(em, em->dbuf_info.buffer2 + xpos + (ypos+i)/2*em->dbuf_info.xsize, pat2, xsize);
	}
}

void em8300_dicom_init(struct em8300_s *em)
{
	em8300_dicom_disable(em);
}
