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

#define __NO_VERSION__

#include "i2c.h"

void out(struct cvdv_cards *card)
{
	write_indexed_register(card, IIO_GPIO_PINS,
			       (card->scl ? SCL : 0) |
			       (card->sda ? SDA : 0) | 1);
	udelay(10);
}

void clkon(struct cvdv_cards *card)
{
	card->scl = 1;
}

void clkoff(struct cvdv_cards *card)
{
	card->scl = 0;
}

void dat(struct cvdv_cards *card, u_char data)
{
	card->sda = data;
}

int rdat(struct cvdv_cards *card)
{
	return ((read_indexed_register(card, IIO_GPIO_PINS) & SDA) ? 1 :
		0);
}


void I2CStart(struct cvdv_cards *card)
{
	dat(card, 1);
	out(card);
	clkon(card);
	out(card);
	dat(card, 0);
	out(card);
	clkoff(card);
	out(card);
}

void I2CStop(struct cvdv_cards *card)
{
	dat(card, 0);
	out(card);
	clkon(card);
	out(card);
	dat(card, 1);
	out(card);
	clkoff(card);
	out(card);
}

int I2CAck(struct cvdv_cards *card, int ack)
{
	dat(card, ack);
	out(card);
	write_indexed_register(card, IIO_GPIO_CONTROL, (~SDA) & 0x07);
	clkon(card);
	out(card);
	ack = rdat(card);
	clkoff(card);
	out(card);
	write_indexed_register(card, IIO_GPIO_CONTROL, 0x07);
	out(card);
	return ack;
}

u_char I2CReadByte(struct cvdv_cards * card, int ack)
{
	int i;
	u_char data = 0;

	clkoff(card);
	dat(card, 1);
	out(card);
	write_indexed_register(card, IIO_GPIO_CONTROL, (~SDA) & 0x07);
	for (i = 7; i >= 0; i--) {
		clkon(card);
		out(card);
		data |= (rdat(card) << i);
		clkoff(card);
		out(card);
	}
	write_indexed_register(card, IIO_GPIO_CONTROL, 0x07);
	I2CAck(card, ack);
	return data;
}


int I2CSendByte(struct cvdv_cards *card, u_char data)
{
	int i;

	for (i = 7; i >= 0; i--) {
		dat(card, data & (1 << i));
		out(card);
		clkon(card);
		out(card);
		clkoff(card);
		out(card);
	}
	i = I2CAck(card, 1);
	return i;
}

void I2CWrite(struct cvdv_cards *card, int adr, int reg, int val)
{
	I2CStart(card);
	I2CSendByte(card, adr);
	I2CSendByte(card, reg);
	I2CSendByte(card, val);
	I2CStop(card);
}


u_char I2CRead(struct cvdv_cards *card, int adr, int reg)
{
	u_char c;

	I2CStart(card);
	I2CSendByte(card, adr);
	I2CSendByte(card, reg);
	I2CStart(card);
	I2CSendByte(card, adr | 1);
	c = I2CReadByte(card, 1);
	I2CStop(card);
	return c;
}


int I2CScan(struct cvdv_cards *card, int adr)
{
	int result;
	I2CStart(card);
	result = I2CSendByte(card, adr);
	I2CStop(card);
	return result;
}

void I2CScanBus(struct cvdv_cards *card)
{
	int i;

	for (i = 0; i < 0xff; i += 2) {
		if (!I2CScan(card, i))
			MDEBUG(0,"Found i2c device at %d\n", i);
	}
}

void I2CSend(struct cvdv_cards *card, int adr, u_char * vals)
{
	int reg, val;
	while (*vals != 0xff) {
		reg = *vals;
		vals++;
		val = *vals;
		vals++;
		I2CWrite(card, adr, reg, val);
	}
}
