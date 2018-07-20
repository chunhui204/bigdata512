#ifndef LINUX_EM8300_H
#define LINUX_EM8300_H

#include "dvbdev.h"
#include <linux/dvb/video.h>
#include <linux/dvb/audio.h>
#include <linux/dvb/demux.h>
#include <linux/dvb/dmx.h>
#include <linux/dvb/ca.h>
#include <linux/dvb/osd.h>

#include "dvb_filter.h"
#include "dvb_demux.h"
#include "dmxdev.h"

typedef struct {
	void *ucode;
	int ucode_size;
} em8300_microcode_t;

typedef struct {
	int reg;
	int val;
	int microcode_register;
} em8300_register_t;

typedef struct {
	int brightness;
	int contrast;
	int saturation;
} em8300_bcs_t;

typedef struct {
	int cal_mode;
	int arg;
	int arg2;
	int result;
	int result2;
} em8300_overlay_calibrate_t;

typedef struct {
	int xpos, ypos;
	int width, height;
} em8300_overlay_window_t;

typedef struct {
	int xsize, ysize;
} em8300_overlay_screen_t;

typedef struct {
	int attribute;
	int value;
} em8300_attribute_t;

typedef struct {
	int color;
	int contrast;
	int top;
	int bottom;
	int left;
	int right;
} em8300_button_t;

#define MAX_UCODE_REGISTER 110

#define EM8300_IOCTL_INIT       _IOW('C',0,em8300_microcode_t)
#define EM8300_IOCTL_READREG    _IOWR('C',1,em8300_register_t)
#define EM8300_IOCTL_WRITEREG   _IOW('C',2,em8300_register_t)
#define EM8300_IOCTL_GETSTATUS  _IOR('C',3,char[1024])
#define EM8300_IOCTL_SETBCS	_IOW('C',4,em8300_bcs_t)
#define EM8300_IOCTL_GETBCS	_IOR('C',4,em8300_bcs_t)
#define EM8300_IOCTL_SET_ASPECTRATIO _IOW('C',5,int)
#define EM8300_IOCTL_GET_ASPECTRATIO _IOR('C',5,int)
#define EM8300_IOCTL_SET_VIDEOMODE _IOW('C',6,int)
#define EM8300_IOCTL_SET_PLAYMODE _IOW('C',7,int)
#define EM8300_IOCTL_SET_AUDIOMODE _IOW('C',8,int)
#define EM8300_IOCTL_GET_AUDIOMODE _IOR('C',8,int)
#define EM8300_IOCTL_SET_SPUMODE _IOW('C',9,int)
#define EM8300_IOCTL_OVERLAY_CALIBRATE _IOWR('C',10,em8300_overlay_calibrate_t)
#define EM8300_IOCTL_OVERLAY_SETMODE _IOW('C',11,int)
#define EM8300_IOCTL_OVERLAY_SETWINDOW _IOWR('C',12,em8300_overlay_window_t)
#define EM8300_IOCTL_OVERLAY_SETSCREEN _IOWR('C',13,em8300_overlay_screen_t)
#define EM8300_IOCTL_OVERLAY_GET_ATTRIBUTE _IOR('C',14,em8300_attribute_t)
#define EM8300_IOCTL_OVERLAY_SET_ATTRIBUTE _IOW('C',14,em8300_attribute_t)
#define EM8300_IOCTL_OVERLAY_SIGNALMODE _IOW('C',15,em8300_attribute_t)
#define EM8300_IOCTL_SCR_GET _IOR('C',16,unsigned)
#define EM8300_IOCTL_SCR_SET _IOW('C',16,unsigned)
#define EM8300_IOCTL_SCR_GETSPEED _IOR('C',17,unsigned)
#define EM8300_IOCTL_SCR_SETSPEED _IOW('C',17,unsigned)
 
#define EM8300_OVERLAY_SIGNAL_ONLY 1
#define EM8300_OVERLAY_SIGNAL_WITH_VGA 2
#define EM8300_OVERLAY_VGA_ONLY 3

#define EM8300_IOCTL_VIDEO_SETPTS 1
#define EM8300_IOCTL_VIDEO_GETSCR _IOR('C',2,unsigned)
#define EM8300_IOCTL_VIDEO_SETSCR _IOW('C',2,unsigned)

