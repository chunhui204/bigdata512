/* 
    cvdv.c

    Copyright (C) Christian Wolff 
                  Marcus Metzler for convergence integrated media.

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

      /////////////////////////////////////////////////////////////////////
     //                                                                 //
    //   Driver for the Convergence Digital Video decoder card (pci)   //
   //   with L64017, L64021, PCM1723, and Bt864/Bt865 chipset         //
  //   (c) Christian Wolff 19990209 for convergence integrated media //
 //                                                                 //
/////////////////////////////////////////////////////////////////////

#define __NO_VERSION__

#include <linux/module.h>
#include "cvdv.h"
#include <linux/dvb/osd.h>
#include "i2c.h"

  //////////////////////  
 // global variables //
//////////////////////

// my little random function for memory test
uint16_t rnd_seed;
uint16_t rnd(uint16_t range)
{				// returns random 0..(range-1) range<=872
	uint32_t b = 75 * (rnd_seed + 1) - 1;
	rnd_seed = (uint16_t) (b & 0xFFFF);
	return ((b * range) / 0xFFFF) - ((b / 0xFFFF) * range);
}
void rnd_omize(void)
{
	rnd_seed = (uint16_t) jiffies;
}

static char *cimlogo[] = {
".............................................",
".............................................",
"......................###....................",
".....................#####...................",
".....................######..................",
"..............#......#####...................",
"...........#####....######...................",
".........########...######...................",
"........########....######...................",
".......#########...######....................",
"......########.....######...####.............",
".....#######.......#####...#####.............",
".....######.......######...######............",
"....#######.......######....######...........",
"....######........######....######...........",
"....#####........######......#####...........",
"...######........######......#####...........",
"...#####.........######......######..........",
"...#####.........#####.......######..........",
"...#####........######........#####..........",
"...#####........######.......######..........",
"...#####........#####.........#####..........",
"...#####.......######........######..........",
"...#####.......######........#####...........",
"...######.......####.........#####...........",
"....#####........##.........######...........",
"....######..................######...........",
"....######.................######............",
".....#######..............######.....#####...",
".....########............#######....#######..",
"......#########........########.....#######..",
".......#######################......########.",
"........#####################.......#######..",
"..........#################.........#######..",
"............#############............#####...",
"...............#.#####.................##....",
".............................................",
"............................................."
};

    /////////////////////////////////////////////
   //                                         //
  //  Controlling the L64021 MPEG-2 Decoder  //
 //                                         //
/////////////////////////////////////////////

int OSDTest(struct cvdv_cards *card)
{
	int i, j, col, x0, y0, x1, y1,aspx;
	uint8_t b;


	if (!card->OSD.open)
		return -2;

	OSDQuery(card, &x0, &y0, &x1, &y1, &aspx);
	OSDShow(card);
	OSDSetColor(card, 0, 0, 0, 0, 0, 0, 0);
	OSDSetColor(card, 1, 128, 255, 255, 0, 0, 0);
	for ( i = 0; i < cimlogo_width; i++){
		for ( j = 0; j < cimlogo_height; j++){
			b = cimlogo[j][i];
			col = (b == '#') ? 1: 0;
			OSDSetPixel(card, x0+i, y0+j, col);
		}
	}

	return 0;
}


void SetVideoSystem(struct cvdv_cards *card)
{
	uint8_t reg;

	// set the hsync and vsync generators in the L64017 according to the video standard
	reg = read_indexed_register(card, IIO_VIDEO_CONTROL1);
	reg &= ~0x03;
	switch (card->videomode) {
	case PAL:		// 864*625*50Hz = 27MHz, 25fps
		I2CWrite(card, card->i2c_addr, CS_CONTROL0, 0x41 | 0x0a);
		I2CWrite(card, card->i2c_addr, CS_CONTROL1, 0x04);
		I2CWrite(card, card->i2c_addr, CS_SC_AMP, 0x15);
		I2CWrite(card, card->i2c_addr, CS_SC_SYNTH0, 0x96);
		I2CWrite(card, card->i2c_addr, CS_SC_SYNTH1, 0x15);
		I2CWrite(card, card->i2c_addr, CS_SC_SYNTH2, 0x13);
		I2CWrite(card, card->i2c_addr, CS_SC_SYNTH3, 0x54);
		reg |= VMS_PAL;
		break;
	case PALN:
		I2CWrite(card, card->i2c_addr, CS_CONTROL0, 0xa1 | 0x0a);
		I2CWrite(card, card->i2c_addr, CS_CONTROL1, 0x04);
		I2CWrite(card, card->i2c_addr, CS_SC_AMP, 0x15);
		I2CWrite(card, card->i2c_addr, CS_SC_SYNTH0, 0x96);
		I2CWrite(card, card->i2c_addr, CS_SC_SYNTH1, 0x15);
		I2CWrite(card, card->i2c_addr, CS_SC_SYNTH2, 0x13);
		I2CWrite(card, card->i2c_addr, CS_SC_SYNTH3, 0x54);
		reg |= VMS_PAL;
		break;

	case PALNc:
		I2CWrite(card, card->i2c_addr, CS_CONTROL0, 0x81 | 0x0a);
		I2CWrite(card, card->i2c_addr, CS_CONTROL1, 0x04);
		I2CWrite(card, card->i2c_addr, CS_SC_AMP, 0x15);
		I2CWrite(card, card->i2c_addr, CS_SC_SYNTH0, 0x8c);
		I2CWrite(card, card->i2c_addr, CS_SC_SYNTH1, 0x28);
		I2CWrite(card, card->i2c_addr, CS_SC_SYNTH2, 0xed);
		I2CWrite(card, card->i2c_addr, CS_SC_SYNTH3, 0x43);
		reg |= VMS_PAL;
		break;

	case NTSC:		// 858*525*59.94006Hz = 27MHz, 29.97fps
		I2CWrite(card, card->i2c_addr, CS_CONTROL0, 0x01 | 0x0a);
		I2CWrite(card, card->i2c_addr, CS_CONTROL1, 0x04);
		I2CWrite(card, card->i2c_addr, CS_SC_AMP, 0x1c);
		I2CWrite(card, card->i2c_addr, CS_SC_SYNTH0, 0x3e);
		I2CWrite(card, card->i2c_addr, CS_SC_SYNTH1, 0xf8);
		I2CWrite(card, card->i2c_addr, CS_SC_SYNTH2, 0xe0);
		I2CWrite(card, card->i2c_addr, CS_SC_SYNTH3, 0x43);
		reg |= VMS_NTSC;
		break;

	case PALM:
		I2CWrite(card, card->i2c_addr, CS_CONTROL0, 0x01 | 0x0a);
		I2CWrite(card, card->i2c_addr, CS_CONTROL1, 0x04);
		I2CWrite(card, card->i2c_addr, CS_SC_AMP, 0x15);
		I2CWrite(card, card->i2c_addr, CS_SC_SYNTH0, 0x4e);
		I2CWrite(card, card->i2c_addr, CS_SC_SYNTH1, 0x4a);
		I2CWrite(card, card->i2c_addr, CS_SC_SYNTH2, 0xe1);
		I2CWrite(card, card->i2c_addr, CS_SC_SYNTH3, 0x43);
		reg |= VMS_PAL;
		break;

	case NTSC60:		// 857*525*60.010002Hz = 27MHz, 30fps
		I2CWrite(card, card->i2c_addr, CS_CONTROL0, 0x21 | 0x0a);
		I2CWrite(card, card->i2c_addr, CS_CONTROL1, 0x04);
		I2CWrite(card, card->i2c_addr, CS_SC_AMP, 0x1c);
		I2CWrite(card, card->i2c_addr, CS_SC_SYNTH0, 0x3e);
		I2CWrite(card, card->i2c_addr, CS_SC_SYNTH1, 0xf8);
		I2CWrite(card, card->i2c_addr, CS_SC_SYNTH2, 0xe0);
		I2CWrite(card, card->i2c_addr, CS_SC_SYNTH3, 0x43);
		reg |= VMS_NTSC;
		break;

	case PALM60:
		I2CWrite(card, card->i2c_addr, CS_CONTROL0, 0x61 | 0x0a);
		I2CWrite(card, card->i2c_addr, CS_CONTROL1, 0x04);
		I2CWrite(card, card->i2c_addr, CS_SC_AMP, 0x15);
		I2CWrite(card, card->i2c_addr, CS_SC_SYNTH0, 0x4e);
		I2CWrite(card, card->i2c_addr, CS_SC_SYNTH1, 0x4a);
		I2CWrite(card, card->i2c_addr, CS_SC_SYNTH2, 0xe1);
		I2CWrite(card, card->i2c_addr, CS_SC_SYNTH3, 0x43);
		reg |= VMS_PAL;
		break;

	case PAL60:
		break;
	}
	write_indexed_register(card, IIO_VIDEO_CONTROL1, reg);
	// set the pixel generators according to the video standard
	L64021Setup(card);
}

int SetVideoAttr(struct cvdv_cards *card, uint16_t vattr)
{
	uint8_t video_compression_mode;
	uint8_t tv_system;
	uint8_t aspect_ratio;
	uint8_t display_mode;
	uint8_t line_21_switch_1;
	uint8_t line_21_switch_2;
	uint8_t source_picture_resolution;
	uint8_t source_picture_letterboxed;
	uint8_t reserved;
	uint8_t film_camera_mode;
	uint16_t hsize, vsize;
	if (vattr != card->lastvattr) {
		video_compression_mode = (vattr >> 14) & 0x03;
		tv_system = (vattr >> 12) & 0x03;
		aspect_ratio = (vattr >> 10) & 0x03;
		display_mode = (vattr >> 8) & 0x03;
		line_21_switch_1 = (vattr >> 7) & 0x01;
		line_21_switch_2 = (vattr >> 6) & 0x01;
		source_picture_resolution = (vattr >> 3) & 0x07;
		source_picture_letterboxed = (vattr >> 2) & 0x01;
		reserved = (vattr >> 1) & 0x01;
		film_camera_mode = (vattr >> 0) & 0x01;
		card->videomode =
			((tv_system == 0) ? NTSC : ((tv_system == 1) ? 
						    PAL : PAL));	
		SetVideoSystem(card);
		hsize =
			((source_picture_resolution == 0) ? 720
			 : ((source_picture_resolution == 1) ? 702 : 352));
		vsize = ((source_picture_resolution == 3)
			 ? ((tv_system == 0) ? 240 : 288)
			 : ((tv_system == 0) ? 480 : 576));
		if (DecoderOpen
		    (card, hsize, vsize, ((aspect_ratio) ? 3 : 2),
		     ((video_compression_mode) ? 0 : 1),
		     source_picture_letterboxed, tv_system)) {	
			MDEBUG(0,
			       ": Video Decoder Open failed: On-card memory insufficient for frame stores\n");
		}
		card->lastvattr = vattr;
	} else {
		MDEBUG(0,
		       ": Video attribute not set, equal to previous one.\n");
	}
	return 0;
}

int SetAudioAttr(struct cvdv_cards *card, uint16_t aattr)
{
	uint8_t audio_coding_mode;
	uint8_t multichannel_extension;
	uint8_t audio_type;
	uint8_t audio_application_mode;
	uint8_t quantization_drc;
	uint8_t fs;
	uint8_t reserved;
	uint8_t num_audio_ch;
	if (aattr) {
		if (aattr != card->lastaattr) {
			audio_coding_mode = (aattr >> 13) & 0x07;
			multichannel_extension = (aattr >> 12) & 0x01;
			audio_type = (aattr >> 10) & 0x03;
			audio_application_mode = (aattr >> 8) & 0x03;
			quantization_drc = (aattr >> 6) & 0x03;
			fs = (aattr >> 4) & 0x03;
			reserved = (aattr >> 3) & 0x01;
			num_audio_ch = (aattr >> 0) & 0x07;
			switch (audio_coding_mode) {
			case 0:	// AC-3
				card->setup.audioselect = audio_AC3;
				break;
			case 2:	// MPEG Audio
				card->setup.audioselect = audio_MPEG;
				break;
			case 3:	// MPEG Audio with ext.
				card->setup.audioselect = audio_MPEG_EXT;
				break;
			case 4:	// Linear Pulse Code fe_modulation_t LPCM
				card->setup.audioselect = audio_LPCM;
				break;
			case 6:	// DTS
				card->setup.audioselect = audio_DTS;
				break;
			case 7:	// SDDS
				card->setup.audioselect = audio_SDDS;
				break;
			}
			DecoderPrepareAudio(card);
			AudioInit(card, ((fs) ? 96 : 48),
				  ((audio_application_mode == 2) ? 1 : 0));
		} else {
			MDEBUG(0,
			       ": Audio attribute not set, equal to previous one.\n");
		}
	} else {
		card->setup.audioselect = audio_none;
		DecoderPrepareAudio(card);
	}
	card->lastaattr = aattr;
	return 0;
}

int Prepare(struct cvdv_cards *card)
{
	int err, h;
	struct StreamSetup *setup = &card->setup;

	if (!card->ChannelBuffersAllocated) {
			
		DecoderStreamReset(card);
		if (setup->streamtype == stream_none) {
			setup->streamtype = stream_PS; 
		}
		
		if (setup->audioselect == audio_none) {
			setup->audioselect = audio_MPEG;
		}

		DecoderPrepareAudio(card);
		AudioMute(card, 1);
		DecoderPrepareVideo(card);
		VideoSetBackground(card, 1, 0, 0, 0);	// black

		switch (setup->streamtype) {
		default:
		case stream_none:	// unknown stream!
			MDEBUG(0,
			       ": Video Decoder Prepare failed: unknown stream type\n");
			return -ENODEV;	// not an MPEG stream!
		case stream_ES:	// Elementary Stream
			err = DecoderPrepareES(card);
			break;
		case stream_PES:	// Packetized Elementary Stream
			err = DecoderPreparePES(card);
			break;
		case stream_PS:	// MPEG-1 System Stream / MPEG-2 Program Stream
			err = DecoderPreparePS(card, -1, 0, 0, 0, 0, 0);
			break;
		case stream_DVD:	// DVD Stream
			err = DecoderPreparePS(card, 0, 0, 0, 0, 3, 1);
			break;
		}
		if (err) {	// insufficient memory
			MDEBUG(0,
			       ": Video Decoder Prepare failed: no kernel memory, please reboot if possible\n");
			CloseCard(card);
			return -ENODEV;
		}
	}

	// Set up the Video Decoder as we have the stream information
	if ((!card->FrameBuffersAllocated)
	    && (card->ChannelBuffersAllocated) && (card->stream.sh.valid)) {
		//  Automatic PAL/NTSC-switch according to MPEG-Source
		h = card->stream.vsize;
		if (h < 480)
			h *= 2;	// catch quarter sized images
		printk(KERN_INFO LOGNAME ": Video mode: %s\n",
		       ((h == 480) ? "NTSC" : "PAL"));
		card->videomode = ((h == 480) ? NTSC : PAL);
		SetVideoSystem(card);
		// Open the Video Decoder with the parameters retreived from the stream
		if (
		    (err =
		     DecoderOpen(card, card->stream.hsize,
				 card->stream.vsize,
				 card->stream.sh.aspectratio,
				 !card->stream.MPEG2, 0,
				 (card->stream.hsize > 480)))) {	// TODO: include vbvbuffersize
			MDEBUG(0,
			       ": Video Decoder Open failed: %s\n",
			       ((err == 1) ?
				"Picture size too big (>1440 pixel wide)" :
				"On-card memory insufficient for frame stores"));
			CloseCard(card);
			return -ENODEV;	// picture too big or insufficient memory
		}
		MDEBUG(1, ": Ready to go\n");
		card->startingV = 1;	// tell the card to start playing as soon as ES-buffers are sufficiently full
		card->startingA = 1;	// tell the card to start playing as soon as ES-buffers are sufficiently full
	}
	

	return 0;
}

int SetSCRstart(struct cvdv_cards *card, uint32_t SCR_base)
{
	uint32_t SCR_compare;
	uint32_t SCR_compareA;
	uint32_t SCR_compareV;
	if (card->startingV) {
		MDEBUG(0, ": SCR in DVD Pack: 0x%08X\n",
		       SCR_base);
		card->startingV = 0;
		card->startingA = 0;
		DecoderMaskByte(card, 0x007, 0xD2, 0xD2);	// Set 0x010, halt SCR counter
		SCR_compare = SCR_base + 000;
		if (SCR_base < 900)
			SCR_base = 0;
		else
			SCR_base -= 900;
		//DecoderWriteDWord(card,0x009,SCR_base);  // Set SCR counter
		DecoderWriteByte(card, 0x009, SCR_base & 0xFF);	// Set SCR counter
		DecoderWriteByte(card, 0x00A, (SCR_base >> 8) & 0xFF);
		DecoderWriteByte(card, 0x00B, (SCR_base >> 16) & 0xFF);
		DecoderWriteByte(card, 0x00C, (SCR_base >> 24) & 0xFF);
		DecoderMaskByte(card, 0x011, 0x03, 0x02);	// compare, not capture
		MDEBUG(0, ": SCR compare value: 0x%08X\n",
		       SCR_compare);
		//DecoderWriteDWord(card,0x00D,SCR_compare);  // Set Compare register
		DecoderWriteByte(card, 0x00D, SCR_compare & 0xFF);	// Set Compare register
		DecoderWriteByte(card, 0x00E, (SCR_compare >> 8) & 0xFF);
		DecoderWriteByte(card, 0x00F, (SCR_compare >> 16) & 0xFF);
		DecoderWriteByte(card, 0x010, (SCR_compare >> 24) & 0xFF);
		//DecoderWriteDWord(card,0x014,SCR_compare);  // Set audio compare reg.
		DecoderWriteByte(card, 0x014, SCR_compare & 0xFF);	// Set audio compare reg.
		DecoderWriteByte(card, 0x015, (SCR_compare >> 8) & 0xFF);
		DecoderWriteByte(card, 0x016, (SCR_compare >> 16) & 0xFF);
		DecoderWriteByte(card, 0x017, (SCR_compare >> 24) & 0xFF);
		DecoderSetByte(card, 0x013, 0x03);	// Video and Audio start on cmp.
		//DecoderSetVideoPanic(card,0,DecoderGetVideoESSize(card)/4);  // video panic at 25 percent
		VideoSetBackground(card, 1, 0, 0, 0);	// black
		SCR_base = DecoderReadByte(card, 0x009);
		SCR_base =
		    SCR_base | ((uint32_t) DecoderReadByte(card, 0x00A) << 8);
		SCR_base =
		    SCR_base | ((uint32_t) DecoderReadByte(card, 0x00B) << 16);
		SCR_base =
		    SCR_base | ((uint32_t) DecoderReadByte(card, 0x00C) << 24);
		SCR_compareA = DecoderReadByte(card, 0x014);
		SCR_compareA =
		    SCR_compareA | ((uint32_t) DecoderReadByte(card, 0x015) <<
				    8);
		SCR_compareA =
		    SCR_compareA | ((uint32_t) DecoderReadByte(card, 0x016) <<
				    16);
		SCR_compareA =
		    SCR_compareA | ((uint32_t) DecoderReadByte(card, 0x017) <<
				    24);
		SCR_compareV = DecoderReadByte(card, 0x00D);
		SCR_compareV =
		    SCR_compareV | ((uint32_t) DecoderReadByte(card, 0x00E) <<
				    8);
		SCR_compareV =
		    SCR_compareV | ((uint32_t) DecoderReadByte(card, 0x00F) <<
				    16);
		SCR_compareV =
		    SCR_compareV | ((uint32_t) DecoderReadByte(card, 0x010) <<
				    24);
		if (DecoderReadByte(card, 0x013) & 0x03)
			MDEBUG(1,": SCR 0x%08X, videocmp=0x%08X, audiocmp=0x%08X %02X\n",
			       SCR_base, SCR_compareV, SCR_compareA,
			       DecoderReadByte(card, 0x013));
		DecoderMaskByte(card, 0x007, 0xD2, 0xC2);	// Del 0x010, SCR counter run
	}
	return 0;
}


    //////////////////////////////
   //                          //
  //  Char Device Procedures  //
 //                          //
//////////////////////////////
static long margi_write(struct cvdv_cards *card, const char *data,
                      unsigned long count, int nonblock)
{

	int res;
	long int out=0;
	int free;

	free = ring_write_rest(&(card->rbufA));

	if (card != NULL) {
		card->nonblock = nonblock;
		if (count > 0) {	// Do we have data?
			if ((res = Prepare(card)))
				return res;
			if (!card->use_ringA)
				MargiSetBuffers(card, NBBUF*CHANNELBUFFERSIZE,
						0);
			if (!nonblock && 
			    !wait_event_interruptible(
				    card->wqA, 
				    ring_write_rest(&(card->rbufA)) >count )){
				
				out = MargiPushA(card, count,
						 data);
			} else {
				out = MargiPushA(card, count, data);
			}
		}
		return out;
	} else {
		MDEBUG(0,
		       ": Video Decoder Prepare failed\n");
		return -ENODEV;
	}
}	


static long margi_write_audio(struct cvdv_cards *card, const char *data,
                      unsigned long count, int nonblock)
{
	struct StreamSetup *setup = &card->setup;

	int res;
	long int out=0;
	int free;

	free = ring_write_rest(&(card->rbufB));

	if (card != NULL) {
		card->nonblock = nonblock;
	
		if (count > 0) {	// Do we have data?
			if ((res = Prepare(card)))
				return res;
			if ((setup->streamtype == stream_ES)
			    || (setup->streamtype == stream_PES)){
				if (!card->use_ringB)
					MargiSetBuffers(card, NBBUF*
                                                         CHANNELBUFFERSIZE,1);
				if (!nonblock && 
				    !wait_event_interruptible(
					    card->wqB, 
					    ring_write_rest(&(card->rbufB))
					    > count)){
					out = MargiPushB(card, count,
							 data);
				} else {
					out = MargiPushB(card, count, data);
				}
			}
		}
		return out;
	} else {
		MDEBUG(0,
		       ": Video Decoder Prepare failed\n");
		return -ENODEV;
	}
}	

void pes_write(uint8_t *buf, int count, void *priv)
{
	struct cvdv_cards *card = (struct cvdv_cards *) priv;
	
	margi_write(card, buf, count, 0);
}

static int
OSD_DrawCommand(struct cvdv_cards *card,osd_cmd_t *dc)
{

	switch (dc->cmd) {
	case OSD_Close:
		MDEBUG(1,": OSD Close\n");
		return OSDClose(card);
	case OSD_Open:	// Open(x0,y0,x1,y1,BitPerPixel(2/4/8),mix(0..15))
		return OSDOpen(card, dc->x0,
			       dc->y0, dc->x1,
			       dc->y1,
			       dc->color & 0x0F,
			       (dc->color >> 4) &
			       0x0F);
	case OSD_Show:
		return OSDShow(card);
	case OSD_Hide:
		return OSDHide(card);
	case OSD_Clear:
		return OSDClear(card);
	case OSD_Fill:	// Fill(color)
		return OSDFill(card, dc->color);
	case OSD_SetColor:    // SetColor(color,R(x0),G(y0),B(x1),opacity(y1))
		return (OSDSetColor
			(card, dc->color, dc->x0,
			 dc->y0, dc->x1, 0,
			 (dc->y1 != 255),
			 (dc->y1 == 0)) >= 0);
	case OSD_SetPalette:// SetPalette(firstcolor{color},lastcolor{x0},data)
		return OSDSetPalette(card,
				     dc->color,
				     dc->x0, (uint8_t *)
				     dc->data);
	case OSD_SetTrans:	// SetTrans(transparency{color})
		return OSDSetTrans(card,
				   (dc->color >> 4)
				   & 0x0F);
	case OSD_SetPixel:	// SetPixel(x0,y0,color)
		return OSDSetPixel(card, dc->x0,
				   dc->y0,
				   dc->color);
	case OSD_GetPixel:	// GetPixel(x0,y0);
		return OSDGetPixel(card, dc->x0,
				   dc->y0);
	case OSD_SetRow:	// SetRow(x0,y0,x1,(uint8_t*)data)
		return OSDSetRow(card, dc->x0,
				 dc->y0, dc->x1,
				 (uint8_t *) dc->data);
	case OSD_SetBlock:	// SetBlock(x0,y0,x1,y1,(uint8_t*)data)
		return OSDSetBlock(card, dc->x0,
				   dc->y0, dc->x1,
				   dc->y1,
				   dc->color,
				   (uint8_t *)
				   dc->data);
	case OSD_FillRow:	// FillRow(x0,y0,x1,color)
		return OSDFillRow(card, dc->x0,
				  dc->y0, dc->x1,
				  dc->color);
	case OSD_FillBlock:	// FillRow(x0,y0,x1,y1,color)
		return OSDFillBlock(card, dc->x0,
				    dc->y0, dc->x1,
				    dc->y1,
				    dc->color);
	case OSD_Line:	// Line(x0,y0,x1,y1,color);
		return OSDLine(card, dc->x0,
			       dc->y0, dc->x1,
			       dc->y1, dc->color);
	case OSD_Query:	// Query(x0,y0,x1,y1,aspect(color:11)
		return OSDQuery(card, &dc->x0,
				&dc->y0, &dc->x1,
				&dc->y1,
				&dc->color);
	case OSD_Test:
		return OSDTest(card);
	default:
		return -EINVAL;
	}
}


static int margi_open(struct cvdv_cards *card, int flags)
{
	int closed;
	if (card != NULL) {
		MDEBUG(1, ": -- open \n");
		CloseCard(card);
		OSDClose(card);
		card->timer.function = Timerfunction;
		card->timer.data=(unsigned long) card;
		card->timer.expires=jiffies+1;
		add_timer(&card->timer);
		if (card->open)
			MDEBUG(0,": margi_open - already open\n");
		closed = 1;
		if (card->open)
			closed = 0;
		if (closed) {	// first open() for this card?
			MargiFreeBuffers(card);
			VideoSetBackground(card, 1, 0, 0, 0);	// black
		}
		card->open++;
		MOD_INC_USE_COUNT;
		return 0;
	} else {
		MDEBUG(0,
		       ": Video Decoder Prepare failed\n");
		return -ENODEV;
	}

}


static int dvb_video_open(struct inode *inode, struct file *file)
{
        struct dvb_device *dvbdev=*(struct dvb_device**) file->private_data;
        struct cvdv_cards *card=(struct cvdv_cards *) dvbdev->priv;
        int err;

        if ((err=dvb_generic_open(inode, file))<0)
                return err;

	card->video_blank=1;
	card->audiostate.AV_sync_state=1;
	card->videostate.stream_source=VIDEO_SOURCE_DEMUX;
	margi_open(card, file->f_flags);
	
	return 0;
}

static int dvb_audio_open(struct inode *inode, struct file *file)
{
        struct dvb_device *dvbdev=*(struct dvb_device**) file->private_data;
        struct cvdv_cards *card=(struct cvdv_cards *) dvbdev->priv;
        int err;

        if ((err=dvb_generic_open(inode, file))<0)
                return err;

	card->audiostate.AV_sync_state=1;
	card->audiostate.stream_source=AUDIO_SOURCE_DEMUX;
        return 0;
}


static int dvb_video_release(struct inode *inode, struct file *file)
{
        struct dvb_device *dvbdev=*(struct dvb_device**) file->private_data;
        struct cvdv_cards *card=(struct cvdv_cards *) dvbdev->priv;

	DecoderPause(card);                   
	card->videostate.play_state = VIDEO_STOPPED;
	if (card->videostate.video_blank)
		VideoSetBackground(card, 1, 0, 0, 0);  
		
        return dvb_generic_release(inode, file);
}

static int dvb_audio_release(struct inode *inode, struct file *file)
{
        struct dvb_device *dvbdev=*(struct dvb_device **) file->private_data;
        struct cvdv_cards *card=(struct cvdv_cards *) dvbdev->priv;

	AudioStopDecode(card);
	card->audiostate.play_state=AUDIO_STOPPED;

        return dvb_generic_release(inode, file);
}


static ssize_t 
dvb_video_write(struct file *file, const char *buf, 
                size_t count, loff_t *ppos)
{
        struct dvb_device *dvbdev=*(struct dvb_device **) file->private_data;
        struct cvdv_cards *card=(struct cvdv_cards *) dvbdev->priv;

	if (card->videostate.stream_source!=VIDEO_SOURCE_MEMORY)
		return -EPERM;
	return margi_write(card, buf, count, 
			   file->f_flags&O_NONBLOCK);
}


static ssize_t 
dvb_audio_write(struct file *file, const char *buf, 
                size_t count, loff_t *ppos)
{
        struct dvb_device *dvbdev=*(struct dvb_device **) file->private_data;
        struct cvdv_cards *card=(struct cvdv_cards *) dvbdev->priv;

	if (card->audiostate.stream_source!=AUDIO_SOURCE_MEMORY)
		return -EPERM;
	if ( card->setup.streamtype !=  stream_PES )
		return -EPERM;
	
	return margi_write_audio(card, buf, count, 
				 file->f_flags&O_NONBLOCK);
}


static int 
dvb_video_ioctl(struct inode *inode, struct file *file,
            unsigned int cmd, void *parg)
{
	struct dvb_device *dvbdev=*(struct dvb_device **) file->private_data;
        unsigned long arg=(unsigned long) parg;
	struct cvdv_cards *card=(struct cvdv_cards *) dvbdev->priv;

	if (((file->f_flags&O_ACCMODE)==O_RDONLY) &&
	    (cmd!=VIDEO_GET_STATUS))
		return -EPERM;

	switch (cmd) {
		
	case VIDEO_STOP:
		DecoderPause(card);                   
		card->videostate.play_state = VIDEO_STOPPED;
		if (card->videostate.video_blank)
			VideoSetBackground(card, 1, 0, 0, 0);  
		
		
		return 0; 
		
	case VIDEO_PLAY:
		
		if (card->videostate.stream_source==
		    VIDEO_SOURCE_MEMORY) {
			if (card->videostate.play_state==VIDEO_FREEZED){
				DecoderUnPause(card);	
			} else {
				DecoderUnPause(card);	
			} 
		}
		break;

	case VIDEO_FREEZE:
		DecoderPause(card);                   
		break;
		
	case VIDEO_CONTINUE:
		if (card->videostate.play_state==VIDEO_FREEZED) {
			DecoderUnPause(card);                   
		} 
		break;
		
	case VIDEO_SELECT_SOURCE:
		card->videostate.stream_source=(video_stream_source_t) arg;
		break;
		
	case VIDEO_SET_BLANK:
		card->videostate.video_blank=(int) arg;
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
		card->videoffwd = 3;
		break;
		
	case VIDEO_SLOWMOTION:
		if (card->videostate.stream_source!=VIDEO_SOURCE_MEMORY)
			return -EPERM;
		card->videoslow = arg;
		
		break;
		
	case VIDEO_GET_CAPABILITIES:
	{
		int cap=VIDEO_CAP_MPEG1|
			VIDEO_CAP_MPEG2|
			VIDEO_CAP_SYS|
			VIDEO_CAP_PROG|
			VIDEO_CAP_SPU|
			VIDEO_CAP_NAVI|
			VIDEO_CAP_CSS;
		
                        
		if (copy_to_user(parg, &cap, 
				 sizeof(cap)))
			return -EFAULT;
		break;
	}

	case VIDEO_SET_STREAMTYPE:
	{
		int f = -1;
		switch(arg){
		case VIDEO_CAP_MPEG1:
		case VIDEO_CAP_MPEG2:
			f = stream_PES;
			break;
			
		case VIDEO_CAP_SYS:
		case VIDEO_CAP_PROG:
			f = stream_PS;
			break;
			
		case VIDEO_CAP_SPU:
		case VIDEO_CAP_NAVI:
		case VIDEO_CAP_CSS:
			f = stream_DVD;
		}   
		card->setup.streamtype =  f;
		
	}			
	break;
	
	case VIDEO_SET_ID:
		card->setup.videoID = arg;
		DecoderPrepareVideo(card);
		break;
		
	case VIDEO_SET_SYSTEM:
		card->videomode = (videosystem) arg;
		SetVideoSystem(card);
		break;
		
	case VIDEO_SET_HIGHLIGHT:
	{
		uint8_t data1[4];
		uint8_t data2[6];
		video_highlight_t vh;
		
		if(copy_from_user(&vh, parg, sizeof(video_highlight_t)))
			return -EFAULT;
		
		data1[0] = vh.contrast1;
		data1[1] = vh.contrast2;
		data1[2] = vh.color1;
		data1[3] = vh.color2;
		data2[0] = vh.ypos & 0xFF;
		data2[1] = (uint8_t) ((vh.ypos >> 1) & 0xFF);
		data2[2] = (uint8_t) ((vh.ypos >> 2) & 0xFF);
		data2[3] = vh.xpos & 0xFF;
		data2[4] = (uint8_t) ((vh.xpos >> 1) & 0xFF);
		data2[5] = (uint8_t) ((vh.xpos >> 2) & 0xFF);
		return DecoderHighlight(card, vh.active, data1, data2);
		break;
	}
	
	case VIDEO_SET_SPU:
	{
		video_spu_t spu;
		
		if(copy_from_user(&spu, parg, sizeof(video_spu_t)))
			return -EFAULT;
		
		return DecoderSPUStream(card, spu.stream_id, spu.active);
		break;
	}
	
	case VIDEO_SET_SPU_PALETTE:
	{
		video_spu_palette_t spup;
		
		if(copy_from_user(&spup, parg, sizeof(video_spu_palette_t)))
			return -EFAULT;
		
		return DecoderSPUPalette(card, spup.length, spup.palette);
		break;
	}
	
	case VIDEO_GET_NAVI:
	{
		video_navi_pack_t navi;
		
		navi.length = DecoderGetNavi(card, (u8 *)&(navi.data));
		if(copy_to_user(parg, &navi, sizeof(video_navi_pack_t)))
			return -EFAULT;
	}
	break;
	
	case VIDEO_SET_ATTRIBUTES:
	{
		if (!card->ChannelBuffersAllocated) {
			DecoderStreamReset(card);
			MargiFlush(card);
			
			card->setup.streamtype = stream_DVD;
			card->setup.videoID = 0;
			DecoderPrepareVideo(card);
			DecoderPreparePS(card, 0, 0, 2, 2, 3, 1);
		}
		
		SetVideoAttr(card, arg);
		card->startingDVDV = 1;	
	}
	break;
	
        case VIDEO_CLEAR_BUFFER:
		/* FIXME */
		break;

	default:
		return -ENOIOCTLCMD;
	}
	return 0;
}

