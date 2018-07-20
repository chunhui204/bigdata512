/* 
    cardbase.h

    Copyright (C) Christian Wolff for convergence integrated media.

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

#ifndef CARDBASE_H
#define CARDBASE_H

#ifdef PCMCIA_DEBUG
static int pc_debug = PCMCIA_DEBUG;
MODULE_PARM(pc_debug, "i");
#define MARGI_DEBUG (pc_debug)
#else
#define MARGI_DEBUG 2 
#endif

// all the internal structs
#include <pcmcia/version.h>
#include "ringbuffy.h"

#include <linux/kernel.h>
#include <linux/config.h>

#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include <linux/delay.h>
#include <linux/poll.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <asm/byteorder.h>

#include <linux/dvb/video.h>
#include <linux/dvb/audio.h>
#include <linux/dvb/dmx.h>
#include <linux/dvb/ca.h>
#include <linux/dvb/osd.h>

#include "../dvbdev.h"
#include "../demux.h"
#include "../dvb_demux.h"
#include "../dmxdev.h"
#include "../dvb_filter.h"

// List of pci cards in the system

#include "cvdvtypes.h"

#define DVERSION             "0.7.0"
#define SHORTDEVNAME        "ConvDVD"
#define MEDDEVNAME          "convergence DVD"
#define LONGDEVNAME         "convergence DVD Video Decoder"
#define LOGNAME             "convdvd "
#define NBBUF 8


#ifdef MARGI_DEBUG
#define MDEBUG(n, args...) if (MARGI_DEBUG>(n)) printk(KERN_ERR LOGNAME args)
#else
#define MDEBUG(n, args...) 
#endif	


#define VID_PAN_SCAN_PREF       0x01    /* Pan and Scan Display preferred */
#define VID_VERT_COMP_PREF      0x02    /* Vertical compression display preferred */
#define VID_VC_AND_PS_PREF      0x03    /* PanScan and vertical Compression if allowed */
#define VID_CENTRE_CUT_PREF     0x05    /* PanScan with zero vector */


// Author definitions
#define NAME                "Christian Wolff"
#define EMAIL               "scarabaeus@convergence.de"
#define COMPANY             "convergence integrated media GmbH"
#define AUTHOR              NAME " <" EMAIL "> " COMPANY

#define MAXDEV            1	// maximum number of cards

#define NAVISIZE 1024		// size of one navi block
#define NAVIBUFFERSIZE NAVISIZE*10	// size of ten navi blocks

#define BLANK 0xFFFFFFFF

#define FIFO_MASK 1023

#define CCIR601Lines(system) (((system==NTSC) || (system==NTSC60) || (system==PALM) || (system==PALM60) || (system==PAL60))?525:625)

// default video mode
#define VIDEO_MODE PAL
//#define VIDEO_MODE NTSC

struct DRAMBlock {
	u32 start;		// start address of the block; (21 bit word address, 64 bit aligned)
	u32 length;		// length of the block (in 16 bit words)
	struct DRAMBlock *next;	// chain link
};

struct CSS {
	u8 status;		// interrupt status from Register 0x0B0
	int ChallengeReady;	// 1 if challenge data valid
	u8 challenge[10];	// challenge data
	int ResponseReady;	// 1 if response data valid
	u8 response[5];		// response data
	int DiskKey;		// 1 if disk key extraction complete
	int TitleKey;		// 1 if title key decryption complete
	int Error;		// 1 if authentication or disc key extraction
	int TitleKeyDiff;	// 1 if title key different from previous
};

struct GOPHeader {
	int valid;		// 1: struct contains valid data
	int timecode;
	int closedgop;
	int brokenlink;
};

struct SequenceHeader {
	int valid;		// 1: struct contains valid data
	int hsize;
	int vsize;
	int aspectratio;
	int frameratecode;
	int bitrate;
	int vbvbuffersize;
	int constrained;
};

struct SequenceExtension {
	int valid;		// 1: struct contains valid data
	int profilelevel;
	int progressive;
	int chroma;
	int hsizeext;
	int vsizeext;
	int bitrateext;
	int vbvbuffersizeext;
	int lowdelay;
	int frextn;
	int frextd;
};

struct AudioMPEG {
	int present;		// true: MPEG audio stream present
	int MPEG2;		// 0:MPEG1 Audio
	int layer;		// 1..3 (I..III)
	int bitrate;		// 0=free, 32-448 kbps
	int samplefreq;		// 32,44,48 (44 eq. 44.1)
	int mode;		// 0=stereo 1=joint-stereo 2=dualchannel 3=single channel (just right channel)
	int modeext;		// Layer I&II: intensity stereo subbands  Layer III: bit 0=intensity stereo, bit 1=ms-stereo
	int copyright;		// true=copyrighted material
	int original;		// 0=copy true=original
	int emphasis;		// 0=no emph. 1=50/15usec 3=CCITT J.17
};

