/* 
    cvdv.h

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

#ifndef RINGBUFFY_H
#define RINGBUFFY_H


#define FULL_BUFFER  -1000
typedef struct ringbuffy{
	long read_pos;
	long write_pos;
	long size;
	char *buffy;
} ringbuffy;

int  ring_init (ringbuffy *rbuf, long size);
void ring_destroy(ringbuffy *rbuf);
int ring_write(ringbuffy *rbuf, const char *data, int count);
int ring_writek(ringbuffy *rbuf, const char *data, int count);
int ring_read(ringbuffy *rbuf, char *data, int count);
long ring_read_rest(ringbuffy *rbuf);
long ring_write_rest(ringbuffy *rbuf);
void ring_flush(ringbuffy *rbuf);
int ring_read_direct(ringbuffy *rbuf, int addr, int count);

#endif /* RINGBUFFY_H */
