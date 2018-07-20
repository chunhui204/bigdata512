/* 
    ringbuffy.c

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

#include "margi.h"
#include "ringbuffy.h"

#ifndef outsl_ns
#define outsl_ns outsl
#endif

int ring_init (ringbuffy *rbuf, long size)
{
	rbuf->size = 0;
	rbuf->read_pos = 0;	
	rbuf->write_pos = 0;
	
	if (size > 0){
		if( !(rbuf->buffy = (char *) vmalloc(sizeof(char)*size)) ){
			MDEBUG(0, 
			       "Not enough memory for ringbuffy\n");
			return -1;
		}
	} else {
		MDEBUG(0, "Wrong size for ringbuffy\n");
		return -1;
	}

	rbuf->size = size;
	return 0;
}


void ring_destroy(ringbuffy *rbuf)
{
	if (rbuf->size){
		vfree(rbuf->buffy);
		rbuf->buffy = NULL;
	}
	rbuf->size = 0;
	rbuf->read_pos = 0;	
	rbuf->write_pos = 0;
}


int ring_write(ringbuffy *rbuf, const char *data, int count)
{

	long diff, free, pos, rest;

      
	if (count <=0 || !rbuf->buffy) return 0;
       	pos  = rbuf->write_pos;
	rest = rbuf->size - pos;
	diff = rbuf->read_pos - pos;
	free = (diff > 0) ? diff-4 : rbuf->size+diff-4;

	if ( free <= 0 ) return 0;
	if ( free < count ) count = free;
	
	if (count >= rest){
		if(copy_from_user (rbuf->buffy+pos, data, rest))
		  return -EFAULT;
		if (count - rest)
			if(copy_from_user(rbuf->buffy, data+rest, 
					  count - rest))
			  return -EFAULT;
		rbuf->write_pos = count - rest;
	} else {
		copy_from_user (rbuf->buffy+pos, data, count);
		rbuf->write_pos += count;
	}

	return count;
}


int ring_read(ringbuffy *rbuf, char *data, int count)
{

	long diff, free, pos, rest;


	if (count <=0 || !rbuf->buffy) return 0;
	pos  = rbuf->read_pos;
	rest = rbuf->size - pos;
	diff = rbuf->write_pos - pos;
	free = (diff >= 0) ? diff : rbuf->size+diff;

	if ( free <= 0 ) return 0;
	if ( free < count ) count = free;
	
	if ( count >= rest ){
		memcpy(data,rbuf->buffy+pos,rest);
		if ( count - rest)
			memcpy(data+rest,rbuf->buffy,count-rest);
		rbuf->read_pos = count - rest;
	} else {
		memcpy(data,rbuf->buffy+pos,count);
		rbuf->read_pos += count;
	}

	return count;
}

int ring_read_direct(ringbuffy *rbuf, int addr, int count)
{

	long diff, free, pos, rest;


	if (count <=0 || !rbuf->buffy) return 0;
	pos  = rbuf->read_pos;
	rest = rbuf->size - pos;
	diff = rbuf->write_pos - pos;
	free = (diff >= 0) ? diff : rbuf->size+diff;

	if ( free <= 0 ) return 0;
	if ( free < count ) count = free;
	
	if ( count >= rest ){
		outsl_ns(addr,rbuf->buffy+pos,rest/4);
		if ( count - rest)
			outsl_ns(addr,rbuf->buffy,(count-rest)/4);
		rbuf->read_pos = count - rest;
	} else {
		outsl_ns(addr,rbuf->buffy+pos,count/4);
		rbuf->read_pos += count;
	}

	return count;
}


long ring_read_rest(ringbuffy *rbuf){
       	long diff, free, pos;

	if (!rbuf->buffy) return 0;
	pos  = rbuf->read_pos;
	diff = rbuf->write_pos - pos;
	free = (diff >= 0) ? diff : rbuf->size+diff;
	
	return free;
}

long ring_write_rest(ringbuffy *rbuf){
       	long diff, free, pos;

	if (!rbuf->buffy) return 0;
       	pos  = rbuf->write_pos;
	diff = rbuf->read_pos - pos;
	free = (diff > 0) ? diff-4 : rbuf->size+diff-4;
	
	return free;
}

void ring_flush(ringbuffy *rbuf){
	rbuf->read_pos  = 0;
	rbuf->write_pos = 0;
}