struct AudioAC3 {
	int present;		// 1: AC3 audio stream present
	int acmod;		// parameters from the AC3 documentation
	int bsmod;
	int dialnorm;
	int dialnorm2;
	int surmixlev;
	int mixlevel;
	int cmixlev;
	int mixlevel2;
	int fscod;
	int lfeon;
	int bsid;
	int dsurmod;
	int frmsizecod;
	int langcod;
	int langcod2;
	int timecod;
	int roomtyp;
	int timecod2;
	int roomtyp2;
};

struct AudioPCM {
	int present;		// 1: PCM audio stream present
	int audio_frm_num;
	int num_of_audio_ch;
	int Fs;
	int quantization;
	int emphasis;
	int mute_bit;
};

struct AudioParam {
	int valid;
	struct AudioMPEG mpeg;
	struct AudioAC3 ac3;
	struct AudioPCM pcm;
};

struct OSDPicture {		// all u32 pointers are 21 bit word addresses 
	int open;		// are the buffers initialized?
	int width;		// frame width
	int height;		// frame height
	int bpp;		// bit per pixel
	int evenfirst;		// first line is in even field
	int aspectratio;	// pixel aspect ratio: 11/aspectratio
	int oddheight;		// height of the odd field
	u32 oddmem;		// DRAM address of allocated memory
	u32 odddata;		// data (=header) pointer
	u32 oddpalette;		// pointer to palette inside data
	u32 oddbitmap;		// pointer to bitmap inside data
	u32 oddterm;		// pointer to termination header
	int evenheight;		// height of the even field
	u32 evenmem;		// DRAM address of allocated memory
	u32 evendata;		// data (=header) pointer
	u32 evenpalette;	// pointer to palette inside data
	u32 evenbitmap;		// pointer to bitmap inside data
	u32 eventerm;		// pointer to termination header
};

struct StreamInfo {
	int valid;		// 1: struct contains valid data
	int MPEG2;		// 0: MPEG1/ISO11172  1: MPEG2/ISO13818
	int hsize;		// overall hsize (hsize&hsizeext)
	int vsize;		// overall vsize (vsize&vsizeext)
	int bitrate;		// overall bitrate (bitrate&bitrateext)
	int vbvbuffersize;	// overall...
	struct GOPHeader gop;
	struct SequenceHeader sh;
	struct SequenceExtension se;
	struct AudioParam audio;
};

struct StreamSetup {		// user selected parameters for the stream playback
	stream_type streamtype;	// what is the type of our input stream?
	audio_type audioselect;	// 0=auto/unknown 1=MPEG 2=LPCM 3=AC3
	int videoID;		// stream ID of the video ES, -1 for any
	int audioID;		// stream ID of the audio ES, -1 for any
	int audioIDext;		// stream ID of the audio extension ES, -1 for none
	int SPDIFmode;		// 0:MPEG/AC3 data on digital S/PDIF out 1:IEC956 data on digital S/PDIF out
};

#define MAX_PTS 256

typedef struct PTSRecord {
	int begin;
	int end;
	int size;
	u32 LastAddr;
	u32 AddrB[MAX_PTS];
	u32 AddrE[MAX_PTS];
	u32 PTS[MAX_PTS];
} PTSStorage;

#define DVB_DEVS_MAX 9

typedef struct dvb_devs_s {
        int num;  
        int tab[DVB_DEVS_MAX];
        int max_users[DVB_DEVS_MAX];
        int max_writers[DVB_DEVS_MAX];
} dvb_devs_t;

