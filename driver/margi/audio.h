/* 
    audio.h

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


#ifndef CVDV_AUDIO_H
#define CVDV_AUDIO_H

  //
 //  Audio Decoder
//
#define __NO_VERSION__
#include "cardbase.h"

#define MAUDIO_PAUSE 0
#define MAUDIO_PLAY 1
#define MAUDIO_FAST 2
#define MAUDIO_SLOW 3

// mode=0 pause
// mode=1 normal speed play
// mode=2 fast play, 16/15
// mode=3 slow play, 16/17
void AudioSetPlayMode(struct cvdv_cards *card, int mode);

void AudioStartDecode(struct cvdv_cards *card);

// Stop Decode flushes the Audio ES channel buffer
void AudioStopDecode(struct cvdv_cards *card);

void AudioStartFormat(struct cvdv_cards *card);

void AudioStopFormat(struct cvdv_cards *card);

//         Audio source:    S/PDIF out:
// mode 0: MPEG             IEC958
// mode 1: AC3              IEC958
// mode 2: MPEG             MPEG
// mode 3: AC3              AC3
// mode 4: PCM              IEC958 (max. 48kHz)
// mode 5: PCM 96->48kHz    IEC958 (48kHz)
// mode 6: CD Bypass        S/PDIF Bypass
// mode 7: PCM FIFO         PCM FIFO
void AudioSetMode(struct cvdv_cards *card, int mode);


// volume: 0..255
void AudioSetVolume(struct cvdv_cards *card, int volume);

// mute=1: mute audio
void AudioMute(struct cvdv_cards *card, int mute);

// mode=0: stereo
// mode=1: surround
void AudioAC3Mode(struct cvdv_cards *card, int mode);

// mode=0: custom analog
// mode=1: custom digital
// mode=2: line-out (default)
// mode=3: RF mode
void AudioAC3Compression(struct cvdv_cards *card, int mode);

// mode=0: AC3
// mode=1: ES1
void AudioAC3Formatter(struct cvdv_cards *card, int mode);

// mode=0: Stereo
// mode=1: Right channel only
// mode=2: Left channel only
// mode=3: Mono Mix
void AudioDualMono(struct cvdv_cards *card, int mode);

// swap=0: L->L, R->R
// swap=1: L->R, R->L
void AudioSwap(struct cvdv_cards *card, int swap);

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
void AudioSetACLK(struct cvdv_cards *card, int select, int divider);

int AudioOpen(struct cvdv_cards *card);

int AudioClose(struct cvdv_cards *card);

// audiorate: 16, 32, 64, 22(.05), 44(.1), 88(.2), 24, 48, 96 kHz
// surround=0: Stereo
// surround=1: Surround
int AudioInit(struct cvdv_cards *card, int audiorate, int surround);


// returns size of the Video ES Buffer in bytes or 0=error
u32 DecoderGetAudioESSize(struct cvdv_cards *card);

// returns level of fullness in bytes
u32 DecoderGetAudioESLevel(struct cvdv_cards *card);

int DecoderKaraoke(struct cvdv_cards *card, int vocal1, int vocal2,
		   int melody);

#endif				/* CVDV_AUDIO_H */