#define EM8300_IOCTL_SPU_SETPTS 1
#define EM8300_IOCTL_SPU_SETPALETTE 2
#define EM8300_IOCTL_SPU_BUTTON 3

#define EM8300_IOCTL_AUDIO_SETPTS _SIOWR('P', 30, int)

#define EM8300_ASPECTRATIO_4_3 0
#define EM8300_ASPECTRATIO_16_9 1
#define EM8300_ASPECTRATIO_LAST 1

#define EM8300_VIDEOMODE_PAL 0
#define EM8300_VIDEOMODE_PAL60 1
#define EM8300_VIDEOMODE_NTSC 2
#define EM8300_VIDEOMODE_LAST 2
#ifndef EM8300_VIDEOMODE_DEFAULT
#define EM8300_VIDEOMODE_DEFAULT EM8300_VIDEOMODE_NTSC
#endif

#define EM8300_AUDIOMODE_ANALOG 0
#define EM8300_AUDIOMODE_DIGITALPCM 1
#define EM8300_AUDIOMODE_DIGITALAC3 2
#ifndef EM8300_AUDIOMODE_DEFAULT
#define EM8300_AUDIOMODE_DEFAULT EM8300_AUDIOMODE_ANALOG
#endif

#define EM8300_SPUMODE_OFF 0
#define EM8300_SPUMODE_ON 1

#define EM8300_PLAYMODE_STOPPED         0
#define EM8300_PLAYMODE_PAUSED          1
#define EM8300_PLAYMODE_SLOWFORWARDS    2
#define EM8300_PLAYMODE_SLOWBACKWARDS   3
#define EM8300_PLAYMODE_SINGLESTEP      4
#define EM8300_PLAYMODE_PLAY            5
#define EM8300_PLAYMODE_REVERSEPLAY     6
#define EM8300_PLAYMODE_SCAN            7
#define EM8300_PLAYMODE_FRAMEBUF	8

#define EM8300_OVERLAY_MODE_OFF 0
#define EM8300_OVERLAY_MODE_RECTANGLE 1
#define EM8300_OVERLAY_MODE_OVERLAY 2

#define EM8300_OVERLAY_CALMODE_XOFFSET 1
#define EM8300_OVERLAY_CALMODE_YOFFSET 2
#define EM8300_OVERLAY_CALMODE_XCORRECTION 3
#define EM8300_OVERLAY_CALMODE_COLOR 4

#define EM9010_ATTRIBUTE_XCORR 1
#define EM9010_ATTRIBUTE_XOFFSET 2
#define EM9010_ATTRIBUTE_YOFFSET 3
#define EM9010_ATTRIBUTE_JITTER 4
#define EM9010_ATTRIBUTE_STABILITY 5
#define EM9010_ATTRIBUTE_KEYCOLOR_UPPER 6
#define EM9010_ATTRIBUTE_KEYCOLOR_LOWER 7
#define EM9010_ATTRIBUTE_MAX 7

#define EM8300_SUBDEVICE_CONTROL 0
#define EM8300_SUBDEVICE_VIDEO 1
#define EM8300_SUBDEVICE_AUDIO 2
#define EM8300_SUBDEVICE_SUBPICTURE 3

#ifndef PCI_VENDOR_ID_SIGMADESIGNS
#define PCI_VENDOR_ID_SIGMADESIGNS 0x1105
#define PCI_DEVICE_ID_SIGMADESIGNS_EM8300 0x8300
#endif

#define CLOCKGEN_SAMPFREQ_MASK 0xc0
#define CLOCKGEN_SAMPFREQ_66 0xc0
#define CLOCKGEN_SAMPFREQ_48 0x40
#define CLOCKGEN_SAMPFREQ_44 0x80
#define CLOCKGEN_SAMPFREQ_32 0x00

#define CLOCKGEN_OUTMASK 0x30
#define CLOCKGEN_DIGITALOUT 0x10
#define CLOCKGEN_ANALOGOUT 0x20

#define CLOCKGEN_MODEMASK 0x0f
#define CLOCKGEN_OVERLAYMODE_1 0x07
#define CLOCKGEN_TVMODE_1 0x0b
#define CLOCKGEN_OVERLAYMODE_2 0x04
#define CLOCKGEN_TVMODE_2 0x02

