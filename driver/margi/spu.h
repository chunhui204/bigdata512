/* 
    spu.h

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

#ifndef CVDV_SPU_H
#define CVDV_SPU_H

#include "cardbase.h"

int DecoderHighlight(struct cvdv_cards *card, int active, u8 * coli,
		     u8 * btn_posi);

int DecoderSPUPalette(struct cvdv_cards *card, int length, u8 * palette);

int DecoderSPUStream(struct cvdv_cards *card, int stream, int active);

#endif				/* CVDV_SPU_H */
