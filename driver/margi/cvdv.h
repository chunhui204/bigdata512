/* 
    cvdv.h

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

#ifndef _CVDV_H_
#define _CVDV_H_

     //////////////////////////////////////////////////////////
    //                                                      //
   //  Convergence Digital Video Decoder Card              //
  //  Definitions for the PCI-Card and the Char-Driver    //
 //                                                      //
//////////////////////////////////////////////////////////


#include "cardbase.h"
#include "dram.h"
#include "osd.h"
#include "crc.h"
#include "l64021.h"
#include "audio.h"
#include "video.h"
#include "streams.h"
#include "decoder.h"
#include "spu.h"

void SetVideoSystem(struct cvdv_cards *card);
u16 rnd(u16 range);
// debugging of the card: 0=normal, 1=color bars, 2=sync out
#define USE_DEBUG  0

#define cimlogo_width 45
#define cimlogo_height 38

#define CHANNELBUFFERSIZE 32768*8

int Prepare(struct cvdv_cards *card);
int OSDTest(struct cvdv_cards *card);
void v4l_init(struct cvdv_cards *card);
int dvb_register(struct cvdv_cards *card);
void dvb_unregister(struct cvdv_cards *card);
#endif				// _CVDV_H_