#define MVCOMMAND_STOP 0x0
#define MVCOMMAND_DISPLAYBUFINFO 0x11
#define MVCOMMAND_10 0x10
#define MVCOMMAND_PAUSE 1
#define MVCOMMAND_START 3

#define MACOMMAND_STOP 0x0
#define MACOMMAND_PAUSE 0x1
#define MACOMMAND_PLAY 0x2

#define IRQSTATUS_VIDEO_VBL 0x10
#define IRQSTATUS_VIDEO_FIFO 0x2
#define IRQSTATUS_AUDIO_FIFO 0x8

#define ENCODER_UNKNOWN 0
#define ENCODER_ADV7175 1 
#define ENCODER_ADV7170 2
#define ENCODER_BT865   3

#ifdef __KERNEL__

#define EM8300_MAX 4

#define EM8300_MAJOR 121
#define EM8300_LOGNAME "em8300"

#define DICOM_MODE_PAL 1
#define DICOM_MODE_NTSC 2
#define DICOM_MODE_PAL60 3

struct dicom_s {
	int luma;
	int chroma;
	int frametop;
	int framebottom;
	int frameleft;
	int frameright;
	int visibletop;
	int visiblebottom;
	int visibleleft;
	int visibleright;
	int tvout;
};

struct displaybuffer_info_s {
	int xsize;
	int ysize;
	int xsize2;
	int flag1,flag2;
	int buffer1;
	int buffer2;
	int unk_present;
	int unknown1;
	int unknown2;
	int unknown3;
};

struct em8300_audio_s {
	int channels;
	int format;
	int speed;
	int slotsize;
	int enable_bits;
};


#define VIDIOCSPANSCAN          _IOW('v',  BASE_VIDIOCPRIVATE+8, int)
#define VID_NONE_PREF           0x00    /* No aspect ration processing preferred */
#define VID_PAN_SCAN_PREF       0x01    /* Pan and Scan Display preferred */
#define VID_VERT_COMP_PREF      0x02    /* Vertical compression display preferred */
#define VID_VC_AND_PS_PREF      0x03    /* PanScan and vertical Compression if allowed */
#define VID_CENTRE_CUT_PREF     0x05    /* PanScan with zero vector */

struct em8300_s
{
	char name[40];
	
	int chip_revision;
	int pci_revision;
	
	int inuse[4];
	int ucodeloaded;
	
	struct pci_dev *dev;
	ulong adr;
	volatile unsigned *mem;
	ulong memsize;
	
	int playmode;
	
	/* Fifos */
	struct fifo_s *mvfifo;
	struct fifo_s *mafifo;
	struct fifo_s *spfifo;
	int mtrr_reg;
	
	/* DICOM */
	int dicom_vertoffset;
	int dicom_horizoffset;
	int dicom_brightness;
	int dicom_contrast;
	int dicom_saturation;
	int dicom_tvout;
	struct displaybuffer_info_s dbuf_info;
	
	/* I2C */
	int i2c_pin_reg;
	int i2c_oe_reg;
	
	/* different between revision 1 and revision 2 boards */
	int mystery_divisor;
	
	/* I2C bus 1*/
	struct i2c_algo_bit_data i2c_data_1;
	struct i2c_adapter i2c_ops_1;
	
	/* I2C bus 2*/
	struct i2c_algo_bit_data i2c_data_2;
	struct i2c_adapter i2c_ops_2;
	
	/* I2C clients */
	int encoder_type;
	struct i2c_client *encoder;
	struct i2c_client *eeprom;
	
	/* Microcode registers */
	unsigned ucode_regs[MAX_UCODE_REGISTER];
	int var_ucode_reg1; /* These are registers that differ */
	int var_ucode_reg2; /* between versions 1 and 2 of the board */
	int var_ucode_reg3; /* " */
	
	/* Interrupt */
	unsigned irqmask;
	
	/* Clockgenerator */
	int clockgen;
	int clockgen_overlaymode;
	int clockgen_tvmode;
	
	/* Timing measurement */
	struct timeval tv,last_status_time;
	long irqtimediff;
	int irqcount;
	int frames;
	int scr;
	
