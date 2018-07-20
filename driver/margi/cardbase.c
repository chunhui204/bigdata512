/* 
    cardbase.c

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

#include "cardbase.h"

// List of pci cards in the system
struct cvdv_cards *first_card = NULL;

u8 FlushPacket[32] = {
	0x00, 0x00, 0x01, 0xE0,	// video stream start code
	0x00, 0x1a,		// 26 more bytes
	0x81, 0xC1,		// flags: copy=1, PTS_DTS=11, PES_extension=1
	0x0D,			// 13 more header bytes
	0x31, 0x00, 0x03, 0x5F, 0xEB,	// PTS
	0x11, 0x00, 0x03, 0x48, 0x75,	// DTS
	0x1E,			// flags: P-STD_buffer=1, 
	0x60, 0xE8,		// P-STD_buffer_scale=1, P-STD_buffer_size=232(kByte)
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void DecoderStreamReset(struct cvdv_cards *card)
{
	card->stream.valid = 0;
	card->stream.sh.valid = 0;
	card->stream.se.valid = 0;
	card->stream.gop.valid = 0;
	card->stream.MPEG2 = 0;
	card->stream.audio.valid = 0;
	memset(&card->stream.audio.mpeg,0,sizeof(struct AudioMPEG));
	memset(&card->stream.audio.ac3,0,sizeof(struct AudioAC3));
	memset(&card->stream.audio.pcm,0,sizeof(struct AudioPCM));
	card->AuxFifoExt = 0;
	card->AuxFifoLayer = -1;
}
void PTSStoreInit(PTSStorage * store, int size)
{
	int i;
	if (size > MAX_PTS)
		size = MAX_PTS;
	store->size = size;
	store->begin = 0;
	store->end = 0;
	store->LastAddr = 0;
	for (i = 0; i < store->size; i++) {
		store->AddrB[i] = 0;
		store->AddrE[i] = 0;
		store->PTS[i] = 0;
	}
}

void DecoderCSSReset(struct cvdv_cards *card)
{
	card->css.status = 0;
	card->css.ChallengeReady = 0;
	card->css.ResponseReady = 0;
	card->css.DiskKey = 0;
	card->css.TitleKey = 0;
	card->css.Error = 0;
	card->css.TitleKeyDiff = 0;
	card->LastAddr = 0;	// last used address in PES buffer
	card->VPTS = 0;
	card->oldVPTS = 0;
	card->VSCR = 0;
	card->APTS = 0;
	card->oldAPTS = 0;
	card->ASCR = 0;
	card->SyncTime = 0;
	card->paused = 0;	// pause status
	card->lastvattr = 0;	// last set dvd video attribute
	card->lastaattr = 0;	// last set dvd audio attribute
	card->nonblock = 0;
}

void DecoderSetupReset(struct cvdv_cards *card)
{
	card->DecoderOpen = 0;
	card->closing = 0;
	card->channelrun = 0;
	card->setup.streamtype = stream_none;
	card->setup.audioselect = audio_none;
	card->setup.videoID = 0;
	card->setup.audioID = 0;
	card->setup.audioIDext = -1;
	card->setup.SPDIFmode = 0;
	card->startingV = 0;
	card->startingA = 0;
	card->startingDVDV = 0;
	card->startingDVDA = 0;
	card->videodelay = 0;
	card->videodelay_last = 0;
	card->videoslow_last = 0;
	card->videoslow = 0;
	card->videoffwd = 0;
	card->videoffwd_last = 0;
	card->videoskip = 0;
	card->videoskip_last = 0;
	card->videosync = 0; 
	card->paused = 0;
	PTSStoreInit(&card->VideoPTSStore, MAX_PTS);
	PTSStoreInit(&card->AudioPTSStore, MAX_PTS);
        card->audiostate.AV_sync_state=1;
}



void card_init(struct cvdv_cards *card)
{
	card->DRAMFirstBlock = NULL;
	card->DRAMSize = 0;
	card->OSD.open = 0;
	card->DMAABusy = 0;
	card->DMABBusy = 0;
	card->IntInstalled = 0;
	card->ChannelBuffersAllocated = 0;
	card->VideoES = BLANK;
	card->AudioES = BLANK;
	card->VideoPES = BLANK;
	card->DataDump = BLANK;
	card->AudioPES = BLANK;
	card->NaviBank = BLANK;
	card->FrameBuffersAllocated = 0;
	card->FrameStoreLuma1 = BLANK;
	card->FrameStoreChroma1 = BLANK;
	card->FrameStoreLuma2 = BLANK;
	card->FrameStoreChroma2 = BLANK;
	card->FrameStoreLumaB = BLANK;
	card->FrameStoreChromaB = BLANK;
	card->DecoderOpen = 0;
	card->AuxFifoHead = 0;
	card->AuxFifoTail = 0;
	card->DataFifoHead = 0;
	card->DataFifoTail = 0;
	card->FifoALast = -1;
	card->FifoBLast = -1;
	//reset_stream(card);
	DecoderStreamReset(card);
	DecoderSetupReset(card);
	card->AudioInitialized = 0;
	card->AudioOldMode = -1;
	card->closing = 0;
	card->startingV = 0;
	card->startingA = 0;
	card->startingDVDV = 0;
	card->startingDVDA = 0;
	card->channelrun = 0;
	card->fields = 0;
	DecoderCSSReset(card);
	card->NaviPackAddress = 0;
	init_waitqueue_head(&card->wqA);
	init_waitqueue_head(&card->wqB);
	card->navihead = 0;	// write pointer for navi ring buffer
	card->navitail = 0;	// read pointer for navi ring buffer
	card->intdecodestatus = 0;	// last status of decode interrupt
	card->showvideo = 0;	// show video instead black as soon as picture slice is there
	card->videodelay = 0;	// slow counter
	card->videodelay_last = 0;	// slow counter
	card->videoffwd = 0;	// fast playback
	card->videoffwd_last = 0;	// fast playback
	card->videoskip = 0;	// fast counter
	card->videoskip_last = 0;	// fast counter
	card->videoslow_last = 0;
	card->videoslow = 0;
	card->videosync = 0;	// do audio/video sync basen on PTS?
	PTSStoreInit(&card->VideoPTSStore, MAX_PTS);
	PTSStoreInit(&card->AudioPTSStore, MAX_PTS);
	card->LastAddr = 0;	// last used address in PES buffer
	card->VPTS = 0;
	card->oldVPTS = 0;
	card->VSCR = 0;
	card->APTS = 0;
	card->oldAPTS = 0;
	card->ASCR = 0;
	card->SyncTime = 0;
	card->paused = 0;	// pause status
	card->lastvattr = 0;	// last set dvd video attribute
	card->lastaattr = 0;	// last set dvd audio attribute
	card->reg08F = 0;	// mirror of decoder registers
	card->reg090 = 0;
	card->reg091 = 0;
	card->reg092 = 0;
	card->reg093 = 0;
	card->highlight_valid = 0;	// SPU highlight info available for next BAV int
	card->do_flush = 0;

	card->open = 0;

	card->VideoESSize = 0;		
	card->AudioESSize = 0;		
	card->VideoPESSize = 0;		
	card->DataDumpSize = 0;		
	card->AudioPESSize = 0;		
	card->NaviBankSize = 0;		
	card->currentType = -1;
	card->rbufA.buffy = NULL;
	card->rbufB.buffy = NULL;
	card->use_ringA = 0;
	card->use_ringB = 0;
	card->hasZV = 0;
#ifdef NOINT
	init_timer(&card->timer);
	spin_lock_init(&card->timelock);
#endif
	card->dvb_registered = 0;
        card->audiostate.AV_sync_state=1;
	card->nonblock = 0;
	card->svhs = 0;
	card->composite = 0;
}

