/* 
    decoder.c

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

#define __NO_VERSION__

#include "decoder.h"
#include "l64021.h"
#include "video.h"
#include "audio.h"
#include "streams.h"
#include "osd.h"
#include "dram.h"
#include "cvdv.h"

int DecoderGetNavi(struct cvdv_cards *card, u8 *navidata) 
{
	if (card->navihead == card->navitail) return 0;
	MDEBUG(3, ": Retreiving NaviPack\n");
	memcpy(navidata, &card->navibuffer[card->navitail], NAVISIZE);
	card->navitail += NAVISIZE;
	if (card->navitail >= NAVIBUFFERSIZE) card->navitail = 0;
	return NAVISIZE;
}

// returns 1 on overrun, 0 on no error
int DecoderQueueNavi(struct cvdv_cards *card, u8 *navidata) 
{
	memcpy(&card->navibuffer[card->navihead], navidata, NAVISIZE);
	card->navihead += NAVISIZE;
	if (card->navihead >= NAVIBUFFERSIZE) card->navihead = 0;
	if (card->navihead == card->navitail) {
		MDEBUG(3, ": NaviPack buffer overflow\n");
		card->navitail += NAVISIZE;
		if (card->navitail >= NAVIBUFFERSIZE) card->navitail = 0;
		return 1;
	}
	return 0;
}

u32 ParseSCR(const u8 *data) 
{
	u32 SCR_base=0;
	u8 scrdata[9];
	copy_from_user (scrdata, data, 9);

	if ((!scrdata[0]) && (!scrdata[1]) && (scrdata[2]==1) 
	    && (scrdata[3]==0xBA) && ((scrdata[4]&0xC0)==0x40)) {
		SCR_base=((scrdata[4]>>3)&0x07);
		SCR_base=(SCR_base<<2) | (scrdata[4]&0x03);
		SCR_base=(SCR_base<<8) | scrdata[5];
		SCR_base=(SCR_base<<5) | ((scrdata[6]>>3)&0x1F);
		SCR_base=(SCR_base<<2) | (scrdata[6]&0x03);
		SCR_base=(SCR_base<<8) | scrdata[7];
		SCR_base=(SCR_base<<5) | ((scrdata[8]>>3)&0x1F);
	}
	return SCR_base;
}

u32 SetSCR(struct cvdv_cards *card, u32 SCR_base) 
{
	MDEBUG(3, ": SCR in DVD Pack: 0x%08X\n",SCR_base);
	if (DecoderReadByte(card, 0x007) & 0x10) {  // SCR already stopped
		DecoderWriteByte(card,0x009,SCR_base&0xFF);  // Set SCR counter
		DecoderWriteByte(card,0x00A,(SCR_base>>8)&0xFF);
		DecoderWriteByte(card,0x00B,(SCR_base>>16)&0xFF);
		DecoderWriteByte(card,0x00C,(SCR_base>>24)&0xFF);
	} else {
		DecoderMaskByte(card,0x007,0xD2,0xD2);   
                // Set 0x10, halt SCR counter
		DecoderWriteByte(card,0x009,SCR_base&0xFF);  // Set SCR counter
		DecoderWriteByte(card,0x00A,(SCR_base>>8)&0xFF);
		DecoderWriteByte(card,0x00B,(SCR_base>>16)&0xFF);
		DecoderWriteByte(card,0x00C,(SCR_base>>24)&0xFF);
		DecoderMaskByte(card,0x007,0xD2,0xC2);   
               // Del 0x10, SCR counter run
	}
	return SCR_base;
}

void DecoderPause(struct cvdv_cards *card) 
{
	DecoderMaskByte(card, 0x007, 0xD2, 0xD2);   
        // Set 0x010, halt SCR counter
	AudioSetPlayMode(card, MAUDIO_PAUSE);
	DecoderStopDecode(card);
	card->videostate.play_state=VIDEO_FREEZED;
	card->videoffwd = 0;
	card->videoslow = 0;
}

void DecoderUnPause(struct cvdv_cards *card) 
{
	DecoderStartDecode(card);
	card->videoffwd = 0;
	AudioSetPlayMode(card, MAUDIO_PLAY);
	DecoderMaskByte(card, 0x007, 0xD2, 0xC2);   
        // Del 0x010, SCR counter run
	card->videostate.play_state=VIDEO_PLAYING;;
	card->videoslow = 0;
}

void CloseCard(struct cvdv_cards *card) 
{
#ifdef NOINT
	spin_lock(&card->timelock);
	del_timer(&card->timer);
	spin_unlock(&card->timelock);
#endif
	MargiFlush(card);
	MDEBUG(1, ": Closing card\n");
	card->DecoderOpen = 1;
	DecoderClose(card);
	DecoderUnPrepare(card);
	DecoderStreamReset(card);
	DecoderSetupReset(card);
	VideoSetBackground(card, 1, 0, 0, 0);  

	AudioClose(card);
	OSDClose(card);
	L64021Init(card);
	MargiFreeBuffers(card);

	OSDOpen(card, 50, 50, 150, 150, 2, 1);
	OSDTest(card);
}


void DecoderReadAudioInfo(struct cvdv_cards *card) 
{
	u8 data;
	static int bitrates[17] = {0, 32, 40, 48, 56, 64, 80, 96, 112, 
				   128, 160, 192, 224, 256, 320, 384, 0};
	struct AudioParam *audio = &card->stream.audio;
	data = DecoderReadByte(card, 0x150);
	audio->mpeg.present = data & 0x60;  
        // MPEG Layer Code 00 reserverd, we can assume valid MPEG params
	if (audio->mpeg.present) {
		audio->mpeg.MPEG2 = data & 0x80;
		audio->mpeg.layer = 4 - ((data >> 5) & 0x03);
		if (data & 0x0F) {
			if ((data & 0x0F) == 1) audio->mpeg.bitrate = 32; 
			else switch (audio->mpeg.layer) {
			case 1: 
				audio->mpeg.bitrate = 32 * (data & 0x0F); 
				break;  // Layer I
			case 2: 
				audio->mpeg.bitrate = bitrates[(data & 0x0F) +
							      1]; 
				break;  // Layer II
			default: 
				audio->mpeg.bitrate = bitrates[data & 0x0F];
				// Layer III
			}
		} else audio->mpeg.bitrate = 0;
		data = DecoderReadByte(card, 0x151);
		switch ((data >> 6) & 0x03) {
		case 0: 
			audio->mpeg.samplefreq = 44; 
			break;
		case 1: 
			audio->mpeg.samplefreq = 48; 
			break;
		case 2: 
			audio->mpeg.samplefreq = 32; 
			break;
		default: 
			audio->mpeg.samplefreq = 0; // invalid
		}
		audio->mpeg.mode = (data >> 3) & 0x03;
		audio->mpeg.modeext = (data >> 1) & 0x03;
		audio->mpeg.copyright = data & 0x01;
		data=DecoderReadByte(card, 0x152);
		audio->mpeg.original = data & 0x80;
		audio->mpeg.emphasis = (data >> 5) & 0x03;
	}
	data = DecoderReadByte(card, 0x153);
	audio->ac3.present = (data != 0);  
	// value 0 for bits 0..5 forbidden, we can assume valid ac3 params
	if (audio->ac3.present) {
		audio->ac3.acmod = (data >> 5) & 0x07;
		audio->ac3.dialnorm = data & 0x1F;
		data = DecoderReadByte(card, 0x154);
		audio->ac3.bsmod = (data >> 5) & 0x07;
		audio->ac3.dialnorm2 = data > 0x1F;
		data = DecoderReadByte(card, 0x155);
		audio->ac3.surmixlev = (data >> 6) & 0x03;
		audio->ac3.mixlevel = (data >> 1) & 0x1F;
		data = DecoderReadByte(card, 0x156);
		audio->ac3.cmixlev = (data >> 6) & 0x03;
		audio->ac3.mixlevel2 = (data >> 1) & 0x1F;
		data = DecoderReadByte(card, 0x157);
		audio->ac3.fscod = (data >> 6) & 0x03;
		audio->ac3.lfeon = (data >> 5) & 0x01;
		audio->ac3.bsid = data & 0x1F;
		data = DecoderReadByte(card, 0x158);
		audio->ac3.dsurmod = (data >> 6) & 0x03;
		audio->ac3.frmsizecod = data & 0x3F;
		audio->ac3.langcod = DecoderReadByte(card, 0x159);
		audio->ac3.langcod2 = DecoderReadByte(card, 0x15A);
		audio->ac3.timecod = DecoderReadByte(card, 0x15B);
		data = DecoderReadByte(card, 0x15C);
		audio->ac3.timecod = (audio->ac3.timecod << 6) | 
		  ((data >> 2) & 0x3F);
		audio->ac3.roomtyp = data & 0x03;
		audio->ac3.timecod2 = DecoderReadByte(card, 0x15D);
		data = DecoderReadByte(card, 0x15E);
		audio->ac3.timecod2 = (audio->ac3.timecod2 << 6) | 
		  ((data >> 2) & 0x3F);
		audio->ac3.roomtyp2 = data & 0x03;
	}
	audio->pcm.present =! (DecoderReadByte(card, 0x161) & 0x20);  
	// PCM FIFO not empty? Then, we can assume valid LPCM params
	if (audio->pcm.present) {
		data = DecoderReadByte(card, 0x15F);
		audio->pcm.audio_frm_num = (data >> 3) & 0x1F;
		audio->pcm.num_of_audio_ch = data & 0x07;
		data = DecoderReadByte(card, 0x160);
		audio->pcm.Fs = (data >> 6) & 0x03;
		audio->pcm.quantization = (data >> 4) & 0x03;
		audio->pcm.emphasis = (data >> 2) & 0x03;
		audio->pcm.mute_bit = (data >> 1) & 0x01;
	}
	switch (card->setup.audioselect) {
	case audio_disable:
		audio->valid = 0;
		break;
	case audio_none:
	case audio_DTS:
	case audio_SDDS:
		if ((audio->valid = (audio->ac3.present || 
				     audio->pcm.present || 
				     audio->mpeg.present))) {
			if (audio->mpeg.present) {
				card->setup.audioselect = audio_MPEG;
			} else if (audio->pcm.present) {
				card->setup.audioselect = audio_LPCM;
			} else if (audio->ac3.present) {
				card->setup.audioselect = audio_AC3;
			} 
		} else {
			audio->valid = 0;
			card->setup.audioselect = audio_none;
		}
		break;
	case audio_MPEG:  // MPEG Audio
	case audio_MPEG_EXT:  // MPEG Audio with extension stream
		audio->valid = audio->mpeg.present;
		break;
	case audio_LPCM:  // Linear Pulse Code fe_modulation_t LPCM
		audio->valid = audio->pcm.present;
		break;
	case audio_AC3:  // AC-3
		audio->valid = audio->ac3.present;
		break;
	}
	MDEBUG(1, ": -- DecoderReadAudioInfo - type/valid %d/%d:\n", card->setup.audioselect, audio->valid);
	if (audio->mpeg.present || audio->ac3.present || audio->pcm.present)
		MDEBUG(1, ": Audio - Decoded parameters:\n");
	if (audio->mpeg.present) MDEBUG(1, ":   MPEG%s Layer %d, %d kHz, %d kbps, %s, %s%s, %s emphasis\n", 
					((audio->mpeg.MPEG2) ? "2" : "1"), 
					audio->mpeg.layer, 
					audio->mpeg.samplefreq, 
					audio->mpeg.bitrate, 
					((audio->mpeg.mode == 0) ? "stereo" : ((audio->mpeg.mode == 1) ? "joint stereo" : ((audio->mpeg.mode == 2) ? "dual channel" : "single channel"))), 
					((audio->mpeg.copyright) ? "copyrighted " : ""), 
					((audio->mpeg.original) ? "original" : "copy"), 
					((audio->mpeg.emphasis == 0) ? "no" : ((audio->mpeg.emphasis == 1) ? "50/15 usec." : ((audio->mpeg.emphasis == 2) ? "invalid" : "J.17")))
		);
	if (audio->ac3.present) MDEBUG(1, ":   AC3 acmod=%d bsmod=%d dialnorm=%d dialnorm2=%d surmixlev=%d mixlevel=%d cmixlev=%d mixlevel2=%d fscod=%d lfeon=%d bsid=%d dsurmod=%d frmsizecod=%d langcod=%d langcod2=%d timecod=%d roomtyp=%d timecod2=%d roomtyp2=%d\n", 
				       audio->ac3.acmod, 
				       audio->ac3.bsmod, 
				       audio->ac3.dialnorm, 
				       audio->ac3.dialnorm2, 
				       audio->ac3.surmixlev, 
				       audio->ac3.mixlevel, 
				       audio->ac3.cmixlev, 
				       audio->ac3.mixlevel2, 
				       audio->ac3.fscod, 
				       audio->ac3.lfeon, 
				       audio->ac3.bsid, 
				       audio->ac3.dsurmod, 
				       audio->ac3.frmsizecod, 
				       audio->ac3.langcod, 
				       audio->ac3.langcod2, 
				       audio->ac3.timecod, 
				       audio->ac3.roomtyp, 
				       audio->ac3.timecod2, 
				       audio->ac3.roomtyp2);
	if (audio->pcm.present) MDEBUG(1, ":   LPCM audio_frm_num=%d num_of_audio_ch=%d Fs=%d quantization=%d emphasis=%d mute_bit=%d\n", 
				       audio->pcm.audio_frm_num, 
				       audio->pcm.num_of_audio_ch, 
				       audio->pcm.Fs, 
				       audio->pcm.quantization, 
				       audio->pcm.emphasis, 
				       audio->pcm.mute_bit);
}

void DecoderReadAuxFifo(struct cvdv_cards *card) 
{
	int i = 0;
	u8 data;
	int layer;

	struct StreamInfo *stream = &card->stream;
	MDEBUG(3, ": AUX - %03X ", card->AuxFifo[card->AuxFifoTail]);
        while (card->AuxFifoHead != card->AuxFifoTail) {
		
		layer = (card->AuxFifo[card->AuxFifoTail] >> 8) & 0x07;
		data = card->AuxFifo[card->AuxFifoTail] & 0xFF;
		card->AuxFifoTail = (card->AuxFifoTail + 1) & FIFO_MASK;
		if (layer != card->AuxFifoLayer) {  // start of a new layer?
			i = 0;
			card->AuxFifoLayer = layer;
		} else i++;
		switch (layer) {  // layer code
		case 0:  // sequence header
			if (! stream->sh.valid) switch (i) {
			case 0: 
				stream->sh.hsize = data & 0x0F; 
				break;
			case 1: 
				stream->sh.hsize = (stream->sh.hsize << 8)
					| data; 
				stream->hsize =	stream->sh.hsize; 
				break;
			case 2: 
				stream->sh.vsize = data & 0x0F; 
				break;
			case 3: 
				stream->sh.vsize = (stream->sh.vsize << 8) | 
					data; 
				stream->vsize = stream->sh.vsize; 
				break;
			case 4: 
				stream->sh.aspectratio = data & 0x0F; 
				break;
			case 5: 
				stream->sh.frameratecode = data & 0x0F; 
				break;
			case 6: 
				stream->sh.bitrate = data & 0x03; 
				break;
			case 7: 
				stream->sh.bitrate = (stream->sh.bitrate << 8)
					| data; 
				break;
			case 8: 
				stream->sh.bitrate = (stream->sh.bitrate << 8)
					| data; 
				stream->bitrate = stream->sh.bitrate; 
				break;
			case 9: 
				stream->sh.vbvbuffersize = data & 0x03; 
				break;
			case 10: 
				stream->sh.vbvbuffersize = 
					(stream->sh.vbvbuffersize << 8) | 
					data; 
				stream->vbvbuffersize = 
					stream->sh.vbvbuffersize; 
				break;
			case 11: 
				stream->sh.constrained = data & 0x01; 
				stream->sh.valid = 1;
				MDEBUG(1, ": AUX - MPEG1 - %dx%d %s %s fps, %d bps, %d kByte vbv%s\n", stream->sh.hsize, stream->sh.vsize, 
				       ((stream->sh.aspectratio == 1) ? "1:1" : 
					((stream->sh.aspectratio == 2) ? "3:4" : 
					 ((stream->sh.aspectratio == 3) ? "9:16" : 
					  ((stream->sh.aspectratio == 4) ? "1:2.21" : 
					   "?:?")))), 
				       ((stream->sh.frameratecode == 1) ? "23.976" : 
					((stream->sh.frameratecode == 2) ? "24" : 
					 ((stream->sh.frameratecode == 3) ? "25" : 
					  ((stream->sh.frameratecode == 4) ? "29.97" : 
					   ((stream->sh.frameratecode == 5) ? "30" : 
					    ((stream->sh.frameratecode == 6) ? "50" : 
					     ((stream->sh.frameratecode == 7) ? "59.94" : 
					      ((stream->sh.frameratecode == 8) ? "60" : 
					       "?")))))))), 
				       stream->sh.bitrate * 400, 
				       stream->sh.vbvbuffersize * 16, 
				       ((stream->sh.constrained) ? ", constrained" : "")
					);
				break;
			}
			break;
		case 1:  // group of pictures
			if (! stream->gop.valid) 
				switch (i) {
				case 0: 
					stream->gop.timecode = data & 0x01; 
					break;
				case 1: 
					stream->gop.timecode = 
						(stream->gop.timecode << 8) | 
						data; 
					break;
				case 2: 
					stream->gop.timecode = 
						(stream->gop.timecode << 8) | 
						data; 
					break;
				case 3: 
					stream->gop.timecode = 
						(stream->gop.timecode << 8) | 
						data; 
					break;
				case 4: 
					stream->gop.closedgop = data & 0x01; 
					break;
				case 5: 
					stream->gop.brokenlink = data & 0x01;
					stream->gop.valid = 1;
					break;
				}
			break;
		case 2:  // picture
			if (0) 
				switch (i) {
				case 0: 
					break;
				}
			break;
		case 7:  // extension layer
			if (i == 0) card->AuxFifoExt = data;
			else 
				switch (card->AuxFifoExt) {  // extension code
				case 1:  // sequence extension
					if ((stream->sh.valid) && 
					    (! stream->se.valid))
						switch (i) {
						case 1: 
							stream->se.profilelevel
								= data; 
							break;
						case 2: 
							stream->se.progressive
								= data & 0x01; 
							break;
						case 3: 
							stream->se.chroma = 
								(data >> 4) & 
								0x03; 
							stream->se.hsizeext = 
								(data >> 2) & 
								0x03;
							stream->se.vsizeext = 
								data & 0x03;
							stream->hsize |= 
								(stream->se.hsizeext << 12);
							stream->vsize |= 
								(stream->se.vsizeext << 12);
							break;
						case 4: 
							stream->se.bitrateext =
								data & 0x0F; 
							break;
						case 5: 
							stream->se.bitrateext =
								(stream->se.bitrateext << 8) | data; 
							stream->bitrate |= 
								(stream->se.bitrateext << 18); 
							break;
						case 6: 
							stream->se.vbvbuffersizeext = data; 
							stream->vbvbuffersize |= (stream->se.vbvbuffersizeext << 10); 
							break;
						case 7:
							stream->se.lowdelay =
								(data >> 7) & 
								0x01;
							stream->se.frextn = 
								(data >> 5) & 
								0x03;
							stream->se.frextd = 
								data & 0x1F;
							stream->se.valid = 1;
							stream->MPEG2 = 1;
							MDEBUG(1, ": AUX - MPEG2 - %dx%d %s %s*%d/%d fps, %d bps, %d kByte vbv%s%s\n", stream->hsize, stream->vsize, 
							       ((stream->sh.aspectratio == 1) ? "1:1" : 
								((stream->sh.aspectratio == 2) ? "3:4" : 
								 ((stream->sh.aspectratio == 3) ? "9:16" : 
								  ((stream->sh.aspectratio == 4) ? "1:2.21" : 
								   "?:?")))), 
							       ((stream->sh.frameratecode == 1) ? "23.976" : 
								((stream->sh.frameratecode == 2) ? "24" : 
								 ((stream->sh.frameratecode == 3) ? "25" : 
								  ((stream->sh.frameratecode == 4) ? "29.97" : 
								   ((stream->sh.frameratecode == 5) ? "30" : 
								    ((stream->sh.frameratecode == 6) ? "50" : 
								     ((stream->sh.frameratecode == 7) ? "59.94" : 
								      ((stream->sh.frameratecode == 8) ? "60" : 
								       "?")))))))), 
							       stream->se.frextn + 1, 
							       stream->se.frextd + 1, 
							       stream->bitrate * 400, 
							       stream->vbvbuffersize * 16, 
							       ((stream->sh.constrained) ? ", constrained" : ""), 
							       ((stream->se.lowdelay) ? ", low delay" : "")
								);
							break;
						}
					break;
				case 2:  // sequence display extension
					if (0)
						switch (i) {
						case 0: 
							break;
						}
					break;
				case 3:  // quant matrix extension
					if (0) 
						switch (i) {
						case 0: 
							break;
						}
					break;
				case 4:  // copyright  extension
					if (0) 
						switch (i) {
						case 0: 
							break;
						}
					break;
				case 7:  // picture display extension
					if (0) switch (i) {
					case 0: 
						break;
					}
					break;
				case 8:  // picture coding extension
					if (0) 
						switch (i) {
						case 0: 
							break;
						}
					break;
				default:
					break;
				}
			break;
		default:break;
		}
		
	}  
}

void DecoderReadDataFifo(struct cvdv_cards *card) 
{
        MDEBUG(3, ": DATA - ");
	while (card->DataFifoHead != card->DataFifoTail) {
	        MDEBUG(3,"%03X ", card->DataFifo[card->DataFifoTail]);
		card->DataFifoTail = (card->DataFifoTail + 1) & FIFO_MASK;
	}
	MDEBUG(3,"\n");
}

int DecoderReadNavipack(struct cvdv_cards *card) 
{
	u32 startaddr, endaddr, writeaddr;
	u8 navipack[1024];
	u16 PacketLength;
	u8 SubStreamID;
	//struct Navi navi;
	int i;
	startaddr = (DecoderReadWord(card, 0x05C) & 0x3FFF) << 7;   
        // 21 bit word address
	endaddr = (DecoderReadWord(card, 0x05E) & 0x3FFF) << 7;     
        // 21 bit word address
	writeaddr = DecoderReadByte(card, 0x075) & 0xFF;
	writeaddr |= (DecoderReadWord(card, 0x077) & 0x0FFF) << 8;
	//writeaddr <<= 3;
	MDEBUG(3, ": -- DecoderReadNavipack 0x%08X-0x%08X, ->0x%08X <-0x%08X\n", 
	       startaddr, endaddr, writeaddr, card->NaviPackAddress);
	
	if (DecoderReadByte(card, 0x07B) & 0xC0) {  // navi pack available?
		DRAMReadByte(card, card->NaviPackAddress, 1024, navipack, 0);
		card->reg07B |= 0x20;  // decrement navi counter
		DecoderWriteByte(card, 0x07B, card->reg07B);
		card->reg07B &= ~0x20;
		//DecoderSetByte(card, 0x07B, 0x20);  // decrement navi counter
		card->NaviPackAddress += 512;       // increment in words
		if (card->NaviPackAddress >= endaddr) 
			card->NaviPackAddress = startaddr;
		MDEBUG(4, ": Navipack %02X %02X %02X %02X  %02X %02X %02X %02X\n", 
		       navipack[0], navipack[1], navipack[2], navipack[3], navipack[4], 
		       navipack[5], navipack[6], navipack[7]);
		if ((!navipack[0]) && (!navipack[1]) && (navipack[2] == 1) && 
		    (navipack[3] == 0xBF)) {
			PacketLength = (navipack[4] << 8) | navipack[5];
			SubStreamID = navipack[6];
			MDEBUG(4, ": Navipack Len=%d, ID=%d\n", PacketLength, SubStreamID);
			i = 7;  // start of payload data in navipack[]
			switch (SubStreamID) {
			case 0:  // Presentation Control Information (PCI)
				if (PacketLength < 980) return 1;  // Packet too small
				DecoderQueueNavi(card, navipack);
				break;
			case 1:  // Data Search Information (DSI)
				if (PacketLength < 1018) return 1;  // Packet too small
				DecoderQueueNavi(card, navipack);
				break;
			default:
				break;
			}
		} else {
			MDEBUG(4, "navipack format error:%02X %02X %02X %02X %02X %02X %02X %02X\n",
			       navipack[0], navipack[1], navipack[2], navipack[3], navipack[4], 
			       navipack[5], navipack[6], navipack[7]);
		}
	} else {
		MDEBUG(4, ": no navi pack avail.\n");
	}
	return 0;
}

int AudioStart(struct cvdv_cards *card) 
{
	DecoderReadAudioInfo(card);  // detect audio type
	if (card->stream.audio.valid) {
		MDEBUG(1, ": Audio Init in delayed decoder start\n");
		if (card->AudioInitialized) AudioClose(card);
		switch (card->setup.audioselect) {
		case audio_MPEG:  // MPEG Audio
		case audio_MPEG_EXT:  // MPEG Audio with ext.
			MDEBUG(1, ": Using MPEG Audio\n");
			AudioInit(card, card->stream.audio.mpeg.samplefreq, 0);
			if (card->stream.audio.mpeg.mode == 3) AudioDualMono(card, 2);  // left channel only
			else AudioDualMono(card, 0);
			break;
		case audio_DTS:
		case audio_LPCM:  // Linear Pulse Code fe_modulation_t LPCM
			MDEBUG(1, ": Using LPCM Audio\n");
			AudioInit(card, 48, 0);  // or 96
			break;
		case audio_AC3:  // AC-3
			MDEBUG(1, ": Using AC-3 Audio\n");
			switch (card->stream.audio.ac3.fscod) {
			case 0:AudioInit(card, 48, 0); break;
			case 1:AudioInit(card, 44, 0); break;
			case 2:AudioInit(card, 32, 0); break;
			}
			break;
		case audio_none:
		case audio_disable:
		case audio_SDDS:
			break;
		}
	} else return 1;
	return 0;
}

u32 DecoderReadSCR(struct cvdv_cards *card, u16 address)
{
	u32 SCR;
	SCR = DecoderReadByte(card, address);
	SCR |= ((u32)DecoderReadByte(card, address+1) << 8);
	SCR |= ((u32)DecoderReadByte(card, address+2) << 16);
	SCR |= ((u32)DecoderReadByte(card, address+3) << 24);
	return SCR;
}

u32 DecoderReadRWAddr(struct cvdv_cards *card, u16 address)
{
	u32 addr;
	addr = DecoderReadByte(card, address) & 0xFF;
	addr |= (((u32)DecoderReadByte(card, address+1) & 0xFF) << 8);
	addr |= (((u32)DecoderReadByte(card, address+2) & 0x0F) << 16);
	return addr;
}

int PTSGetFirstPTS(PTSStorage *store, u32 *PTS)
{
	if ( store->end == store->begin ) {
		return 0;
	} else {
		*PTS = store->PTS[store->begin];
		return 1;
	}
}

void PTSStoreAdd(PTSStorage *store, u32 PTS, u32 AddrB, u32 AddrE)
{
	int new;
	MDEBUG(3, ": PTSStoreAdd - store in [%d] %08X - %08X\n", store->end, AddrB, AddrE);

 // cheap fix: don't store if address rollover
	if ((AddrB & 0x00080000) != (AddrE & 0x00080000)) return;

	new = store->end;

	store->end++;
	if (store->end >= store->size) store->end = 0;
	if (store->end == store->begin) {
		store->begin++;
		if (store->begin >= store->size) store->begin = 0;
	}

	store->AddrB[new] = AddrB;
	store->AddrE[new] = AddrE;
	store->PTS[new] = PTS;
}

int PTSGetPTS (PTSStorage *store, u32 Addr, u32 *PTS )
{
	u32 AddrB;
	u32 AddrE;
	int i;
	int found;
	int search;

	MDEBUG(3, ": PTSGetPTS - search %08X\n", Addr);

	if (store->end == store->begin) {
		store->LastAddr = Addr;
		return 0;
	}

	// Search for the PTS in the array
	found = 0;
	search = 1;
	while (search && !found) {
	 // Get the first value
		i = store->begin;
		AddrB = store->AddrB[i];
		AddrE = store->AddrE[i];

		MDEBUG(3, ": PTSGetPTS - search in [%d] %08X - %08X\n", i, AddrB, AddrE);

	 //If in range, keep it
		if ((Addr >= AddrB) && (Addr <= AddrE)) {
			*PTS = store->PTS[i];
			found = 1;
		} else {
			if ((Addr & 0x00080000) == (AddrB & 0x00080000)) {
				if (Addr < AddrB ) search = 0;
			} else {
				if ((store->LastAddr & 0x00080000) == (Addr & 0x00080000)) search = 0;
			}
		}
		if (search) {
			store->begin++;
			if (store->begin >= store->size) store->begin = 0;
			if (store->end == store->begin ) search = 0;
		}
	}
	store->LastAddr = Addr;
	return found;
}


u32 GetPTS(u8 *data, u32* MediaPointer, int mpeg, int hlength,int off)
{
	u32 PTS = 0xFFFFFFFFUL;
	int p = 0;
	
	// Read PTS, if present
	if ((mpeg == 2 && data[p + 7] & 0x80) ||
	    (mpeg == 1 && off)) {
		if (mpeg == 1) p = off-9;
		PTS = (data[p + 9] >> 1) & 0x03UL;
		PTS = (PTS << 8) | (data[p + 10] & 0xFFUL);
		PTS = (PTS << 7) | ((data[p + 11] >> 1) & 0x7FUL);
		PTS = (PTS << 8) | (data[p + 12] & 0xFFULL);
		PTS = (PTS << 7) | ((data[p + 13] >> 1) & 0x7FUL);
	}
	// Now, skip rest of PES header and stuffing
	if (mpeg == 2){
		p += (9 + (data[p + 8] & 0xFF));
		p = ((p + 7) / 8) * 8;
	} else p = hlength+7;
	if (!(data[p++] | data[p++] | data[p++] | data[p++])) {
		*MediaPointer = (u32)data[p++] & 0xFF;
		*MediaPointer = (*MediaPointer << 8) | ((u32)data[p++] & 0xFF);
		*MediaPointer = (*MediaPointer << 8) | ((u32)data[p++] & 0xFF);
		*MediaPointer = (*MediaPointer << 8) | ((u32)data[p++] & 0xFF);
	} else {
		*MediaPointer = 0xFFFFFFFFUL;
	}
	return PTS;
}

int ReadPESChunk(struct cvdv_cards *card, u32 *addr, u8 *data, u32 start, u32 end)
{
	int i = 5, err = -1;
	while (err && (i--)) err &= DRAMReadByte(card, *addr << 2, 8, &data[0], 0);
	if (err) return 1;
	(*addr)++;
	if (*addr >= end) *addr = start;
	return 0;
}

void ReadPESHeaders(struct cvdv_cards *card)
{
	u8 startcode[] = {0x00, 0x00, 0x01};
	int LoopCount;
	u32 LastVAddr; // Current Video Address
	u32 LastAAddr; // Current Audio Address
	u32 Addr;      // Current Header Address
	u32 PESAddr;   // Pointer from Header Block
	u32 PTS;       // PTS from Header Block
	u8 Data[32];
	u32 AudioPESStart;
	u32 AudioPESEnd;
	int i, j, p, fail;
	u32 FailAddr;
	int hlength=0;
	int mpeg=0;
	int check;
	int mp=0;
	int off=0;
	
	AudioPESStart = (DecoderReadWord(card, 0x058) & 0x3FFF) << 5;
	AudioPESEnd = ((DecoderReadWord(card, 0x05A) & 0x3FFF) + 1) << 5;

	LastVAddr = DecoderReadRWAddr(card, 0x060);
	LastAAddr = DecoderReadRWAddr(card, 0x063);
	
	if (card->LastAddr == 0) card->LastAddr = AudioPESStart;

	//Read the PES header buffer
	Addr  = DecoderReadRWAddr(card, 0x072) & 0x0007FFFF;
	if (Addr >= AudioPESEnd) {
		Addr = card->LastAddr = AudioPESStart;
	}

	LoopCount = 0;
	while ((card->LastAddr != Addr) && (LoopCount++ < 200)) {
		FailAddr = card->LastAddr;
		fail = 0;
		p = 0;

		if (ReadPESChunk(card, &card->LastAddr, &Data[p], 
				 AudioPESStart, AudioPESEnd)) continue;
		p+=8;
		j=1;
		
		if (memcmp(Data, startcode, 3)) continue;
 		if ((Data[3] == 0xE0) || (Data[3] == 0xBD) 
		    || ((Data[3] & 0xE0) == 0xC0)) {
		  
			fail |= ReadPESChunk(card, &card->LastAddr, 
					     &Data[p], AudioPESStart, 
					     AudioPESEnd);


			p+=8;
			j++;
			if ( (Data[6] & 0xC0) == 0x80 ){
				hlength = 9+Data[8];
				mpeg = 2;
			} else {
				mpeg = 1;
				mp = 6;
				check = Data[mp];
				mp++;
				while (check == 0xFF){
					if (!fail && mp == p) {
						fail |= ReadPESChunk(
							card, 
							&card->LastAddr, 
							&Data[p], 
							AudioPESStart, 
							AudioPESEnd);
						p+=8;
						j++;
					}
					check = Data[mp];
					mp++;
				}
				if (!fail && mp == p) {
					fail |= ReadPESChunk(
						card, 
						&card->LastAddr, 
						&Data[p], 
						AudioPESStart, 
						AudioPESEnd);
					p+=8;
					j++;
				}
				
				if ( !fail && (check & 0xC0) == 0x40){
					check = Data[mp];
					mp++;
					if (!fail && mp == p) {
						fail |= ReadPESChunk(
							card, 
							&card->LastAddr, 
							&Data[p], 
							AudioPESStart, 
							AudioPESEnd);
					  p+=8;
					  j++;
					}
					check = Data[mp];
					mp++;
				}
				if ( !fail && (check & 0x20)){
					if (check & 0x30) hlength = mp+10;
					else hlength = mp+5;
					off = mp-1;
				}
			}

			for (i = 1; (i < ((hlength+7) / 8)) && (!fail);
			     i++) {
				fail |= ReadPESChunk(card, &card->LastAddr, 
						     &Data[p], AudioPESStart, 
						     AudioPESEnd);
				p+=8;
				j++;
			}

			if (!fail) {
				PTS = GetPTS(Data, &PESAddr, 
					     mpeg, hlength,off);
				if ((PTS != 0xFFFFFFFF) && 
				    (PESAddr != 0xFFFFFFFF)) {
			 		if (Data[3] == 0xE0) {  // Video
						PTSStoreAdd(&card->VideoPTSStore, PTS, PESAddr, LastVAddr);
					} else {  // Audio
						PTSStoreAdd(&card->AudioPTSStore, PTS, PESAddr, LastAAddr);
					}
				}
			}
		} else {
			//card->LastAddr = Addr;
		}
		// In case of error, rewind and try again
		if (fail) card->LastAddr = FailAddr; 
		}
}

void L64021Intr(struct cvdv_cards *card) 
{
	u32 SCR_base, SCR_compareV, SCR_compareA;
	u32 VideoAddr, AudioAddr, PTS;
	int i, a, v, as, vs, ap, vp;
	u8 intr[5];
	u8 layer;
	long ISRTime, DeltaSyncTime, Offset;
	
	int used = 0;
	u8 err;

	err = DecoderReadByte(card, 0x095);
	if (err & 0x17) {
		MDEBUG(0, ": Packet Error: 0x%02X\n", err);
	}

	ISRTime = 0;  // TODO system time
  
	for (i = 0; i < 5; i++) 
		if ((intr[i] = DecoderReadByte(card, i))) used = 1;
	if (used) {
		if (intr[0] & 0x80) {  // new field
			card->fields++;
			
			if (card->videoffwd){
				if (!card->videoffwd_last){
					AudioStopDecode(card);
					card->videosync = 0;
					card->videoskip = card->videoffwd;
					card->videoskip = 0;
					card->videoffwd_last = 1;
					card->videoskip_last = 0;
				} else {
					if (card->videoskip_last == -1){
						card->videoskip = 
							card->videoffwd;
					}
					
					if (!card->videoskip)
						card->videoskip_last = -1;
					else
						card->videoffwd_last =
							card->videoffwd;
				} 
			} else if( card->videoffwd_last ){
				card->videoffwd_last = 0;
				if (card->audiostate.AV_sync_state)
					card->videosync = 1;
				AudioStartDecode(card);
			}				
			
	
			if (card->videoslow){
				if (!card->videoslow_last){
					AudioStopDecode(card);
					card->videosync = 0;
					card->videodelay = card->videoslow;
					card->videoskip = 0;
					card->videoslow_last = 1;
					card->videodelay_last = 0;
				} else {
					if (card->videodelay_last == -1){
						card->videodelay = 
							card->videoslow;
					}
					
					if (!card->videodelay)
						card->videodelay_last = -1;
					else
						card->videodelay_last =
							card->videodelay;
				} 
			} else if( card->videoslow_last ){
				card->videoslow_last = 0;
				if (card->audiostate.AV_sync_state)
					card->videosync = 1;
				AudioStartDecode(card);
			}				
			

			if (card->videodelay > 0) {
				if( (DecoderReadByte(card, 0x0ED) & 0x03) 
				    == 0x00)	{
					card->videodelay--;
					if(card->videodelay){
						DecoderWriteByte(card, 0x0ED, 
								 0x01);
					} else {
						DecoderWriteByte(card, 0x0ED, 
								 0x00);
					}
				} else {
					card->videodelay--;
					if(!card->videodelay){
						DecoderWriteByte(card, 0x0ED, 
								 0x00);
					}
				}
			} else if (card->videoskip > 0) {
				if ((DecoderReadByte(card, 0x0EC) & 0x03) 
				    == 0x00) {
					if (DecoderReadWord(card, 0x096) > 5){
  // pictures in video ES channel
						card->videoskip--;
						if(card->videoskip) {
							DecoderWriteByte(card,
									 0x0EC
									 ,0x03);
						} else {
							DecoderWriteByte(card,
									 0x0EC
									 ,0x00);
						}
					} else {
						card->videoskip = 0;
						DecoderWriteByte (card, 0x0EC,
								  0x00);
					}
				}
			}


			i = (DecoderReadByte(card, 0x113) & 0xFC) | 
				(DecoderReadByte(card, 0x114) & 0x01);
			v = DecoderGetVideoESLevel(card);
			if (card->startingV) {
				vs = card->VideoESSize;
				if (vs > 0) vp = (100 * v) / vs;
				else vp = 0;
				if (vp > 90) {
					MDEBUG(0,": Delayed Video Decoder start\n");
					card->startingV = 0;
					DecoderStartDecode(card);
					//DecoderSetVideoPanic(card, 1, 3);  
					// video panic at 3 pictures
					//DecoderSetVideoPanic(card, 0, DecoderGetVideoESSize(card) / 4);  // video panic at 25 percent
				}
			}
			a = DecoderGetAudioESLevel(card);
			if (card->startingA) {
				as = card->AudioESSize;
				if (as > 0) ap = (100 * a) / as;
				else ap = 0;
				if (ap > 90) {
					MDEBUG(0,": Delayed Audio Decoder start\n");
					AudioSetPlayMode(card, MAUDIO_PLAY);
					if (!AudioStart(card)) {
						card->startingA = 0;
					}
				}
			}
			if (card->fields >= 250) {  // 5 seconds (PAL)
				SCR_base = DecoderReadSCR(card, 0x009);
				SCR_compareA = DecoderReadSCR(card, 0x014);
				SCR_compareV = DecoderReadSCR(card, 0x00D);
				if (DecoderReadByte(card, 0x013) & 0x03)
				card->fields = 0;
			}
		}

		if (intr[0] & 0x04) {  // First Slice Start Code
			if (card->showvideo) {
				// Unmute card video if first picture slice detected
				VideoSetBackground(card, 0, 0, 0, 0);      // Video on black
				card->showvideo = 0;
			}
		}
		
		if (intr[0] & 0x02 ) {  // Aux/User Data Fifo
			used = 0;
			while ( (used++ < 1000) && 
				(layer = DecoderReadByte(card, 0x040)) & 0x03){
				card->AuxFifo[card->AuxFifoHead] = 
					((layer << 6) & 0x0700) | 
					DecoderReadByte(card, 0x043);
				card->AuxFifoHead = (card->AuxFifoHead + 1) & 
					FIFO_MASK;
			}
			if (used < 1000) DecoderReadAuxFifo(card);
			used = 0;

			while ( (used++ < 1000) && 
				(layer = DecoderReadByte(card, 0x041)) & 0x03){
				card->DataFifo[card->DataFifoHead] = 
					((layer << 6) & 0x0300) | 
					DecoderReadByte(card, 0x043);
				card->DataFifoHead = (card->DataFifoHead + 1) 
					& FIFO_MASK;
			}
			if (used < 1000 ) DecoderReadDataFifo(card);
		}

		if ((intr[0] & 0x01) != card->intdecodestatus) {  
			// decode status
			card->intdecodestatus = intr[0] & 0x01;
			MDEBUG(0, ": Int - decode status now %s\n", 
			       ((card->intdecodestatus) ? 
				"running" : "stopped"));
			if (card->intdecodestatus) {  // now running
				//DecoderSetVideoPanic(card, 1, 3);  
				// video panic at 3 pictures
				card->showvideo = 1;
			} else {  // now stopped
				if (card->closing) {
					card->closing = 0;
					CloseCard(card);
				} 
			}
		 
		}

		if (intr[1] & 0x10) {  // Begin Active Video
			if (card->highlight_valid) {
				for (i = 0; i < 10; i++)
					DecoderWriteByte(card, 0x1C0 + i, 
							 card->highlight[i]);
				card->highlight_valid = 0;
			}
		}
		if (intr[1] & 0x08) {  // SPU Start Code Detected
			MDEBUG(0, ": Int - SPU Start Code Detected\n");
		}
		
		if (intr[1] & 0x04) {  // SCR compare audio
			MDEBUG(0, ": Int - SCR compare audio\n");
			DecoderDelByte(card, 0x013, 0x01);
			AudioStart(card);
		}

		if (intr[2] & 0x20) {  // DSI PES data ready
			DecoderReadNavipack(card);
		}

		if (intr[2] & 0x06) {  // Audio / Video PES data ready
			ReadPESHeaders(card);
		}

		if (intr[3] & 0x40) {  // CSS
			card->css.status = DecoderReadByte(card, 0x0B0);
			if (card->css.status&0x01) 
				card->css.ChallengeReady = 1; 
			// challenge ready
			if (card->css.status&0x02) 
				card->css.ResponseReady = 1;   
			// response ready
			if (card->css.status&0x04) 
				card->css.DiskKey = 1;        
			// Disk key ready
			if (card->css.status&0x08) 
				card->css.Error = 1;          
			// Disk key error
			if (card->css.status&0x10) 
				card->css.TitleKey = 1;        
			// Title key ready
			if (card->css.status&0x20) 
				card->css.TitleKeyDiff = 1;    
			// Title key error
		}


		if (intr[3] & 0x30) { 
			// Audio/Video ES channel buffer underflow
			MDEBUG(1,": Int - ES channel buffer underflow\n");
			if (card->closing) {
				card->closing = 0;
				CloseCard(card);
			} 
		}

		if (intr[4] & 0x10 ) {  // SPU decode error
			MDEBUG(1,": Int - SPU decode error: (1CA)=0x%02X\n", 
			       DecoderReadByte(card, 0x1CA));
			DecoderDelByte(card, 0x1A0, 0x01);  // SPU decode stop
			DecoderSetByte(card, 0x1A0, 0x01);  // SPU decode start
		}
		
		// Audio / Video Syncronisation

		if (card->videosync && !card->videoskip && !card->videodelay) {
			SCR_base = DecoderReadSCR(card, 0x009);
			SCR_compareV = DecoderReadSCR(card, 0x00D);
			if (intr[1] & 0x02) {  // picture start code detected
				DecoderMaskByte(card, 0x011, 0x03, 0x01);   
				// Set SCR compare/capture mode to capture
				DecoderSetByte(card, 0x11, 0x04);           
				// Set "capture on picture start"
				if (intr[1] & 0x01) {  
					// audio sync code detected
					DecoderSetByte(card, 0x11, 0x08); 
					// Set "capture on audio sync code"
				}
				VideoAddr = DecoderReadRWAddr(card,0x080);
				if (PTSGetPTS(&card->VideoPTSStore, VideoAddr,
					      &PTS)) {
					card->oldVPTS = card->VPTS;
					card->VPTS = PTS;
					card->VSCR = ((long)SCR_compareV 
						      - (long)PTS) / 2;
//					card->VideoTime = ISRTime;
				}
			} else if (intr[1] & 0x01) {  
				// audio sync code detected
				DecoderMaskByte(card, 0x011, 0x03, 0x01);   
				// Set SCR compare/capture mode to capture
				DecoderSetByte(card, 0x11, 0x08);           
				// Set "capture on audio sync code"
				AudioAddr = DecoderReadRWAddr(card,0x083);
				if (PTSGetPTS(&card->AudioPTSStore, AudioAddr,
					      &PTS)) {
					card->oldAPTS = card->APTS;
					card->APTS = PTS;
					card->ASCR = ((long)SCR_compareV - 
						      (long)PTS) / 2;
				} else {
					card->ASCR = 0x7FFFFFFF;
				}
				
				if (card->VSCR != 0x7FFFFFFF) {
					if (card->ASCR != 0x7FFFFFFF) {
						DeltaSyncTime = ISRTime - 
							card->SyncTime;
						card->SyncTime = ISRTime;
	
			    // Calculate Audio and Video SCR difference
						Offset = (card->ASCR - 
							  card->VSCR - 
							  (10 * 736)) / 736;
	
	       // if the APTS and SCR are off update SCR to keep SubPic synced
						if ((SCR_compareV > card->APTS)
						    || ((card->APTS - 
							 SCR_compareV) > 
							10000)) {
							Offset = 0;
							SetSCR(card, 
							       card->APTS);
						}
	
						// if more than 3 frames away
						if ((Offset > 3) || 
						    (Offset < -3)) {
							if (Offset > 0 ) {
								card->videodelay = 0;
								if (Offset < 100) {
									if (Offset < 10) {
										card->videodelay = 1;
									} else {
										card->videodelay = Offset / 2;
										if (card->videodelay > 20) {
											card->videodelay = 20;
										}
									}
									MDEBUG(0,": <<< Pausing  %d\n", card->videodelay);
								} else {
								}
							} else {
								card->videoskip = 0;
								if (Offset > -100) {
									if (Offset < -10) {
										card->videoskip = 10;
									} else {
										card->videoskip = 3;
									}
									MDEBUG(0, ": >>> FForward  %d\n", card->videoskip);
								}
							}
						} else {
						}
						card->VSCR = 0x7FFFFFFF;
					}
				}
			}
		}
	}
	DecoderWriteByte(card, 0x006, 0x01);  // Clear Interrupt Pin
}

// Enable the IRQ Masks
void L64021InstallIntr(struct cvdv_cards *card) {
	u8 data;
	
	data=0;
	data |= 0x80;  // new field
	data |= 0x40;  // audio sync recovery
	data |= 0x20;  // SPU SCR compare
	// data |= 0x10;  // SDRAM Transfer Done
	// data |= 0x08;  // Sequence End Code Detect
	data |= 0x04;  // First Slice Start Code
	data |= 0x02;  // Aux/User Data Fifo
	data |= 0x01;  // decode status
	DecoderWriteByte(card, 0x000, (~data) & 0xFF);

	data = 0;
	// data |= 0x80;  // SCR compare
	// data |= 0x40;  // SCR Overflow
	// data |= 0x20;  // Begin Vertical Blank
	data |= 0x10;  // Begin Active Video
	data |= 0x08;  // SPU Start Code Detected
	data |= 0x04;  // SCR compare audio
	data |= 0x02;  // picture start code detected
	data |= 0x01;  // audio sync code detected
	DecoderWriteByte(card, 0x001, (~data) & 0xFF);

	data = 0;
	// data |= 0x80;  // DTS video event
	// data |= 0x40;  // DTS audio event
	data |= 0x20;  // DSI PES data ready
	// data |= 0x10;  // Seq end code in video channel
	data |= 0x08;  // SPU PES data ready
	data |= 0x04;  // Video PES data ready
	data |= 0x02;  // Audio PES data ready
	// data |= 0x01;  // Pack data ready
	DecoderWriteByte(card, 0x002, (~data) & 0xFF);

	data = 0;
	// data |= 0x80;  // Reserved
	data |= 0x40;  // CSS
	data |= 0x20;  // Video ES channel buffer underflow
	data |= 0x10;  // Audio ES channel buffer underflow
	// data |= 0x08;  // Data Dump channel PES data ready
	data |= 0x04;  // SPU channel buffer overflow
	//data |= 0x02;  // Video ES channel buffer overflow
	//data |= 0x01;  // Audio ES channel buffer overflow
	DecoderWriteByte(card, 0x003, (~data) & 0xFF);

	data = 0;
//	data |= 0x80;  // S/PDIF channel buffer underflow
	// data |= 0x40;  // packet error
	// data |= 0x20;  // reserved
	data |= 0x10;  // SPU decode error
//	data |= 0x08;  // Audio Sync error
//	data |= 0x04;  // Audio CRC or illegal bit error
//	data |= 0x02;  // context error
//	data |= 0x01;  // VLC or Run length error
	DecoderWriteByte(card, 0x004, (~data) & 0xFF);
	card->IntInstalled = 1;
}

int L64021RemoveIntr(struct cvdv_cards *card) {
	// Disable the IRQ Masks
	DecoderWriteByte(card, 0x000, 0xFF);   // No ints
	DecoderWriteByte(card, 0x001, 0xFF);   // No ints
	DecoderWriteByte(card, 0x002, 0xFF);   // No ints
	DecoderWriteByte(card, 0x003, 0xFF);   // No ints
	DecoderWriteByte(card, 0x004, 0xFF);   // No ints
	card->IntInstalled = 0;
	return 0;
}

int L64021Reset(struct cvdv_cards *card) {
	L64021RemoveIntr(card);  // Stop interrupts
	// Reset
	MDEBUG(1, ": L64021 Software reset...\n");
	//DecoderSetByte(card, 0x007, 0x20);  // reset on
	DecoderMaskByte(card, 0x007, 0xE2, 0xE2);  // reset on
	while (!(DecoderReadByte(card, 0x007) & 0x02)) ;  // wait until reset is done
	//DecoderDelByte(card, 0x007, 0x20);  // reset off
	DecoderMaskByte(card, 0x007, 0xE2, 0xC2);  // reset off
	MDEBUG(1, ": L64021 Software reset done.\n");
	DecoderStopChannel(card);
	DecoderStopDecode(card);
	DecoderStreamReset(card);
	DecoderSetupReset(card);
	printk(KERN_INFO LOGNAME ": L64021 Rev. 0x%02X reset successfully.\n", 
DecoderReadByte(card, 0x0F5));
	return 0;
}

int L64021Setup(struct cvdv_cards *card) {
	MDEBUG(1, ": -- L64021Setup\n");
	DecoderWriteByte(card, 0x0C1, 0x88);  // 
	switch (card->videomode) {
		case NTSC:  // NTSC M, N. America, Taiwan, Japan
			DecoderMaskByte(card, 0x122, 0x03, 0x01);  // Television Standard: NTSC
			/* Default values:
			DecoderWriteByte(card, 0x116, 90);    // Main Reads per Line
			DecoderWriteByte(card, 0x11A, 4);     // Vline Count Init
			DecoderWriteByte(card, 0x11C, 0x13);  // Pixel State Reset Value / BT.656 Mode / Sync Active Low
			DecoderWriteByte(card, 0x129, 23);    // Start- and End Row
			DecoderWriteByte(card, 0x12A, 262 & 0xFF);
			DecoderWriteByte(card, 0x12B, (262>>4)&0x70);
			DecoderWriteByte(card, 0x12C, 244 & 0xFF);    // Start- and End Column
			DecoderWriteByte(card, 0x12D, 1683 & 0xFF);
			DecoderWriteByte(card, 0x12E, ((1683>>4)&0x70)|((244>>8)&0x07));
			DecoderWriteByte(card, 0x132, 240 & 0xFF);    // SAV Column
			DecoderWriteByte(card, 0x133, 1684 & 0xFF);   // EAV Column
			DecoderWriteByte(card, 0x134, ((1684>>4)&0x70)|((240>>8)&0x07));
			DecoderWriteByte(card, 0x12F, (21&0x1F)|((262>>3)&0x20)|(1<<6)|((265>>1)&0x80));  // VCode Zero...
			DecoderWriteByte(card, 0x130, 262&0xFF);      // ... and VCode Even
			DecoderWriteByte(card, 0x131, 265&0xFF);      // ... and FCode
			*/
			break;
		case PAL:  // PAL-B, D, G, H, I, Europe, Asia
			DecoderMaskByte(card, 0x122, 0x03, 0x02);  // Television Standard: PAL
			/* Default values:
			DecoderWriteByte(card, 0x116, 90);    // Main Reads per Line
			DecoderWriteByte(card, 0x11A, 1);     // Vline Count Init
			DecoderWriteByte(card, 0x11C, 0x13);  // Pixel State Reset Value / BT.656 Mode / Sync Active Low
			DecoderWriteByte(card, 0x129, 23);    // Start- and End Row
			DecoderWriteByte(card, 0x12A, 310 & 0xFF);
			DecoderWriteByte(card, 0x12B, (310>>4)&0x70);
			DecoderWriteByte(card, 0x12C, 264 & 0xFF);    // Start- and End Column
			DecoderWriteByte(card, 0x12D, 1703 & 0xFF);
			DecoderWriteByte(card, 0x12E, ((1703>>4)&0x70)|((264>>8)&0x07));
			DecoderWriteByte(card, 0x132, 260 & 0xFF);    // SAV Column
			DecoderWriteByte(card, 0x133, 1704 & 0xFF);   // EAV Column
			DecoderWriteByte(card, 0x134, ((1704>>4)&0x70)|((260>>8)&0x07));
			DecoderWriteByte(card, 0x12F, (21&0x1F)|((310>>3)&0x20)|(0<<6)|((312>>1)&0x80));  // VCode Zero...
			DecoderWriteByte(card, 0x130, 310&0xFF);      // ... and VCode Even
			DecoderWriteByte(card, 0x131, 312&0xFF);      // ... and FCode
			*/
			break;
		case PAL60:  // PAL 60Hz
		case NTSC60:  // NTSC 60Hz, USA HDTV
		case PALM:  // PAL-M normal, Brazil
		case PALM60:  // PAL-M HDTV, Brazil
		case PALN:  // PAL-N, Uruguay, Paraguay
		case PALNc:  // PAL-Nc, Argentinia
		default:  // TODO: set mode according to other standards
			DecoderMaskByte(card, 0x122, 0x03, 0x00);  // Television Standard: User programmed
			DecoderWriteByte(card, 0x116, 90);    // Main Reads per Line
			DecoderWriteByte(card, 0x11A, 1);     // Vline Count Init
			DecoderWriteByte(card, 0x11C, 0x13);  // Pixel State Reset Value / BT.656 Mode / Sync Active Low
			DecoderWriteByte(card, 0x129, 23);    // Start- and End Row
			DecoderWriteByte(card, 0x12A, 310 & 0xFF);
			DecoderWriteByte(card, 0x12B, (310>>4)&0x70);
			DecoderWriteByte(card, 0x12C, 264 & 0xFF);    // Start- and End Column
			DecoderWriteByte(card, 0x12D, 1703 & 0xFF);
			DecoderWriteByte(card, 0x12E, ((1703>>4)&0x70)|((264>>8)&0x07));
			DecoderWriteByte(card, 0x132, 260 & 0xFF);    // SAV Column
			DecoderWriteByte(card, 0x133, 1704 & 0xFF);   // EAV Column
			DecoderWriteByte(card, 0x134, ((1704>>4)&0x70)|((260>>8)&0x07));
			DecoderWriteByte(card, 0x12F, (21&0x1F)|((310>>3)&0x20)|(0<<6)|((312>>1)&0x80));  // VCode Zero...
			DecoderWriteByte(card, 0x130, 310&0xFF);      // ... and VCode Even
			DecoderWriteByte(card, 0x131, 312&0xFF);      // ... and FCode
			break;
	}
	DecoderWriteByte(card, 0x045, 0x00);  // disable compares and panic mode
	DecoderWriteByte(card, 0x094, 0x00);    // disable TOS Detect
	DecoderMaskByte(card, 0x109, 0x30, 0x00);  // Display Override off, don't change OSD, Background
	DecoderWriteByte(card, 0x112, 0x00);  // Disable Horizontal 2:1 Filter
	DecoderWriteByte(card, 0x113, 0x14);  // FreezeMode 1 / 3:2 Pulldown / Repeat First Field / Top Field First
	DecoderWriteByte(card, 0x114, ( 5 <<3)|( 0 <<1)|( 0 <<2)|( 1 <<7));  // VideoMode/FilterEnable/FilterAB/FieldSyncEnable
	DecoderWriteByte(card, 0x115, 0);     // Horizontal Filter Scale
	DecoderWriteByte(card, 0x117, 0x80);  // Automatic Field Inversion Correction