static int 
dvb_audio_ioctl(struct inode *inode, struct file *file,
		unsigned int cmd, void *parg)
{               
	struct dvb_device *dvbdev=*(struct dvb_device **) file->private_data;
        unsigned long arg=(unsigned long) parg;
	struct cvdv_cards *card=(struct cvdv_cards *) dvbdev->priv;
	uint16_t attr;

	if (((file->f_flags&O_ACCMODE)==O_RDONLY) &&
	    (cmd!=AUDIO_GET_STATUS))
		return -EPERM;

	switch (cmd) {

	case AUDIO_STOP:
		if (card->audiostate.stream_source!=AUDIO_SOURCE_MEMORY)
			break;
		AudioStopDecode(card);
		card->audiostate.play_state=AUDIO_STOPPED;
		break;
		
	case AUDIO_PLAY:
		if (card->audiostate.stream_source!=AUDIO_SOURCE_MEMORY)
			break;
		AudioSetPlayMode(card, MAUDIO_PLAY);
		card->audiostate.play_state=AUDIO_PLAYING;
		break;
		
	case AUDIO_PAUSE:
		card->audiostate.play_state=AUDIO_PAUSED;
		AudioSetPlayMode(card, MAUDIO_PAUSE);
		break;
		
	case AUDIO_CONTINUE:
		if (card->audiostate.play_state==AUDIO_PAUSED) {
			card->audiostate.play_state=AUDIO_PLAYING;
			AudioSetPlayMode(card, MAUDIO_PLAY);
		} 
		break;
		
	case AUDIO_SELECT_SOURCE:
		card->audiostate.stream_source=
			(audio_stream_source_t) arg;
		break;
		
	case AUDIO_SET_MUTE:
	{
		AudioMute(card, arg);
		card->audiostate.mute_state=(int) arg;
		break;
	}
	
	case AUDIO_SET_AV_SYNC:
		card->videosync=(int) arg;
		card->audiostate.AV_sync_state=(int) arg;
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
	
	case AUDIO_CLEAR_BUFFER:
		/* FIXME */
		break;

	case AUDIO_SET_STREAMTYPE:
	{
		int f = -1;
		
		switch(arg){
		case AUDIO_CAP_DTS:
		case AUDIO_CAP_MP3:
		case AUDIO_CAP_AAC:
		case AUDIO_CAP_SDDS:
		case AUDIO_CAP_OGG:
			f = audio_none;    
			break;
			
		case AUDIO_CAP_LPCM:
			f = audio_LPCM;
			break;
			
		case AUDIO_CAP_MP1:
		case AUDIO_CAP_MP2:
			f = audio_MPEG;
			break;
			
		case AUDIO_CAP_AC3:
			f = audio_AC3;
			break;
		}
		
		card->setup.audioselect = (audio_type) f;
		DecoderPrepareAudio(card);
		break;
	}
	
	case AUDIO_SET_ID:
		if (arg < 0 || arg >32) arg = 0;
		card->setup.audioID = arg;
		arg = 0;

	case AUDIO_SET_EXT_ID:
		if (arg < 0 || arg >32) arg = arg&0x0F;
		card->setup.audioIDext = arg;
		
		attr = card->lastaattr;
		DecoderSelectAudioID(card);
		card->lastaattr = attr;
		break;
		
	case AUDIO_SET_MIXER:
		return -EINVAL;
		
	case AUDIO_SET_ATTRIBUTES:
		SetAudioAttr(card,arg);
		card->startingDVDA = ((card->setup.audioselect != audio_none)
				      && (card->setup.audioselect != 
					  audio_disable));
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

static int
dvb_osd_ioctl(struct inode *inode, struct file *file,
            unsigned int cmd, void *parg)
{
	struct dvb_device *dvbdev=*(struct dvb_device **) file->private_data;
	struct cvdv_cards *card=(struct cvdv_cards *) dvbdev->priv;

	switch (cmd) {
	case OSD_SEND_CMD:
	{
		osd_cmd_t doc;
		
		if(copy_from_user(&doc, parg, 
				  sizeof(osd_cmd_t)))
			return -EFAULT;
		return OSD_DrawCommand(card, &doc);
	}
	default:
		return -EINVAL;
	}
}


static unsigned int dvb_video_poll(struct file *file, poll_table *table)
{
        struct dvb_device *dvbdev=*(struct dvb_device **) file->private_data;
	struct cvdv_cards *card=(struct cvdv_cards *) dvbdev->priv;

	if (card != NULL) {
		poll_wait(file, &card->wqA , table);
		if  (  !card->rbufA.buffy || ring_write_rest(&(card->rbufA)) )
			return (POLLOUT | POLLWRNORM);	
		else {
			return 0;
		}
	} else
		return POLLERR;
}

static unsigned int dvb_audio_poll(struct file *file, poll_table *table)
{
        struct dvb_device *dvbdev=*(struct dvb_device **) file->private_data;
	struct cvdv_cards *card=(struct cvdv_cards *) dvbdev->priv;

	if (card != NULL) {
		poll_wait(file, &card->wqB, table);
		if  (  !card->rbufB.buffy || ring_write_rest(&(card->rbufB)) )
			return (POLLOUT | POLLWRNORM);	
		else {
			return 0;
		}
	} else
		return POLLERR;
}

static struct file_operations dvb_video_fops = {
        owner:          THIS_MODULE,
        read:           0,
        write:          dvb_video_write,
        ioctl:          dvb_generic_ioctl,
        open:           dvb_video_open,
        release:        dvb_video_release,
        poll:           dvb_video_poll,
};

static struct dvb_device dvbdev_video = {
        id: -1,
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

static struct dvb_device dvbdev_audio = {
        id: -1,
        priv: 0,
        users: 1,
        writers: 1,
        fops: &dvb_audio_fops,
        kernel_ioctl: dvb_audio_ioctl,
};

static struct file_operations dvb_osd_fops = {
        owner:          THIS_MODULE,
        read:           0,
        write:          0,
        ioctl:          dvb_generic_ioctl,
        open:           dvb_generic_open,
        release:        dvb_generic_release,
        poll:           0,
};

static struct dvb_device dvbdev_osd = {
        id: -1,
        priv: 0,
        users: 1,
        writers: 1,
        fops: &dvb_osd_fops,
        kernel_ioctl: dvb_osd_ioctl,
};

/******************************************************************************
 * driver registration 
 ******************************************************************************/

static int 
dvb_start_feed(struct dvb_demux_feed *dvbdmxfeed)
{
        struct dvb_demux *dvbdmx=dvbdmxfeed->demux;
        struct cvdv_cards * card = (struct cvdv_cards *)dvbdmx->priv;
 
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
					       
					       dvb_filter_setup_ts2pes(
							     &card->tsa, 
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
dvb_stop_feed(struct dvb_demux_feed *dvbdmxfeed)
{
        struct dvb_demux *dvbdmx=dvbdmxfeed->demux;
        struct cvdv_cards * card = (struct cvdv_cards *)dvbdmx->priv;
        if (!card)
                return -EINVAL;

        if (dvbdmxfeed->type == DMX_TYPE_TS) {
		if ((dvbdmxfeed->ts_type & TS_DECODER) 
		    && (dvbdmxfeed->pes_type<=1)) {
			if (dvbdmx->playing) {
				dvb_filter_ipack_free(&card->tsa);
				dvb_filter_ipack_free(&card->tsv);
				DecoderPause(card);
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

static uint16_t get_pid(uint8_t *pid)
{
	uint16_t pp = 0;

	pp = (pid[0] & PID_MASK_HI)<<8;
	pp |= pid[1];

	return pp;
}


static int 
dvb_write_to_decoder(struct dvb_demux_feed *dvbdmxfeed, uint8_t *buf, size_t count)
{
        struct dvb_demux *dvbdmx=dvbdmxfeed->demux;
        struct cvdv_cards * card = (struct cvdv_cards *)dvbdmx->priv;
	uint16_t pid = 0;
	int off = 0;

	ipack *p;

        if (!card)
                return -EINVAL;
	
	pid = get_pid(buf+1);
			
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
		MDEBUG(0,"AUDIO count: %d  off: %d\n",count,off);
		margi_write_audio(card, buf+off+4, TS_SIZE-4-off, 0);
	} else {
		MDEBUG(0,"VIDEO count: %d  off: %d\n",count,off);
		margi_write(card, buf+off+4, TS_SIZE-4-off, 0);
	}

	dvb_filter_ts_to_pes( p, buf); // don't need count (=188)
        return 0;
}

int dvb_register(struct cvdv_cards *card)
{
        struct dvb_device *dvbd=&card->dvb_dev;
	struct dvb_demux *dvbdemux = (struct dvb_demux *)&card->demux;
        int ret;

        if (card->dvb_registered)
                return -1;
	if (dvb_register_adapter(&card->dvb_adapter,"Margi")<0)
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
	
	dvb_dmx_init(&card->demux);

	card->dmxdev.filternum=256;
	card->dmxdev.demux=&dvbdemux->dmx;
	card->dmxdev.capabilities=0;

	dvb_dmxdev_init(&card->dmxdev, card->dvb_adapter);
        
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

        dvbd->priv=(void *) card;

	dvb_register_device(card->dvb_adapter, &card->video_dev, &dvbdev_video, 
			    card, DVB_DEVICE_VIDEO);
	dvb_register_device(card->dvb_adapter, &card->audio_dev, &dvbdev_audio, 
			    card, DVB_DEVICE_AUDIO);
	dvb_register_device(card->dvb_adapter, &card->osd_dev,   &dvbdev_osd,   
			    card, DVB_DEVICE_OSD);
        
	return 0;

}

void
dvb_unregister(struct cvdv_cards *dvb)
{
        struct dvb_demux *dvbdemux=&dvb->demux;

        if (!dvb->dvb_registered)
                return;
        dvbdemux->dmx.close(&dvbdemux->dmx);
        dvbdemux->dmx.remove_frontend(&dvbdemux->dmx, &dvb->mem_frontend);

        dvb_dmxdev_release(&dvb->dmxdev);
        dvb_dmx_release(&dvb->demux);

	dvb_unregister_device(dvb->audio_dev);
	dvb_unregister_device(dvb->video_dev);
	dvb_unregister_device(dvb->osd_dev);

        dvb_unregister_adapter(dvb->dvb_adapter);
}
