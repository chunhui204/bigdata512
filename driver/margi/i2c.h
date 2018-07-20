/* 
    i2c.h

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

#ifndef I2C_H
#define I2C_H
#include "cardbase.h"
#include "l64014.h"
#include "margi.h"

void out(struct cvdv_cards *card);
void clkon(struct cvdv_cards *card);
void clkoff(struct cvdv_cards *card);
void dat(struct cvdv_cards *card, u_char data);
int rdat(struct cvdv_cards *card);
void I2CStart(struct cvdv_cards *card);
void I2CStop(struct cvdv_cards *card);
int I2CAck(struct cvdv_cards *card, int ack);
u_char I2CReadByte(struct cvdv_cards *card, int ack);
int I2CSendByte(struct cvdv_cards *card, u_char data);
void I2CWrite(struct cvdv_cards *card, int adr, int reg, int val);
u_char I2CRead(struct cvdv_cards *card, int adr, int reg);
int I2CScan(struct cvdv_cards *card, int adr);
void I2CScanBus(struct cvdv_cards *card);
void I2CSend(struct cvdv_cards *card, int adr, u_char * vals);

#endif				/* I2C_H */