//  DecoderWriteByte(card, 0x117, 0x00);  // no Automatic Field Inversion Correction
	DecoderWriteByte(card, 0x118, 0);     // Horizontal Pan and Scan Word Offset (signed)
	DecoderWriteByte(card, 0x119, 0);     // Vertical Pan and Scan Line Offset
	DecoderWriteByte(card, 0x11B, 0x00);  // Override Picture Width
//    if (0) {  // letterbox
//      DecoderWriteByte(card, 0x114, (DecoderReadByte(card, 0x114) & ~0x78) | 0x40);  // mode 8
//      DecoderWriteByte(card, 0x129, 0x35);
//      DecoderWriteByte(card, 0x12A, 0xE7);
//      DecoderWriteByte(card, 0x114, DecoderReadByte(card, 0x114) & ~0x77);  // ???
//    } else {
//      if (0) {  // MPEG-1
//        DecoderWriteByte(card, 0x114, (DecoderReadByte(card, 0x114) & ~0x78) | 0x10);  // mode 2
//      } else {  // MPEG-2
//        DecoderWriteByte(card, 0x114, (DecoderReadByte(card, 0x114) & ~0x78) | 0x28);  // mode 5
//      }
//    }
	L64021InstallIntr(card);  // Set the interrupt masks, again
	
	return 0;
}

int L64021Init(struct cvdv_cards *card) {
MDEBUG(1, ": -- L64021Init\n");
	L64021Reset(card);
	L64021Setup(card);
	VideoSetBackground(card, 1, 0, 0, 0);  // black
	DecoderWriteByte(card, 0x135, 0x01);  // Enable Video Out, Disable SPU Mix
	DecoderWriteByte(card,0x11C,0x13);  // Pixel State Reset Value / BT.656 Mode / Sync Active Low
	L64021InstallIntr(card);
	return 0;
}


