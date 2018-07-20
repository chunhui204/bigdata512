/* 
    audio.c

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

//
//  Audio Decoder
//
#define __NO_VERSION__

#include "audio.h"
#include "l64021.h"

// mode=0 pause
// mode=1 normal speed play
// mode=2 fast play, 16/15
// mode=3 slow play, 16/17

void AudioSetPlayMode(struct cvdv_cards *card, int mode)
{
	DecoderMaskByte(card, 0x163, 0x60, (mode & 0x03) << 5);	
	// audio decoder play mode
	DecoderMaskByte(card, 0x164, 0x60, (((mode) ? 1 : 0) & 0x03) << 5);
	// S/PDIF formatter play mode
}

void AudioStartDecode(struct cvdv_cards *card)
{
	DecoderSetByte(card, 0x163, 0x80);
}

// Stop Decode flushes the Audio ES channel buffer
void AudioStopDecode(struct cvdv_cards *card)
{
	DecoderDelByte(card, 0x163, 0x80);
}

void AudioStartFormat(struct cvdv_cards *card)
{
	DecoderSetByte(card, 0x164, 0x80);
}

void AudioStopFormat(struct cvdv_cards *card)
{
	DecoderDelByte(card, 0x164, 0x80);
}

//         Audio source:    S/PDIF out:
// mode 0: MPEG             IEC958
// mode 1: AC3              IEC958
// mode 2: MPEG             MPEG
// mode 3: AC3              AC3
// mode 4: PCM              IEC958 (max. 48kHz)
// mode 5: PCM 96->48kHz    IEC958 (48kHz)
// mode 6: CD Bypass        S/PDIF Bypass
// mode 7: PCM FIFO         PCM FIFO
void AudioSetMode(struct cvdv_cards *card, int mode)
{
	mode &= 0x07;
	AudioSetPlayMode(card, MAUDIO_PAUSE);
	AudioStopFormat(card);
	DecoderMaskByte(card, 0x165, 0xE0, mode << 5);
	if ((mode == 2) || (mode == 3))
		AudioStartFormat(card);
}


// volume: 0..255
void AudioSetVolume(struct cvdv_cards *card, int volume)
{
	DecoderWriteByte(card, 0x16A, volume);	// Set PCM scale to volume
}

// mute=1: mute audio
void AudioMute(struct cvdv_cards *card, int mute)
{
	DecoderMaskByte(card, 0x166, 0x40, (mute ? 0x40 : 0x00));	
	// mute PCM
	DecoderMaskByte(card, 0x16E, 0x10, (mute ? 0x10 : 0x00));	
	// mute S/PDIF
}

// mode=0: stereo
// mode=1: surround
void AudioAC3Mode(struct cvdv_cards *card, int mode)
{
	DecoderMaskByte(card, 0x166, 0x10, (mode ? 0x10 : 0x00));
}

// mode=0: custom analog
// mode=1: custom digital
// mode=2: line-out (default)
// mode=3: RF mode
void AudioAC3Compression(struct cvdv_cards *card, int mode)
{
	DecoderMaskByte(card, 0x166, 0x03, mode & 0x03);
}

// mode=0: AC3
// mode=1: ES1
void AudioAC3Formatter(struct cvdv_cards *card, int mode)
{
	DecoderMaskByte(card, 0x166, 0x03, mode & 0x03);
}

// mode=0: Stereo
// mode=1: Right channel only
// mode=2: Left channel only
// mode=3: Mono Mix
void AudioDualMono(struct cvdv_cards *card, int mode)
{
	DecoderMaskByte(card, 0x166, 0x0C, (mode & 0x03) << 2);
}

// swap=0: L->L, R->R
// swap=1: L->R, R->L
void AudioSwap(struct cvdv_cards *card, int swap)
{
	DecoderMaskByte(card, 0x16B, 0x04, (swap ? 0x00 : 0x04));
}

// select=0: use clock from ACLK_441 pin -> ACLK=44.1kHz*N
// select=1: use clock from ACLK_48 pin  -> ACLK=48.0kHz*N
// select=2: use clock from ACLK_32 pin  -> ACLK=32.0kHz*N
//  Since the programmable sample rate generator of the PCM1723 is connected to 
//  all 3 of them, it doen't matter wich one you choose.
// divider=0: ACLK=768*Fs / S/PDIF-BCLK=ACLK/6 / DAC-BCLK=ACLK/12 / DAC-A_ACLK=ACLK/3
// divider=1: ACLK=768*Fs / S/PDIF-BCLK=ACLK/6 / DAC-BCLK=ACLK/12 / DAC-A_ACLK=ACLK/2
// divider=2: ACLK=512*Fs / S/PDIF-BCLK=ACLK/4 / DAC-BCLK=ACLK/8  / DAC-A_ACLK=ACLK/2
// divider=3: ACLK=384*Fs / S/PDIF-BCLK=ACLK/3 / DAC-BCLK=ACLK/6  / DAC-A_ACLK=ACLK/1
// divider=4: ACLK=256*Fs / S/PDIF-BCLK=ACLK/2 / DAC-BCLK=ACLK/4  / DAC-A_ACLK=ACLK/1
// divider=5: ACLK=768*48kHz / S/PDIF-BCLK=ACLK/6 / DAC-BCLK=ACLK/6  / DAC-A_ACLK=ACLK/1
// divider=6: ACLK=512*48kHz / S/PDIF-BCLK=ACLK/4 / DAC-BCLK=ACLK/4  / DAC-A_ACLK=ACLK/1
// divider=C: ACLK=768*48kHz / S/PDIF-BCLK=ACLK/9 / DAC-BCLK=ACLK/18 / DAC-A_ACLK=ACLK/3
// divider=D: ACLK=512*48kHz / S/PDIF-BCLK=ACLK/6 / DAC-BCLK=ACLK/12 / DAC-A_ACLK=ACLK/3
// divider=E: ACLK=512*48kHz / S/PDIF-BCLK=ACLK/6 / DAC-BCLK=ACLK/12 / DAC-A_ACLK=ACLK/2
// divider=F: ACLK=256*48kHz / S/PDIF-BCLK=ACLK/3 / DAC-BCLK=ACLK/6  / DAC-A_ACLK=ACLK/1
//  Fs is the audio sample frequency
//  For the normal cases, (32, 44.1, and 48 kHz) select divider 0 through 4 and set 
//  sample frequency in PCM1723 accordingly
//  For 96 kHz, select divider 5 or 6, and set PCM1723 to 48kHz*768 or *512 resp.
//  Divider C through F are for 32 kHz sample frequency with a 48kHz*x ACLK
void AudioSetACLK(struct cvdv_cards *card, int select, int divider)
{
	DecoderMaskByte(card, 0x16B, 0x03, select & 0x03);
	DecoderMaskByte(card, 0x16C, 0x0F, divider & 0x0F);
}

int AudioOpen(struct cvdv_cards *card)
{
	// initialize the audio card
	MDEBUG(1, ": -- AudioOpen\n");
	write_indexed_register(card, IIO_OSC_AUD, 0x10);
	return 0;
}

int AudioClose(struct cvdv_cards *card)
{
	MDEBUG(1, ": -- AudioClose\n");
	card->AudioInitialized = 0;
	return 0;
}

// audiorate: 16, 32, 64, 22(.05), 44(.1), 88(.2), 24, 48, 96 kHz
// surround=0: Stereo
// surround=1: Surround
int AudioInit(struct cvdv_cards *card, int audiorate, int surround)
{
	//if ((audiorate!=44) && (audiorate!=32)) audiorate=48;
	MDEBUG(1, ": -- AudioInit %d\n", audiorate);

	DACSetFrequency(card, audiorate, 256);	// put Fs*256 on ACLK inputs

	if (audiorate == 96)
		AudioSetACLK(card, 1, 0x06);	// 512*48kHz at ACLK
	else
		AudioSetACLK(card, 1, 0x04);	// 256 times Fs at ACLK

	DecoderDelByte(card, 0x166, 80);	// no mute on error
	DecoderWriteByte(card, 0x168, 0xFF);	// dynscalehigh
	DecoderWriteByte(card, 0x169, 0xFF);	// dynscalelow
	DecoderWriteByte(card, 0x16A, 0xFF);	// PCM scale

	// IEC958 Setup
	DecoderDelByte(card, 0x16D, 0x20);	// Overwrite Emphasis off
	DecoderSetByte(card, 0x16D, 0x40);	// Copyright Override
	DecoderDelByte(card, 0x16D, 0x80);	// Copyright Bit off
	DecoderDelByte(card, 0x16E, 0x01);	// Overwrite Category off
	DecoderDelByte(card, 0x16E, 0x08);	// Overwrite Quatization off
	DecoderSetByte(card, 0x170, 0x08);	// Musicam Stream Debug

	AudioAC3Mode(card, (surround ? 1 : 0));
	AudioAC3Compression(card, 2);
	AudioAC3Formatter(card, 0);

	AudioDualMono(card, 0);
	AudioSwap(card, 0);

	AudioMute(card, 0);
//  AudioSetPlayMode(card,MAUDIO_PLAY);

	card->AudioInitialized = 1;
	return 0;
}


// returns size of the Video ES Buffer in bytes or 0=error
u32 DecoderGetAudioESSize(struct cvdv_cards * card)
{
	if (!card->ChannelBuffersAllocated)
		return 0;	// buffer not initialised
	return (u32) ((DecoderReadWord(card, 0x04E) & 0x3FFF) -
		      (DecoderReadWord(card, 0x04C) & 0x3FFF)) * 256;	// bytes
}

// returns level of fullness in bytes
u32 DecoderGetAudioESLevel(struct cvdv_cards * card)
{
	u32 items;
	items = DecoderReadByte(card, 0x089);
	items |= ((DecoderReadWord(card, 0x08A) & 0x07FF) << 8);
	items *= 8;		// 64 bit per item
	return items;
}

int DecoderKaraoke(struct cvdv_cards *card, int vocal1, int vocal2,
		   int melody)
{
	DecoderMaskByte(card, 0x18C, 0x40, ((vocal1) ? 0x40 : 0x00));
	DecoderMaskByte(card, 0x18C, 0x80, ((vocal2) ? 0x80 : 0x00));
	DecoderMaskByte(card, 0x18C, 0x20, ((melody) ? 0x20 : 0x00));
	return 0;
}