	/* Audio */
	struct em8300_audio_s audio;
	int audio_mode;
        int pcm_mode;
	int dsp_num;
/* remove these when audio sync goes away */
	int audio_sync;
	int audio_ptsvalid;
	uint32_t audio_pts;
	uint32_t audio_lastpts;
	uint32_t audio_lag;
	uint32_t last_calcbuf;
  /* the following used to be static in em8300_audio_write() */
	int rollover;
	uint32_t basepts;
	uint32_t rollover_pts;
	uint32_t rollover_lastpts;
	uint32_t rollover_startpts;
	uint32_t lastpts_count;
/* */
	int dword_DB4;
	unsigned char byte_D90[24];
	
	/* Video */
	int video_mode;
	int video_playmode;
	int aspect_ratio;
	uint32_t video_pts;
	uint32_t video_lastpts;
	int video_ptsvalid,video_offset,video_count;
	int video_ptsfifo_ptr;
	wait_queue_head_t video_ptsfifo_wait;

	int video_ptsfifo_waiting;
	int video_first;
	int var_video_value;
	
	/* Sub Picture */
	int sp_pts,sp_ptsvalid,sp_count;
	int sp_ptsfifo_ptr;
	wait_queue_head_t sp_ptsfifo_wait;

	int sp_ptsfifo_waiting;
	int sp_mode;
	
	int linecounter;
	
	/* EM9010 overlay processor */
	int overlay_enabled;
	int overlay_mode;
	int overlay_gamma_enable;
	int overlay_xres;
	int overlay_yres;
	int overlay_frame_xpos;
	int overlay_frame_ypos;
	int overlay_frame_width;
	int overlay_frame_height;
	int overlay_a[EM9010_ATTRIBUTE_MAX+1];
	int overlay_double_y;
	int overlay_xcorr_default;
	int overlay_70;
	int overlay_dword_24bb8;

};

#ifdef USE_DVB
#define DVB_DEVS_MAX 16

typedef struct dvb_devs_s {
        int num;  
        int tab[DVB_DEVS_MAX];
        int max_users[DVB_DEVS_MAX];
        int max_writers[DVB_DEVS_MAX];
} dvb_devs_t;

struct dvb_stuff{
        struct dvb_device       dvb_dev;
	struct em8300_s         *em8300;
        dvb_adapter_t       *dvb_adapter;
        dvb_device_t        *video_dev;
        dvb_device_t        *audio_dev;
	
        dvb_demux_t             demux;
        dvb_devs_t *dvb_devs;

        int users[DVB_DEVS_MAX];
        int writers[DVB_DEVS_MAX];

        uint16_t                lastaattr;
        dmxdev_t                dmxdev;
        boolean                 video_blank;
        struct video_status      videostate;
        struct audio_status      audiostate;
        int                     dvb_registered;
        char                    demux_id[16];
        dmx_frontend_t          mem_frontend;

        struct ipack_s          tsa,tsv; 
        struct ipack_s          pesa,pesv; 
        u8                      *kbuf[2];
        int audio_id;
        int audio_extid;
};
void dvb_unregister(struct dvb_stuff *card);
int dvb_register(struct dvb_stuff *card);

#endif


#define TIMEDIFF(a,b) a.tv_usec - b.tv_usec + \
	    1000000 * (a.tv_sec - b.tv_sec)


/*
  Prototypes
*/

/* em8300_i2c.c */
int em8300_i2c_init(struct em8300_s *em);
void em8300_i2c_exit(struct em8300_s *em);
void em8300_clockgen_write(struct em8300_s *em, int abyte);

void em9010_write(struct em8300_s *em, int reg, int data);
int em9010_read(struct em8300_s *em, int reg);
int em9010_read16(struct em8300_s *em, int reg);
void em9010_write16(struct em8300_s *em, int reg, int value);

/* em8300_audio.c */
int em8300_audio_ioctl(struct em8300_s *em,unsigned int cmd, unsigned long arg);
int em8300_audio_open(struct em8300_s *em);
int em8300_audio_release(struct em8300_s *em);
int em8300_audio_setup(struct em8300_s *em);
int em8300_audio_write(struct em8300_s *em, const char * buf,
		       size_t count, loff_t *ppos);
int mpegaudio_command(struct em8300_s *em, int cmd);

