/* 
    spu.c

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

#include "spu.h"
#include "l64021.h"

int DecoderHighlight(struct cvdv_cards *card, int active, u8 * coli,
		     u8 * btn_posi)
{
	int i;
	if ((coli == NULL) || (btn_posi == NULL))
		return 1;
	MDEBUG(0,": -- DecoderHighlight: col 0x%02X%02X, contr 0x%02X%02X, act %d, %d,%d - %d,%d\n",
	       coli[0], coli[1], coli[2], coli[3], active,
	       (((int) btn_posi[0] & 0x3F) << 4) | (btn_posi[1] >> 4),
	       (((int) btn_posi[3] & 0x3F) << 4) | (btn_posi[4] >> 4),
	       (((int) btn_posi[1] & 0x03) << 8) | btn_posi[2],
	       (((int) btn_posi[4] & 0x03) << 8) | btn_posi[5]);
	//for (i=0; i<4; i++) DecoderWriteByte(card,0x1C0+i,coli[i]);
//  DecoderWriteByte(card,0x1C0,coli[1]);
//  DecoderWriteByte(card,0x1C1,coli[0]);
//  DecoderWriteByte(card,0x1C2,coli[3]);
//  DecoderWriteByte(card,0x1C3,coli[2]);
	//for (i=0; i<6; i++) DecoderWriteByte(card,0x1C4+i,btn_posi[i]);
//  for (i=0; i<6; i++) DecoderWriteByte(card,0x1C4+i,btn_posi[5-i]);
	//if (active) DecoderSetByte(card,0x1BF,0x01);
	//else        DecoderDelByte(card,0x1BF,0x01);

	//for (i=0; i<4; i++) card->highlight[i]=coli[3-i];
	card->highlight[0] = coli[1];
	card->highlight[1] = coli[0];
	card->highlight[2] = coli[3];
	card->highlight[3] = coli[2];
	for (i = 0; i < 6; i++)
		card->highlight[4 + i] = btn_posi[5 - i];
	card->highlight_valid = 1;
	if (active)
		DecoderWriteByte(card, 0x1BF, 0x01);
	else
		DecoderWriteByte(card, 0x1BF, 0x00);
//DecoderSetByte(card,0x135,0x02);  // Enable SPU Mix
//DecoderWriteByte(card,0x1A0,0x01);  // decode start, display on
	return 0;
}

int DecoderSPUPalette(struct cvdv_cards *card, int length, u8 * palette)
{
	int i;
	MDEBUG(1,": -- DecoderSPUPalette: setting up %d bytes of SPU palette(Y,Cr,Cb):", length);
	for (i = 0; i < (length / 3); i++)
		MDEBUG(1," %d=(%d,%d,%d)", i, palette[i * 3],palette[i * 3 + 1], 
		       palette[i * 3 + 2]);
	MDEBUG(1,"\n");
	DecoderDelByte(card, 0x1A0, 0x01);	// SPU decode stop
	DecoderSetByte(card, 0x1A0, 0x10);
	for (i = 0; i < length; i++)
		DecoderWriteByte(card, 0x1BE, palette[i]);
	DecoderSetByte(card, 0x1A0, 0x01);	// SPU decode start
	return 0;
}

int DecoderSPUStream(struct cvdv_cards *card, int stream, int active)
{
	MDEBUG(1,": -- DecoderSPUStream: stream %d, active %d\n", stream,
	       active);
	if (stream < 32) {
		card->reg092 |= (0x20 | (stream & 0x1F));	// stream ID and select
		DecoderWriteByte(card, 0x092, card->reg092);
		DecoderMaskByte(card, 0x112, 0x20, 0x20);	// chroma filter enable
		DecoderMaskByte(card, 0x1A1, 0x0F, 0x00);	// SPU timeout
		DecoderWriteByte(card, 0x1BF, 0x00);	// HighLight off
		DecoderSetByte(card, 0x135, 0x02);	// Enable SPU Mix
		if (active)
			DecoderWriteByte(card, 0x1A0, 0x01);	// decode start, display on
		else
			DecoderWriteByte(card, 0x1A0, 0x05);	// decode start, display off
	} else {
		DecoderWriteByte(card, 0x1A0, 0x04);	// decode stop, display off
		card->reg092 &= (~0x20);	// stream select off
		DecoderWriteByte(card, 0x092, card->reg092);
		DecoderDelByte(card, 0x135, 0x02);	// Disable SPU Mix
	}
	return 0;
}
