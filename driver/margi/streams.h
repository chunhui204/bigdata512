/* 
    streams.h

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

#ifndef CVDV_STREAMS_H
#define CVDV_STREAMS_H

#include "cardbase.h"

// Frees allocated channel buffers
int DecoderKillChannelBuffers(struct cvdv_cards *card);

// Allocates channel buffers
// All sizes in bytes, preferably multiple of 256 (will be rounded up otherwise)
int DecoderSetChannelBuffers(struct cvdv_cards *card, int VideoES,	// Video ES Channel Buffer size, e.g. 229376 byte for NTSC
			     int AudioES,	// Audio ES Channel Buffer size, 4096 byte
			     int VideoPES,	// Video PES Header / SPU Channel Buffer size, 512 byte
			     int DataDump,	// Data Dump Channel Buffer size, e.g. 80896 byte
			     int AudioPES,	// Audio PES Header / System Channel Buffer size, 512 byte
			     int NaviBank);	// Navi Bank Channel Buffer size, 2048 byte

//int DecoderReadFifo

int DecoderUnPrepare(struct cvdv_cards *card);

void DecoderPrepare(struct cvdv_cards *card);

// Selects audio type MPEG and sets stream ID's
// AID:     -1=all MPEG, Audio Stream ID: 0..31
// AExt:    -1=unused, Audio Stream Extension ID: 0..31, only used if AType=5
void DecoderSelectAudioID(struct cvdv_cards *card);

// AHeader: 0=No Headers, 1=first PTS/DTS header, 2=all headers, 3=All with PTS/DTS
// AType:   0=disable audio, 1=MPEG ID (MPEG 1), 2=Lin.PCM ID, 3=AC3 ID, 4=all MPEG (use only, if just one MPEG audio stream), 5=MPEG multichannel ID (MPEG 2)
// AID:     -1=all MPEG, Audio Stream ID: 0..31
// AExt:    -1=unused, Audio Stream Extension ID: 0..31, only used if AType=5
// IEC956:  0:MPEG/AC3 data on digital out 1:IEC956 data on digital S/PDIF out
void DecoderPrepareAudio(struct cvdv_cards *card);

// VHeader: -1=disable Video, 0=No Headers, 1=first PTS/DTS header, 2=all headers, 3=All with PTS/DTS
// VID: -1=all MPEG, 0..15=Video Stream ID
void DecoderPrepareVideo(struct cvdv_cards *card);

// Prepare Decoder for Elementary Streams, Disable Preparser
int DecoderPrepareES(struct cvdv_cards *card);

// Prepare Decoder for Packetised Elementary Streams, set parameters of Preparser
int DecoderPreparePES(struct cvdv_cards *card);


// Prepare Decoder for MPEG 1 Systems Streams or MPEG 2 Program Streams
// SPUID: -1:ignore, 0...15 SPU Substream ID
// DataDump: 0:disable data dump stream, 1:enable data dump stream
// PackHeader: 0:write no headers, 1:write one header, 2:write all headers, 3:always discard
// SysHeaader: 0:always discard, 1:write one header, 2:write all headers, 3:always discard
// DSIHeader: 0:write no DSI or PCI headers, 3:write DSI and PCI headers + packets
// DVD: 0: normal MPEG-2 data, 1: DVD stream with navi pack data
int DecoderPreparePS(struct cvdv_cards *card,
		     int SPUID, int DataDump,
		     int PackHeader, int SysHeader, int DSIHeader,
		     int DVD);

#endif				/* CVDV_STREAMS_H */