/* em8300_ucode.c */
int em8300_ucode_upload(struct em8300_s *em, void *ucode_user, int ucode_size);

/* em8300_misc.c */
int em8300_setregblock(struct em8300_s *em, int offset, int val, int len);
int em8300_writeregblock(struct em8300_s *em, int offset, unsigned *buf, int len);
int em8300_waitfor(struct em8300_s *em, int reg, int val, int mask);
int em8300_waitfor_not(struct em8300_s *em, int reg, int val, int mask);

/* em8300_dicom.c */
void em8300_dicom_setBCS(struct em8300_s *em, int brightness, int contrast, int saturation);
void em8300_dicom_enable(struct em8300_s *em);
void em8300_dicom_disable(struct em8300_s *em);
int em8300_dicom_update(struct em8300_s *em);
void em8300_dicom_init(struct em8300_s *em);
int em8300_dicom_get_dbufinfo(struct em8300_s *em);
void em8300_dicom_fill_dispbuffers(struct em8300_s *em, int xpos, int ypos, int xsize,
				  int ysize, unsigned int pat1, unsigned int pat2);

/* em8300_video.c */

void em8300_video_open(struct em8300_s *em);
int em8300_video_setplaymode(struct em8300_s *em, int mode);
int em8300_video_setup(struct em8300_s *em);
int em8300_video_release(struct em8300_s *em);
void em8300_video_setspeed(struct em8300_s *em, int speed);
int em8300_video_write(struct em8300_s *em, const char * buf,
		       size_t count, loff_t *ppos);
int em8300_video_ioctl(struct em8300_s *em, unsigned int cmd, unsigned long arg);
void em8300_video_check_ptsfifo(struct em8300_s *em);

/* em8300_spu.c */
int em8300_spu_write(struct em8300_s *em, const char * buf,
		       size_t count, loff_t *ppos);
int em8300_spu_open(struct em8300_s *em);
int em8300_spu_ioctl(struct em8300_s *em, unsigned int cmd, unsigned long arg);
int em8300_spu_init(struct em8300_s *em);
void em8300_spu_check_ptsfifo(struct em8300_s *em);
int em8300_ioctl_setspumode(struct em8300_s *em, int mode);

/* em8300_ioctl.c */
int em8300_control_ioctl(struct em8300_s *em, int cmd, unsigned long arg);
int em8300_ioctl_setvideomode(struct em8300_s *em, int mode);
int em8300_ioctl_setaspectratio(struct em8300_s *em, int ratio);
void em8300_ioctl_getstatus(struct em8300_s *em, char *usermsg);
int em8300_ioctl_init(struct em8300_s *em, em8300_microcode_t *useruc);
void em8300_ioctl_enable_videoout(struct em8300_s *em, int mode);
int em8300_ioctl_setplaymode(struct em8300_s *em, int mode);
int em8300_ioctl_setaudiomode(struct em8300_s *em, int mode);
int em8300_ioctl_getaudiomode(struct em8300_s *em, int mode);
int em8300_ioctl_overlay_calibrate(struct em8300_s *em, em8300_overlay_calibrate_t *c);
int em8300_ioctl_overlay_setwindow(struct em8300_s *em,em8300_overlay_window_t *w);
int em8300_ioctl_overlay_setscreen(struct em8300_s *em,em8300_overlay_screen_t *s);
int em8300_ioctl_overlay_setmode(struct em8300_s *em,int val);

/* em9010.c */
int em9010_cabledetect(struct em8300_s *em);
int em9010_calibrate_xoffset(struct em8300_s *em);
int em9010_calibrate_yoffset(struct em8300_s *em);
int em9010_init(struct em8300_s *em);
int em9010_overlay_set_signalmode(struct em8300_s *em, int val);
int em9010_overlay_update(struct em8300_s *em);
int em9010_overlay_set_res(struct em8300_s *em, int xres, int yres);
void sub_4288c(struct em8300_s *em, int pa, int pb, int pc, int pd, int pe, int pf,
	       int pg, int ph);
int em9010_get_attribute(struct em8300_s *em, int attribute);
int em9010_set_attribute(struct em8300_s *em, int attribute, int value);

#endif /* __KERNEL__ */

#endif /* LINUX_EM8300_H */