struct cvdv_cards {
	struct dvb_device       dvb_dev;
	struct dvb_demux        demux;
	struct cvdv_cards *next;
	void *margi;
	struct bus_operations *bus;
	u_char scl;
	u_char sda;
	int i2c_addr;
	u32 VideoESSize;		
	u32 AudioESSize;		
	u32 VideoPESSize;		
	u32 DataDumpSize;		
	u32 AudioPESSize;		
	u32 NaviBankSize;		
	int currentType;
	ringbuffy rbufA;
	ringbuffy rbufB;
	int use_ringA;
	int use_ringB;
	int nonblock;
	u8 *addr;
	unsigned int size;
	struct DRAMBlock *DRAMFirstBlock;
	u32 DRAMSize;
	struct OSDPicture OSD;
	int DMAABusy;		// Is the DMA A currently in use?
	int DMABBusy;		// Is the DMA B currently in use?
	int IntInstalled;	// is the card interrupt routine installed?
	int ChannelBuffersAllocated;	// Are the channel buffers for the decoder allocated?
	u32 VideoES;		// 21 bit word address of the allocated channel
	u32 AudioES;		// 21 bit word address of the allocated channel
	u32 VideoPES;		// 21 bit word address of the allocated channel
	u32 DataDump;		// 21 bit word address of the allocated channel
	u32 AudioPES;		// 21 bit word address of the allocated channel
	u32 NaviBank;		// 21 bit word address of the allocated channel
	int FrameBuffersAllocated;	// Are the frame buffers for the decoder allocated?
	u32 FrameStoreLuma1;	// 21 bit word address of the allocated frame
	u32 FrameStoreChroma1;	// 21 bit word address of the allocated frame
	u32 FrameStoreLuma2;	// 21 bit word address of the allocated frame
	u32 FrameStoreChroma2;	// 21 bit word address of the allocated frame
	u32 FrameStoreLumaB;	// 21 bit word address of the allocated frame
	u32 FrameStoreChromaB;	// 21 bit word address of the allocated frame
	int DecoderOpen;	// Is the Decoder initialized?
	u16 AuxFifo[FIFO_MASK + 1];	// Auxiliary Fifo Data
	int AuxFifoHead;	// Auxiliary Fifo Position
	int AuxFifoTail;	// Auxiliary Fifo Position
	u16 DataFifo[FIFO_MASK + 1];	// Data Fifo Data
	int DataFifoHead;	// Data Fifo Position
	int DataFifoTail;	// Data Fifo Position
	int FifoALast;		// last used thread of FIFO A
	int FifoBLast;		// last used thread of FIFO B
	videosystem videomode;	// current video output mode, PAL or NTSC
	struct StreamInfo stream;	// header information of the current stream
	struct StreamSetup setup;	// should be filled bevor sending data, but default is OK
	int AuxFifoExt;		// used by Aux FIFO parser
	int AuxFifoLayer;	//  "   "   "   "     "
	int AudioInitialized;	// Is the Audio set up?
	int AudioOldMode;	// remainder of the previous mode while trickmodes, or -1
	int open;	// is the 64017 initialized and the video out active?
	int closing;		// 1 if char device closed, but DMA still running
	int startingV;		// 1 if card is waiting for the Video ES buffer to fill up, to start the decoder
	int startingA;		// 1 if card is waiting for the Audio ES buffer to fill up, to start the decoder
	int startingDVDV;	// 1 if card is waiting for the Video ES buffer to fill up, to start the decoder
	int startingDVDA;	// 1 if card is waiting for the Audio ES buffer to fill up, to start the decoder
	int channelrun;		// 1 if channel has been started by the host
	int fields;		// counter of video fields, debugging only
	struct CSS css;		// CSS data
	u32 NaviPackAddress;	// Read address of the Navi Pack Buffer
	wait_queue_head_t wqA;
	wait_queue_head_t wqB;
	u8 navibuffer[NAVIBUFFERSIZE];
	int navihead;
	int navitail;
	int intdecodestatus;
	int showvideo;
	int videodelay;
	int videodelay_last;
	int videoskip;
	int videoskip_last;
	int videosync;
	int videoslow;
	int videoslow_last;
	int videoffwd;
	int videoffwd_last;
	PTSStorage VideoPTSStore;
	PTSStorage AudioPTSStore;
	u32 LastAddr;
	u32 VPTS;
	u32 oldVPTS;
	long VSCR;
	u32 APTS;
	u32 oldAPTS;
	int scrset;
	long ASCR;
	long SyncTime;
	int paused;
	u16 lastvattr;
	u16 lastaattr;
	u8 reg07B;		// mirrors of write-only register
	u8 reg08F;
	u8 reg090;
	u8 reg091;
	u8 reg092;
	u8 reg093;
	u8 highlight[10];	// content of registers 1C0 thru 1C0, to be written after next BAV int.
	int highlight_valid;	// if 1
	int do_flush;		// if 1, send flush packet after last transfer done
	int hasZV;

	struct timer_list timer;
	spinlock_t timelock;

        dvb_devs_t *dvb_devs;
        int users[DVB_DEVS_MAX];
        int writers[DVB_DEVS_MAX];

        dmxdev_t                dmxdev;
        int                     video_blank;
        struct video_status     videostate;
        struct audio_status     audiostate;
	int                     dvb_registered;
	char                    demux_id[16];
	dmx_frontend_t          mem_frontend;
	ipack                   tsa;
	ipack                   tsv;

	int svhs;
	int composite;

        struct dvb_adapter     *dvb_adapter;
        struct dvb_device      *video_dev;
        struct dvb_device      *audio_dev;
        struct dvb_device      *osd_dev;


};

extern u8 FlushPacket[32];

extern struct cvdv_cards *first_card;

void DecoderStreamReset(struct cvdv_cards *card);

void DecoderSetupReset(struct cvdv_cards *card);

void DecoderCSSReset(struct cvdv_cards *card);

void card_init(struct cvdv_cards *card);

#endif				/* CARDBASE_H */
