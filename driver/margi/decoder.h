/* 
    decoder.h

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

#ifndef CVDV_DECODER_H
#define CVDV_DECODER_H

#include "cardbase.h"


int DecoderGetNavi(struct cvdv_cards *card, u8 * navidata);

// returns 1 on overrun, 0 on no error
int DecoderQueueNavi(struct cvdv_cards *card, u8 * navidata);

u32 ParseSCR(const u8 * scrdata);

u32 SetSCR(struct cvdv_cards *card, u32 SCR_base);

void DecoderPause(struct cvdv_cards *card);

void DecoderUnPause(struct cvdv_cards *card);

void CloseCard(struct cvdv_cards *card);


void DecoderReadAudioInfo(struct cvdv_cards *card);

void DecoderReadAuxFifo(struct cvdv_cards *card);

void DecoderReadDataFifo(struct cvdv_cards *card);

int DecoderReadNavipack(struct cvdv_cards *card);

int AudioStart(struct cvdv_cards *card);

// Puts decoder in pause after so many fields
void StepsToPause(struct cvdv_cards *card, int steps);

void L64021Intr(struct cvdv_cards *card);
//static void L64021Intr(struct cvdv_cards *card);

// Enable the IRQ Masks
void L64021InstallIntr(struct cvdv_cards *card);

int L64021RemoveIntr(struct cvdv_cards *card);

int L64021Reset(struct cvdv_cards *card);

int L64021Setup(struct cvdv_cards *card);

int L64021Init(struct cvdv_cards *card);

#endif				/* CVDV_DECODER_H */
