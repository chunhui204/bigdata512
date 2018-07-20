/* 
    streams.c

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

#include "streams.h"
#include "dram.h"
#include "l64021.h"
#include "video.h"
#include "audio.h"

// Frees allocated channel buffers
int DecoderKillChannelBuffers(struct cvdv_cards *card)
{
	MDEBUG(1, ": -- DecoderKillChannelBuffers\n");
	DecoderStopDecode(card);
	DRAMFree(card, card->VideoES);
	card->VideoES = BLANK;
	DRAMFree(card, card->AudioES);
	card->AudioES = BLANK;
	DRAMFree(card, card->VideoPES);
	card->VideoPES = BLANK;
	DRAMFree(card, card->DataDump);
	card->DataDump = BLANK;
	DRAMFree(card, card->AudioPES);
	card->AudioPES = BLANK;
	DRAMFree(card, card->NaviBank);
	card->NaviBank = BLANK;
	card->ChannelBuffersAllocated = 0;
//  DecoderWriteWord(
	return 0;
}

// Allocates channel buffers
// All sizes in bytes, preferably multiple of 256 (will be rounded up otherwise)
int DecoderSetChannelBuffers(struct cvdv_cards *card, int VideoES,	// Video ES Channel Buffer size, e.g. 229376 byte for NTSC
			     int AudioES,	// Audio ES Channel Buffer size, 4096 byte
			     int VideoPES,	// Video PES Header / SPU Channel Buffer size, 512 byte
			     int DataDump,	// Data Dump Channel Buffer size, e.g. 80896 byte
			     int AudioPES,	// Audio PES Header / System Channel Buffer size, 512 byte
			     int NaviBank)
{				// Navi Bank Channel Buffer size, 2048 byte
#define BUFFERSET(buf,adr,align)  if (buf>0) {\
  if (buf&((1<<align)-1)) buf=(buf&~((1<<align)-1))+(1<<align);\
  addr=DRAMAlloc(card,buf,1<<align);\
  if (addr==BLANK) return adr;\
  card->buf=addr;\
  addr>>=align;\
  DecoderWriteByte(card,adr,addr&0xFF);\
  DecoderWriteByte(card,adr+1,(addr>>8)&(0x003F));\
  addr+=(buf>>align);\
  DecoderWriteByte(card,adr+2,(addr-1)&0xFF);\
  DecoderWriteByte(card,adr+3,((addr-1)>>8)&0x003F);\
}
	u32 addr;
	MDEBUG(1, ": -- DecoderSetChannelBuffers\n");
	//DecoderStopDecode(card);
	DecoderStopChannel(card);
	VideoES >>= 1;		// change to word sizes
	AudioES >>= 1;
	VideoPES >>= 1;
	DataDump >>= 1;
	AudioPES >>= 1;
	NaviBank >>= 1;
	if (card->ChannelBuffersAllocated)
		DecoderKillChannelBuffers(card);
	BUFFERSET(VideoES, 0x048, 7);
	BUFFERSET(AudioES, 0x04C, 7);
	BUFFERSET(VideoPES, 0x050, 7);
	BUFFERSET(DataDump, 0x054, 7);
	BUFFERSET(AudioPES, 0x058, 7);
	BUFFERSET(NaviBank, 0x05C, 7);

	card->VideoESSize = VideoES;		
	card->AudioESSize = AudioES;		
	card->VideoPESSize = VideoPES;		
	card->DataDumpSize = DataDump;		
	card->AudioPESSize = AudioPES;		
	card->NaviBankSize = NaviBank;		

	DecoderWriteByte(card, 0x044, 0x7F);
	DecoderWriteByte(card, 0x044, 0x01);
	if (NaviBank) {
		card->reg07B |= 0x10;	// navi pack counter enable
		DecoderWriteByte(card, 0x07B, card->reg07B);
		//DecoderSetByte(card,0x07B,0x10);  // navi pack counter enable
		card->NaviPackAddress =
		    (DecoderReadWord(card, 0x05C) & 0x3FFF) << 7;
		MDEBUG(4, ": navi bank init'ed: 0x%08X\n",card->NaviPackAddress);
	} else {
		card->reg07B &= ~0x10;	// navi pack counter disable
		DecoderWriteByte(card, 0x07B, card->reg07B);
		//DecoderDelByte(card,0x07B,0x10);  // navi pack counter disable
		card->NaviPackAddress = 0;
	}
	card->ChannelBuffersAllocated = 1;
#undef BUFFERSET
	return 0;
}

//int DecoderReadFifo

int DecoderUnPrepare(struct cvdv_cards *card)
{
	MDEBUG(0, ": -- DecoderUnPrepare\n");
	//DecoderStopDecode(card);
	DecoderStopChannel(card);
	DecoderKillChannelBuffers(card);
	return 0;
}

void DecoderPrepare(struct cvdv_cards *card)
{
	//VideoSetBackground(card,0,0,0,0);      // Video on black
	VideoSetBackground(card, 1, 0, 0, 0);	// black
	//VideoSetBackground(card,2,83,90,249);  // Red
	//VideoSetBackground(card,2,155,53,53);  // Green
	//VideoSetBackground(card,2,35,212,114); // Blue
	//VideoSetBackground(card,2,4,128,128);  // Black
	//VideoSetBackground(card,3,155,53,53);  // Video on Green

	//DecoderWriteByte(card,0x044,0x00);  // Reset channel buffers on error
//  DecoderWriteByte(card,0x044,0x01);  // don't Reset channel buffers on error

	DecoderWriteByte(card, 0x040, 0x01);	// Reset Aux FIFO
	DecoderWriteByte(card, 0x041, 0x01);	// Reset Data FIFO
	//DecoderWriteByte(card,0x044,0x7E);    // Reset channel buffers, Reset channel buffers on error
	DecoderWriteByte(card, 0x044, 0x7F);	// Reset channel buffers, don't Reset channel buffers on error
//  udelay(100);
//  DecoderWriteByte(card,0x040,0x00);  // Reset Aux FIFO
//  DecoderWriteByte(card,0x041,0x00);  // Reset Data FIFO
//  DecoderDelByte(card,0x044,0x7E);    // Reset channel buffers
}

// Selects audio type MPEG and sets stream ID's
// AID:     -1=all MPEG, Audio Stream ID: 0..31
// AExt:    -1=unused, Audio Stream Extension ID: 0..31, only used if AType=5
void DecoderSelectAudioID(struct cvdv_cards *card)
{
	int AID = card->setup.audioID;
	int AExt = card->setup.audioIDext;
	MDEBUG(1, ": -- SelectAudio %d %d\n", AID, AExt);
	DecoderWriteByte(card, 0x07C, AExt & 0x1F);	// Audio Stream Extension ID
	card->reg08F = (card->reg08F & ~0x1F) | (AID & 0x1F);
	DecoderWriteByte(card, 0x08F, card->reg08F);
	//DecoderMaskByte(card,0x08F,0x1F,AID&0x1F);   // Set Stream ID
}

// AHeader: 0=No Headers, 1=first PTS/DTS header, 2=all headers, 3=All with PTS/DTS
// AType:   0=disable audio, 1=MPEG ID (MPEG 1), 2=Lin.PCM ID, 3=AC3 ID, 4=all MPEG (use only, if just one MPEG audio stream), 5=MPEG multichannel ID (MPEG 2)
// AID:     -1=all MPEG, Audio Stream ID: 0..31
// AExt:    -1=unused, Audio Stream Extension ID: 0..31, only used if AType=5
// IEC956:  0:MPEG/AC3 data on digital out 1:IEC956 data on digital S/PDIF out
void DecoderPrepareAudio(struct cvdv_cards *card)
{
	int AHeader = 2;
	int AType = 3;
	int AID = card->setup.audioID;
	int AExt = card->setup.audioIDext;
	int IEC956 = card->setup.SPDIFmode;
	MDEBUG(1, ": -- PrepAudio %d %d %d %d %d\n",
	       AHeader, card->setup.audioselect, AID, AExt, IEC956);
	switch (card->setup.audioselect) {
	case audio_disable:
	case audio_none:
	case audio_SDDS:
		AType = 0;
		break;
	case audio_MPEG:	// MPEG Audio
		AType = 1;
		break;
	case audio_MPEG_EXT:	// MPEG Audio with extension stream
		AType = 5;
		break;
	case audio_LPCM:	// Linear Pulse Code fe_modulation_t LPCM
		AType = 2;
		break;
	case audio_AC3:	// AC-3
		AType = 3;
		break;
	case audio_DTS:	// DTS
		AType = 8;
		break;
	}
	if (AType <= 0) {
		card->reg08F = 0x00;	// disable audio and discard all packets
		DecoderWriteByte(card, 0x08F, card->reg08F);
		//DecoderWriteByte(card,0x08F,0x00);  // disable audio and discard all packets
		//DecoderMaskByte(card,0x093,0xC3,0xC0);  // write no headers
		card->reg093 = (card->reg093 & ~0x03);	// write no headers
		DecoderWriteByte(card, 0x093, card->reg093);
	} else {
		AudioOpen(card);
		DecoderMaskByte(card, 0x165, 0x1F, 0x00);	// reset the register
		if (AType == 8) {	// DTS
			card->reg090 |= 0x01;	// DTS in Transport Private 1 Stream stored in AudioES channel buffer
			DecoderWriteByte(card, 0x090, card->reg090);
			//DecoderSetByte(card,0x090,0x01);  // DTS in Transport Private 1 Stream stored in AudioES channel buffer
			AudioSetMode(card, 0);
			DecoderSetByte(card, 0x165, 0x01);
			AudioStartFormat(card);
		} else if (AType == 3) {	// AC3
			card->reg090 |= 0x01;	// AC3 in Transport Private 1 Stream stored in AudioES channel buffer
			DecoderWriteByte(card, 0x090, card->reg090);
			//DecoderSetByte(card,0x090,0x01);  // AC3 in Transport Private 1 Stream stored in AudioES channel buffer
			AudioSetMode(card, ((IEC956) ? 1 : 3));
		} else if (AType == 2) {	// PCM
			card->reg090 |= 0x01;	// PCM in Transport Private 1 Stream stored in AudioES channel buffer
			DecoderWriteByte(card, 0x090, card->reg090);
			//DecoderSetByte(card,0x090,0x01);  // PCM in Transport Private 1 Stream stored in AudioES channel buffer
			AudioSetMode(card, 4);
		} else {	// MPEG
			card->reg090 &= ~0x01;	// MPEG Audio stored in AudioES channel buffer
			DecoderWriteByte(card, 0x090, card->reg090);
			//DecoderDelByte(card,0x090,0x01);  // MPEG Audio stored in AudioES channel buffer
			if (AID < 0)
				AType = 4;
			if (AExt >= 0)
				AType = 5;
			else
				AExt = -1;
			AudioSetMode(card, ((IEC956) ? 0 : 2));
		}
		card->setup.audioID = AID;
		card->setup.audioIDext = AExt;
		DecoderSelectAudioID(card);
		card->reg08F = (card->reg08F & ~0xE0) | ((AType & 0x07) << 5);	// Set Stream Type
		DecoderWriteByte(card, 0x08F, card->reg08F);
		//DecoderMaskByte(card,0x08F,0xE0,(AType&0x07)<<5);   // Set Stream Type
		AudioSetVolume(card, 0xFF);	// Set PCM scale to full volume
		//DecoderMaskByte(card,0x093,0xC3,(AHeader&0x03)|0xC0);  // write header select
		card->reg093 = (card->reg093 & ~0x03) | (AHeader & 0x03);	// write header select
		DecoderWriteByte(card, 0x093, card->reg093);
		//  Mute the card and put it in play mode, then wait for the parameters to be parsed and un-mute if successful
		//AudioMute(card,1);
		if (AType > 0) {
			AudioStartDecode(card);
			//AudioSetPlayMode(card,MAUDIO_PLAY);
			AudioSetPlayMode(card, MAUDIO_PAUSE);
		}
		//card->startingA=1;
	}
	card->lastaattr = 0;
}

// VHeader: -1=disable Video, 0=No Headers, 1=first PTS/DTS header, 2=all headers, 3=All with PTS/DTS
// VID: -1=all MPEG, 0..15=Video Stream ID
void DecoderPrepareVideo(struct cvdv_cards *card)
{
	int VHeader = 3;
	int VID = card->setup.videoID;
	if (VHeader < 0) {
		card->reg091 = 0x00;
		DecoderWriteByte(card, 0x091, card->reg091);
		//DecoderWriteByte(card,0x091,0x00);
	} else {
		if (VID < 0) {
			card->reg091 = ((VHeader & 0x03) << 6) | (2 << 4);
			DecoderWriteByte(card, 0x091, card->reg091);
			//DecoderWriteByte(card,0x091,((VHeader&0x03)<<6)|(2<<4));
		} else {
			card->reg091 =
			    ((VHeader & 0x03) << 6) | (1 << 4) | (VID &
								  0x0F);
			DecoderWriteByte(card, 0x091, card->reg091);
			//DecoderWriteByte(card,0x091,((VHeader&0x03)<<6)|(1<<4)|(VID&0x0F));
		}
	}
}

// Prepare Decoder for Elementary Streams, Disable Preparser
int DecoderPrepareES(struct cvdv_cards *card)
{
	int i;
	MDEBUG(1, ": -- PrepareES\n");
	//DecoderStopDecode(card);

//  DecoderWriteByte(card,0x05,0x00);

	DecoderMaskByte(card, 0x007, 0xCE, 0xC2 | (3 << 2));	// Stream Select: A/V Elementary Stream
	MDEBUG(3, ": Int - A VideoES w/r addr: %08X %08X\n",
	       (DecoderReadByte(card,0x060)|(DecoderReadByte(card,0x061)<<8)|
		(DecoderReadByte(card,0x062)<<16))<<2,
	       (DecoderReadByte(card,0x06C)|(DecoderReadByte(card,0x06D)<<8)|
		(DecoderReadByte(card,0x06E)<<16))<<2);
	// set the decoding buffers
	card->reg093 = (card->reg093 & ~0xFC);	// write no header
	DecoderWriteByte(card, 0x093, card->reg093);
	if ((i = DecoderSetChannelBuffers(card, 256000, 4096, 0, 0, 0, 0))) {
		MDEBUG(0, ": SetDecoderBuffers failed for buffer at 0x%03X\n", i);
		DecoderKillChannelBuffers(card);
		return 1;
	}
	MDEBUG(3, ": Int - B VideoES w/r addr: %08X %08X\n",
	       (DecoderReadByte(card,0x060)|(DecoderReadByte(card,0x061)<<8)|
		(DecoderReadByte(card,0x062)<<16))<<2,
	       (DecoderReadByte(card,0x06C)|(DecoderReadByte(card,0x06D)<<8)|
		(DecoderReadByte(card,0x06E)<<16))<<2);

	MDEBUG(3, ": Int - C VideoES w/r addr: %08X %08X\n",
	       (DecoderReadByte(card,0x060)|(DecoderReadByte(card,0x061)<<8)|
		(DecoderReadByte(card,0x062)<<16))<<2,
	       (DecoderReadByte(card,0x06C)|(DecoderReadByte(card,0x06D)<<8)|
		(DecoderReadByte(card,0x06E)<<16))<<2);

//  DecoderStartChannel(card);   
//  DecoderStartDecode(card);

	MDEBUG(3, ": Int - D VideoES w/r addr: %08X %08X\n",
	       (DecoderReadByte(card,0x060)|(DecoderReadByte(card,0x061)<<8)|
		(DecoderReadByte(card,0x062)<<16))<<2,
	       (DecoderReadByte(card,0x06C)|(DecoderReadByte(card,0x06D)<<8)|
		(DecoderReadByte(card,0x06E)<<16))<<2);

	DecoderPrepare(card);

	return 0;
}

// Prepare Decoder for Packetised Elementary Streams, set parameters of Preparser
int DecoderPreparePES(struct cvdv_cards *card)
{

	// SPUID: -1=No SPU, 0..31=Display SPU of this ID
	// DataDump: 0=disable DataDump, 1=process DataDump Substreams
	// PackHeader: 0=write no headers, 1=write one header, 2=write all headers
	// SysHeader: 0=write no headers, 1=write one header, 2=write all headers
	// DSIHeader: 0=write no headers, 3=write PCI and DSI headers and packets
	int i;
	int SPUID = -1;
	int DataDump = 0;
	int PackHeader = 0;
	int SysHeader = 0;
	int DSIHeader = 0;

	MDEBUG(1, ": -- PreparePES\n");
	DecoderMaskByte(card, 0x007, 0xCE, 0xC2 | (0 << 2));	// Stream Select: A/V PES Packets

	if (SPUID < 0)
		card->reg092 = 0;	// Do we use SPU?
	else
		card->reg092 = 0x20 | (SPUID & 0x1F);
	if (DataDump)
		card->reg092 |= 0x40;	// Do we use DataDump?
	DecoderWriteByte(card, 0x092, card->reg092);
	//DecoderMaskByte(card,0x093,0xFC,((DSIHeader&0x03)<<6)|((PackHeader&0x03)<<4)|((SysHeader&0x03)<<2));
	card->reg093 =
	    (card->reg093 & ~0xFC) | (((DSIHeader & 0x03) << 6) |
				      ((PackHeader & 0x03) << 4) |
				      ((SysHeader & 0x03) << 2));
	DecoderWriteByte(card, 0x093, card->reg093);
	// set the decoding buffers
	if (
	    (i =
	     DecoderSetChannelBuffers(card, 256000, 4096, 512, 0, 512,
				      0))) {
		MDEBUG(0,": SetDecoderBuffers failed for buffer at 0x%03X\n", i);
		DecoderKillChannelBuffers(card);
		return 1;
	}

	DecoderPrepare(card);

	return 0;
}


// Prepare Decoder for MPEG 1 Systems Streams or MPEG 2 Program Streams
// SPUID: -1:ignore, 0...15 SPU Substream ID
// DataDump: 0:disable data dump stream, 1:enable data dump stream
// PackHeader: 0:write no headers, 1:write one header, 2:write all headers, 3:always discard
// SysHeader: 0:always discard, 1:write one header, 2:write all headers, 3:always discard
// DSIHeader: 0:write no DSI or PCI headers, 3:write DSI and PCI headers + packets
// DVD: 0: normal MPEG-2 data, 1: DVD stream with navi pack data
int DecoderPreparePS(struct cvdv_cards *card,
		     int SPUID, int DataDump,
		     int PackHeader, int SysHeader, int DSIHeader, int DVD)
{
	int i=0;
	MDEBUG(1, ": -- PreparePS %s\n", ((DVD) ? "DVD" : ""));
	//DecoderStopDecode(card);
	DecoderMaskByte(card, 0x007, 0xCE, 0xC2 | (1 << 2));	// Stream Select: MPEG1 System / MPEG2 Program Stream

	if (SPUID < 0)
		card->reg092 = 0;	// Do we use SPU?
	else
		card->reg092 = 0x20 | (SPUID & 0x1F);
	if (DataDump)
		card->reg092 |= 0x40;	// Do we use DataDump?
	DecoderWriteByte(card, 0x092, card->reg092);
	//DecoderMaskByte(card,0x093,0xFC,((DSIHeader&0x03)<<6)|((PackHeader&0x03)<<4)|((SysHeader&0x03)<<2));
	card->reg093 =
	    (card->reg093 & ~0xFC) | (((DSIHeader & 0x03) << 6) |
				      ((PackHeader & 0x03) << 4) |
				      ((SysHeader & 0x03) << 2));
	DecoderWriteByte(card, 0x093, card->reg093);
	// set the decoding buffers
	if (DVD) {		// do we need SPU-, navi- and datadump-buffers?
		
	  //	  if(card->videomode == NTSC)
		i = DecoderSetChannelBuffers(card, 340000, 32768, 32768, 0, 
					     512,4096) ;
		//else
		//		i = DecoderSetChannelBuffers(card, 291878, 16384, 512, 0, 
		//   512,0) ;

		if (i) {
			MDEBUG(0,": SetDecoderBuffers failed for buffer at 0x%03X\n", i);
			DecoderKillChannelBuffers(card);
			return 1;
		}
		
	} else {		// normal PS
		if (
		    (i =
                  DecoderSetChannelBuffers(card, 340000, 32768, 512,
                                   0, 512, 0))) {
 			MDEBUG(0,": SetDecoderBuffers failed for buffer at 0x%03X\n", i);
			DecoderKillChannelBuffers(card);
			return 1;
		}
	}

	DecoderPrepare(card);

	return 0;
}
