/* 
    cvdvtypes.h

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

#ifndef _CVDVEXT_H_
#define _CVDVEXT_H_

#include <sys/ioctl.h>
#include "cvdvtypes.h"

#define OSD_Draw(file,cmd) ioctl(fileno(file),_IO(CVDV_IOCTL_MAGIC,IOCTL_DRAW),&cmd)
#define DecoderCommand(file,cmd) ioctl(fileno(file),_IO(CVDV_IOCTL_MAGIC,IOCTL_DECODER),&cmd)

#endif  // _CVDVEXT_H_
