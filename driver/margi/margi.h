/* 
    margi.h

    Copyright (C) Marcus Metzler for convergence integrated media.

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


#ifndef margi_cs_h
#define margi_cs_h

#include "cardbase.h"
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/ptrace.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <asm/system.h>


#define PBUFFER 100

u_char read_indexed_register(struct cvdv_cards *card, int addr);
void write_indexed_register(struct cvdv_cards *card, int addr,
			    u_char data);
void WriteByte(struct cvdv_cards *card, int addr, u_char data);
u_char ReadByte(struct cvdv_cards *card, int addr);
void MaskByte(struct cvdv_cards *card, int addr, u_char mask, u_char bits);
int MargiFreeBuffers(struct cvdv_cards *card);
int MargiSetBuffers(struct cvdv_cards *card, uint32_t size, int isB);
int MargiFlush (struct cvdv_cards *card);
int MargiPushA(struct cvdv_cards *card, int count, const char *data);
int MargiPushB(struct cvdv_cards *card, int count, const char *data);
int DecoderStartChannel(struct cvdv_cards *card);
int DecoderStopChannel(struct cvdv_cards *card);
void DACSetFrequency(struct cvdv_cards *card, int khz, int multiple);
stream_type get_stream_type(struct cvdv_cards *card);
audio_type get_audio_type(struct cvdv_cards *card);

void Timerfunction(unsigned long data);

#endif
