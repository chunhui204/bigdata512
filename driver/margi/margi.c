/* 
    margi.c

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

#include "margi.h"
#include <linux/module.h>
#include <pcmcia/cs_types.h>
#include <pcmcia/cs.h>
#include <pcmcia/cistpl.h>
#include <pcmcia/cisreg.h>
#include <pcmcia/bus_ops.h>
#include <pcmcia/ds.h>


#include "l64014.h"
#include "l64021.h"
#include "i2c.h"
#include "decoder.h"
#include "dram.h"
#include "video.h"
#include "cvdv.h"


static char *version = "margi_cs.c 0.6 02/04/2000 (Marcus Metzler)";

//#define USE_BH 1
#ifdef USE_BH
#define MARGI_BH 31
// shouldn't be a number, but then MARGI_BH must be entered into interrupt.h
#endif

MODULE_AUTHOR(AUTHOR);
MODULE_DESCRIPTION(MEDDEVNAME " Driver V." DVERSION);
#ifdef MODULE_LICENSE
MODULE_LICENSE("GPL");
#endif

#define MAX_DEV 4

/* Parameters that can be set with 'insmod' */
static int svhs = 1;
MODULE_PARM(svhs,"i");
static int composite = 1;
MODULE_PARM(composite,"i");
#ifdef INT_ZOOMED_VIDEO
static int use_zv = 1;
MODULE_PARM(use_zv,"i");
#else
static int use_zv = 0;
#define INT_ZOOMED_VIDEO 0
#warning NO ZV support in kernel
#endif

/* Release IO ports after configuration? */
static int free_ports = 0;

/* The old way: bit map of interrupts to choose from */
/* This means pick from 15, 14, 12, 11, 10, 9, 7, 5, 4, and 3 */
static u_int irq_mask = 0xdeb8;
/* Newer, simpler way of listing specific interrupts */
static int irq_list[4] = { -1 };

MODULE_PARM(free_ports, "i");
MODULE_PARM(irq_mask, "i");
MODULE_PARM(irq_list, "1-4i");

typedef struct margi_info_t {
	dev_link_t link;
	dev_node_t node;
	struct cvdv_cards card;
	int stop;
} margi_info_t;



/*
   The event() function is this driver's Card Services event handler.
   It will be called by Card Services when an appropriate card status
   event is received.  The config() and release() entry points are
   used to configure or release a socket, in response to card
   insertion and ejection events.  They are invoked from the margi
   event handler. 
*/

static void margi_config(dev_link_t * link);
static void margi_release(u_long arg);
static int margi_event(event_t event, int priority,
		       event_callback_args_t * args);
/*
   The attach() and detach() entry points are used to create and destroy
   "instances" of the driver, where each instance represents everything
   needed to manage one actual PCMCIA card.
*/

static dev_link_t *margi_attach(void);
static void margi_detach(dev_link_t *);
static u_char read_lsi_status(struct cvdv_cards *card);

/*
   You'll also need to prototype all the functions that will actually
   be used to talk to your device.  See 'memory_cs' for a good example
   of a fully self-sufficient driver; the other drivers rely more or
   less on other parts of the kernel.
*/

/*
   The dev_info variable is the "key" that is used to match up this
   device driver with appropriate cards, through the card configuration
   database.
*/

static dev_link_t *dev_table[MAX_DEV] = { NULL, /* ... */  };

static dev_info_t dev_info = "margi_cs";

/*
   A linked list of "instances" of the margi device.  Each actual
   PCMCIA card corresponds to one device instance, and is described
   by one dev_link_t structure (defined in ds.h).

   You may not want to use a linked list for this -- for example, the
   memory card driver uses an array of dev_link_t pointers, where minor
   device numbers are used to derive the corresponding array index.
*/

static dev_link_t *dev_list = NULL;

/*
   A dev_link_t structure has fields for most things that are needed
   to keep track of a socket, but there will usually be some device
   specific information that also needs to be kept track of.  The
   'priv' pointer in a dev_link_t structure can be used to point to
   a device-specific private data structure, like this.

   To simplify the data structure handling, we actually include the
   dev_link_t structure in the device's private data structure.

   A driver needs to provide a dev_node_t structure for each device
   on a card.  In some cases, there is only one device per card (for
   example, ethernet cards, modems).  In other cases, there may be
   many actual or logical devices (SCSI adapters, memory cards with
   multiple partitions).  The dev_node_t structures need to be kept
   in a linked list starting at the 'dev' field of a dev_link_t
   structure.  We allocate them in the card's private data structure,
   because they generally shouldn't be allocated dynamically.

   In this case, we also provide a flag to indicate if a device is
   "stopped" due to a power management event, or card ejection.  The
   device IO routines can use a flag like this to throttle IO to a
   card that is not ready to accept it.

   The bus_operations pointer is used on platforms for which we need
   to use special socket-specific versions of normal IO primitives
   (inb, outb, readb, writeb, etc) for card IO.
*/

void DACSetFrequency(struct cvdv_cards *card, int khz, int multiple) {
	uint8_t b = 	read_indexed_register(card, IIO_OSC_AUD);

	b &= 0xf8;

	switch (khz){
	case 32:
		b |= 0x04;
		break;
	case 48:
		b |= 0x00;
		break;
	case 44:
		b |= 0x01;
		break;
	case 96:
		b |= 0x02;
		break;
	default:
		b |= 0x00;
		break;
   	}
	write_indexed_register(card, IIO_OSC_AUD, b);

}

int MargiFreeBuffers(struct cvdv_cards *card)
{
	MDEBUG(1, ": -- MargiFreeBuffers\n");
	
	ring_destroy(&(card->rbufB));
	card->use_ringB = 0;
	ring_destroy(&(card->rbufA));
	card->use_ringA = 0;
	
	return 0;
}


int MargiSetBuffers(struct cvdv_cards *card, uint32_t size, int isB)
{
	int err = 0;

	MDEBUG(0, ": -- MargiSetBuffers(%d) %d\n",
	       size, isB);

	if (isB){
		err = ring_init(&(card->rbufB),size);
		if (!err) card->use_ringB = 1;
	} else {
		err = ring_init(&(card->rbufA),size);
		if (!err) card->use_ringA = 1;
	}
	
	MDEBUG(0,"set buffers: %d use_ringA: %d  use_ringB: %d\n",err,
card->use_ringA,card->use_ringB);
	return err;
}


int MargiFlush (struct cvdv_cards *card)
{
	int co = 0;
	int i;
	for (i=0;i<100;i++){
		MargiPushA(card, 32, FlushPacket);
		MargiPushB(card, 32, FlushPacket);
	}
	while ( (ring_write_rest(&(card->rbufA))|| ring_write_rest(&(card->rbufB)))  && co<100) 
		co++;
	VideoSetBackground(card, 1, 0, 0, 0);	// black

	if (card->use_ringA) ring_flush(&(card->rbufA));
	if (card->use_ringB) ring_flush(&(card->rbufB));
	card->DMAABusy = 0;
	card->DMABBusy = 0;


	DecoderStopChannel(card);
	DecoderStreamReset(card);
	DecoderSetupReset(card);
	card->channelrun = 0;

	MDEBUG(1, ": Margi Flush \n");
	return 0;
}


int MargiPushA(struct cvdv_cards *card, int count, const char *data)
{
	int fill;
  
	fill =  ring_read_rest(&(card->rbufA));

	if (!card->use_ringA)
		return 0;
	if ((count>fill || fill > 3*card->rbufA.size/4)
	    && !card->channelrun){
		DecoderStartChannel(card);
		card->DMAABusy = 1;
	}

	count = ring_write(&(card->rbufA),data,count);

	return count;
}

int MargiPushB(struct cvdv_cards *card, int count, const char *data)
{
	int fill;
  
	fill =  ring_read_rest(&(card->rbufB));

	if (!card->use_ringB)
		return 0;
	if ((count>fill || fill > 3*card->rbufB.size/4)
	    && !card->channelrun){
		DecoderStartChannel(card);
		card->DMABBusy = 1;
	}

	count = ring_write(&(card->rbufB),data,count);

	return count;
}

int DecoderStartChannel(struct cvdv_cards *card)
{
	DecoderMaskByte(card, 0x007, 0xC3, 0xC3);	// channel start

#ifdef BYPASS 
	DecoderMaskByte(card,0x005,0x0F,0x08);
#else
	DecoderMaskByte(card,0x005,0x0F,0x01);
#endif
	card->channelrun = 1;
	return 0;
}

int DecoderStopChannel(struct cvdv_cards *card)
{
	DecoderMaskByte(card, 0x007, 0xC3, 0xC2);	// channel reset
	DecoderSetByte(card, 0x005, 0x04);	// channel pause
	card->channelrun = 0;
	return 0;
}

uint32_t DecoderGetAudioBufferSpace(struct cvdv_cards *card)
{

	uint32_t MaxSize, Size;

	MaxSize = card->AudioESSize;
	Size = DecoderGetAudioESLevel(card);

	if (Size>MaxSize)
	  return 0;
	return (MaxSize - Size);

}

uint32_t DecoderGetVideoBufferSpace(struct cvdv_cards *card)
{

	uint32_t MaxSize, Size;

	MaxSize = card->VideoESSize;
	Size = DecoderGetVideoESLevel(card);

	if (Size>MaxSize)
	  return 0;
	return (MaxSize - Size);

}

uint32_t DecoderGetBufferSpace(struct cvdv_cards *card)
{
	uint32_t audio,video;
	
	audio = DecoderGetAudioBufferSpace(card);
	video = DecoderGetVideoBufferSpace(card);

	if (audio > 2048) audio -= 2048;
	if (video > 2048) video -= 2048;

	if (audio < video) return audio;
	return video;
}



static int ringDMA (struct cvdv_cards *card){
	
	uint32_t size = 0;
	u_char stat;
	dev_link_t *link = &(((margi_info_t *) card->margi)->link);
	uint32_t acount=0;
	uint32_t vcount=0;
	uint8_t data;
	ringbuffy *buffy;
	int stype;
	wait_queue_head_t *wq;
	stat = read_lsi_status(card);
	

	stype = card->setup.streamtype;

	if (stat & LSI_ARQ) {
		stat = read_lsi_status(card);
	}

	if (stat & LSI_READY){
		data = read_indexed_register(card, IIO_LSI_CONTROL);
		data |= RR;
		write_indexed_register(card, IIO_LSI_CONTROL, data);
		return 0;
	}

	if ((stat & LSI_ARQ) == 0) {
		switch(stype){
		case stream_PES:
		case stream_ES:
			data = read_indexed_register(card, IIO_LSI_CONTROL);
			data &= ~DSVC;
			write_indexed_register(card, IIO_LSI_CONTROL, data);
			buffy = &card->rbufB;
			wq = &(card->wqB);
			acount = ring_read_rest(buffy);
			size = DecoderGetAudioBufferSpace(card);
			if (size > 2048) size -= 2048;
			break;
		default:
			buffy = &card->rbufA;
			wq = &(card->wqA);
			acount = ring_read_rest(buffy);
			size = DecoderGetBufferSpace(card);
			break;
		}
		if (acount > size) acount = size & 0xfffffffc;
		if (acount>=2048) acount &=0xfffff800;
		acount &=0xfffffffc;
	       
		if (acount > size) acount = size & 0xfffffffc;
		if (acount) {
			ring_read_direct(buffy,
					 link->io.BasePort1+DIO_LSI_STATUS, 
					 acount);
		} else {
			wake_up_interruptible(wq);
			acount = 0;
		}
	} else {
		acount = 0;
	}

	if ((stat & LSI_VRQ) == 0 && 
	    (stype == stream_PES || stype == stream_ES)) {
		data = read_indexed_register(card, IIO_LSI_CONTROL);
		data |= DSVC;
		write_indexed_register(card, IIO_LSI_CONTROL, data);
		buffy = &card->rbufA;
		wq = &(card->wqA);
		vcount = ring_read_rest(buffy);

	        size = DecoderGetVideoBufferSpace(card);
		if (size > 2048) size -= 2048;
		if (vcount > size) vcount = size & 0xfffffffc;
		if (vcount>=2048) vcount &=0xfffff800;
		vcount &=0xfffffffc;
	       
		if (vcount > size) vcount = size & 0xfffffffc;
		if (vcount) {
			ring_read_direct(buffy,
					 link->io.BasePort1+DIO_LSI_STATUS, 
					 vcount);
		} else {
			wake_up_interruptible(wq);
			vcount = 0;
		}
	} else {
		vcount = 0;
	}

	return vcount+acount;
}


u_char read_indexed_register(struct cvdv_cards * card, int addr)
{
	dev_link_t *link = &(((margi_info_t *) card->margi)->link);
	u_char data;
	spin_lock(&card->timelock);

	outb(addr, link->io.BasePort1 + DIO_CONTROL_INDEX);
	data = (inb(link->io.BasePort1 + DIO_CONTROL_DATA));

 	spin_unlock(&card->timelock);
	return data;
}


void write_indexed_register(struct cvdv_cards *card, int addr, u_char data)
{
	dev_link_t *link = &(((margi_info_t *) card->margi)->link);
	spin_lock(&card->timelock);

	outb(addr, link->io.BasePort1 + DIO_CONTROL_INDEX);
	outb(data, link->io.BasePort1 + DIO_CONTROL_DATA);

	spin_unlock(&card->timelock);
}

void WriteByte(struct cvdv_cards *card, int addr, u_char data)
{
	dev_link_t *link = &(((margi_info_t *) card->margi)->link);

	spin_lock(&card->timelock);

	outb((u_char) (addr & 255),
	     link->io.BasePort1 + DIO_LSI_INDEX_LOW);
	outb(((addr & 256) ? 1 : 0),
	     link->io.BasePort1 + DIO_LSI_INDEX_HIGH);
	outb(data, link->io.BasePort1 + DIO_LSI_DATA);

	spin_unlock(&card->timelock);
}

u_char ReadByte(struct cvdv_cards *card, int addr)
{
	dev_link_t *link = &(((margi_info_t *) card->margi)->link);
	u_char data;

	spin_lock(&card->timelock);

	outb((u_char) (addr & 255),
	     link->io.BasePort1 + DIO_LSI_INDEX_LOW);
	outb(((addr & 256) ? 1 : 0),
	     link->io.BasePort1 + DIO_LSI_INDEX_HIGH);
	data = inb(link->io.BasePort1 + DIO_LSI_DATA);

	spin_unlock(&card->timelock);
	return data;
}

void MaskByte(struct cvdv_cards *card, int addr, u_char mask, u_char bits)
{
	WriteByte(card, addr, (ReadByte(card, addr) & ~(mask)) | (bits));
}



#define MAXWRITE CHANNELBUFFERSIZE/2
#define MAX_COUNT 400

#ifdef USE_BH
struct cvdv_cards *bh_card;

static void do_margi_bh(void)
{
	struct cvdv_cards *card = bh_card;
#else

static void do_margi(struct cvdv_cards *card)
{

#endif
	int countA;
	int try;
	int stype = card->setup.streamtype;

	countA = 0;

	card->currentType = 0;
	for ( try = 0; try < MAX_COUNT ;try++)
		if (countA < MAXWRITE){
			int count = 0;
			switch (stype){
			case stream_PES:
			case stream_ES:
				count = ringDMA(card);
				countA += count;
				if (!count) 
					try=MAX_COUNT;			
				break;
			case stream_PS:
			case stream_DVD:
				count = ringDMA(card);
				countA += count;
				if (!count) 
					try=MAX_COUNT;
				break;
			}
		} else break;

}




void L64014Intr_function(struct cvdv_cards *card)
{
	uint8_t control,mask,stat;
	int try;


	control= read_indexed_register(card, IIO_IRQ_CONTROL);
	if (control & IRQ_EN){
		mask = 0;
		if ( control & DEC_EN ) mask |= DEC_INT;
		if ( control & VSYNC_EN ) mask |= VSYNC_INT;
		stat = read_indexed_register(card, IIO_IRQ_STATUS);
		try = 0;
		while ( (try++ < 100) && (stat & mask) ){		      
		
		  if (stat & VSYNC_INT) {
	
				write_indexed_register(card,IIO_IRQ_CONTROL,
						       control & (~VSYNC_EN));
				write_indexed_register(card,IIO_IRQ_CONTROL,
						       control);


				if (card->DMAABusy || card->DMABBusy){

#ifdef USE_BH
					bh_card = card;
					mark_bh(MARGI_BH);
#else 
					do_margi(card);
#endif
					if(card->use_ringA || card->use_ringB){
					  L64021Intr(card);
					}
				} 
			}

			if (stat & DEC_INT) {
				write_indexed_register(card,IIO_IRQ_CONTROL,
						       control & (~DEC_EN));
				write_indexed_register(card,IIO_IRQ_CONTROL,
						       control);
				
				if(card->use_ringA || card->use_ringB){
					L64021Intr(card);
				}
			}

			stat = read_indexed_register(card, IIO_IRQ_STATUS);
		}
	}

}


void Timerfunction(unsigned long data)
{
	struct cvdv_cards *card = (struct cvdv_cards *) data;

	L64014Intr_function(card);

	card->timer.function = Timerfunction;
	card->timer.data=(unsigned long) card;
	card->timer.expires=jiffies+10;
	if ( card->open)
		add_timer(&card->timer);

}

void L64014Intr(int irq, void *dev_id, struct pt_regs *regs)
{
	margi_info_t *margi = dev_id;
	struct cvdv_cards *card = &(margi->card);
	u_char dio_index, lsi_index_low, lsi_index_high;

	spin_lock(&card->timelock);

	//save registers
	dio_index = inb(margi->link.io.BasePort1 + DIO_CONTROL_INDEX);
	lsi_index_low = inb(margi->link.io.BasePort1 + DIO_LSI_INDEX_LOW);
	lsi_index_high = inb(margi->link.io.BasePort1 + DIO_LSI_INDEX_HIGH);
	

	L64014Intr_function(card);

	//load registers
	outb(dio_index, margi->link.io.BasePort1 + DIO_CONTROL_INDEX);
	outb(lsi_index_low, margi->link.io.BasePort1 + DIO_LSI_INDEX_LOW);
	outb(lsi_index_high,margi->link.io.BasePort1 + DIO_LSI_INDEX_HIGH);
	spin_unlock(&card->timelock);
}

int L64014RemoveIntr(struct cvdv_cards *card)
{
	MDEBUG(1, ": -- L64014RemoveIntr\n");
	// Disable the IRQ's
	write_indexed_register(card, IIO_IRQ_CONTROL, 0x00);
	if (!card->IntInstalled)
		return 1;
	L64021RemoveIntr(card);
	return 0;
}

void l64020Reset(struct cvdv_cards *card){
	uint8_t data;
	
	
	data = read_indexed_register(card, IIO_LSI_CONTROL);
	data &= ~(RR | DR);
	write_indexed_register(card, IIO_LSI_CONTROL, data);
	mdelay(100);
	data = read_indexed_register(card, IIO_LSI_CONTROL);
	data |= DR;
	write_indexed_register(card, IIO_LSI_CONTROL, data);

	data = read_indexed_register(card,IIO_GPIO_PINS);
	data &= ~0x01;
	write_indexed_register(card,IIO_GPIO_PINS,data);
	data |= 0x01;
	write_indexed_register(card,IIO_GPIO_PINS,data);
	
	//write_indexed_register(card, IIO_LSI_CONTROL, DR);
	
	data = read_indexed_register(card, IIO_LSI_CONTROL);
	data &= ~DSVC;
	write_indexed_register(card, IIO_LSI_CONTROL, data);

}

void ZV_init(struct cvdv_cards *card)
{
	uint32_t delay, activel;
	uint8_t reg;
	delay = 235;
	activel = delay + 1448;
	
	// init delay and active lines
	write_indexed_register(card, IIO_VIDEO_HOR_DELAY, 
			       (uint8_t)(delay & 0x00FF));
	write_indexed_register(card, IIO_VIDEO_HOR_ACTIVE, 
			       (uint8_t)(activel & 0x00FF));      
	reg = ((uint8_t)((activel >> 4) & 0x0070))|((uint8_t)((delay >> 8) & 0x0007));
	write_indexed_register(card, IIO_VIDEO_HOR_HIGH, reg);

	//init video
	reg = read_indexed_register(card, IIO_VIDEO_CONTROL0);
	reg |= (ZVCLK13 | ZV16BIT | ZVCLKINV);
	write_indexed_register(card, IIO_VIDEO_CONTROL0, reg);
	reg = read_indexed_register(card, IIO_VIDEO_CONTROL1);
	reg |= (ZV_OVERRIDE | ZV_ENABLE);
	write_indexed_register(card, IIO_VIDEO_CONTROL1, reg);
}

void set_svhs(struct cvdv_cards *card, int onoff)
{
	uint8_t val;

	val =  I2CRead(card, card->i2c_addr, CS_DAC)&0x0f;
	MDEBUG(1, ": --svhs val 0x%02x\n",val);

	if (onoff){
		if (!card->svhs){
			I2CWrite(card, card->i2c_addr, CS_DAC, val|0x03);
			card->svhs = 1;
		}
	} else {
		if (!card->svhs){
			I2CWrite(card, card->i2c_addr, CS_DAC, val|0x30);
			card->svhs = 1;
		}
	}

}

void set_composite(struct cvdv_cards *card, int onoff)
{
	uint8_t val;

	val =  I2CRead(card, card->i2c_addr, CS_DAC)&0x0f;
	MDEBUG(1, ": --composite val 0x%02x\n",val);
	

	if (onoff){
		if (!card->composite){
			I2CWrite(card, card->i2c_addr, CS_DAC, val|0x84);
			card->composite = 1;
		}
	} else {
		if (!card->svhs){
			I2CWrite(card, card->i2c_addr, CS_DAC, val|0xE0);
			card->composite = 1;
		}
	}

}


int L64014Init(struct cvdv_cards *card)
{
	uint16_t testram[16];
	int i, err;

	MDEBUG(1, ": -- L64014Init\n");
	card->videomode = VIDEO_MODE;

	/* Reset 64020 */
	write_indexed_register(card, IIO_GPIO_CONTROL, 0x01);
	l64020Reset(card);
	/* init GPIO */
	write_indexed_register(card, IIO_GPIO_CONTROL, 0x01);
	write_indexed_register(card, IIO_GPIO_PINS, 0xff);

	/* Set to PAL */
	write_indexed_register(card, IIO_VIDEO_CONTROL0, 0);
	write_indexed_register(card, IIO_VIDEO_CONTROL1, VMS_PAL);

	/* Set Audio freq */
	write_indexed_register(card, IIO_OSC_AUD, 0x12);

	write_indexed_register(card, CSS_COMMAND, 0x01);


	MDEBUG(0, "CSID: %02x\n", I2CRead(card, 0, 0x3d));
	card->i2c_addr = I2CRead(card, 0, 0x0f);
	MDEBUG(0, "I2CADDR: %02x\n", card->i2c_addr);

	I2CWrite(card, card->i2c_addr, CS_CONTROL0, 0x4a);
	I2CWrite(card, card->i2c_addr, CS_CONTROL1, 0x04);
	I2CWrite(card, card->i2c_addr, CS_SC_AMP, 0x15);
	I2CWrite(card, card->i2c_addr, CS_SC_SYNTH0, 0x96);
	I2CWrite(card, card->i2c_addr, CS_SC_SYNTH1, 0x15);
	I2CWrite(card, card->i2c_addr, CS_SC_SYNTH2, 0x13);
	I2CWrite(card, card->i2c_addr, CS_SC_SYNTH3, 0x54);

//	I2CWrite(card, card->i2c_addr, CS_DAC, 0x87);
	if (svhs) set_svhs(card, 1);
	if (composite) set_composite(card, 1);
	I2CWrite(card, card->i2c_addr, CS_BKG_COL, 0x03);

	MDEBUG(0,"Decoder Status: %d\n", read_lsi_status(card));
	MDEBUG(0,"lsi stat %d\n", DecoderReadByte(card, 0x005));

	if (use_zv) ZV_init(card);
	L64021Init(card);

	// Find out how much DRAM we have
	card->DRAMSize = 0x00100000;	// maximum size
	do {
		MDEBUG(0,
		       ": Probing DRAM Size: 0x%08X (%d kByte) ... ",
		       card->DRAMSize, card->DRAMSize / 512);
		for (i = 0; i < 8; i++)
			testram[i] = rnd(0x100) | (rnd(0x100) << 8);
		if (DRAMWriteWord(card, 0, 4, &testram[0], 0))
			MDEBUG(0, ": DRAM Write error.\n");
		if (DRAMWriteWord
		    (card, card->DRAMSize - 4, 4, &testram[4],
		     0)) MDEBUG(0,
				": DRAM Write error.\n");
		if (DRAMReadWord(card, 0, 4, &testram[8], 0))
			MDEBUG(0, ": DRAM Read error.\n");
		if (DRAMReadWord
		    (card, card->DRAMSize - 4, 4, &testram[12],
		     0)) MDEBUG(0, ": DRAM Read error.\n");
		err = 0;
		for (i = 0; (!err) && (i < 8); i++)
			if (testram[i] != testram[i + 8])
				err = i + 1;
		if (err) {
			MDEBUG(0," failed\n");
		} else {
			MDEBUG(0," ok\n");
		}
		if (err)
			MDEBUG(2,": DRAM compare error at cell %d: 0x%04X %04X %04X %04X->0x%04X %04X %04X %04X / 0x%04X %04X %04X %04X->0x%04X %04X %04X %04X\n",
			       err, testram[0], testram[1], testram[2],
			       testram[3], testram[8], testram[9],
			       testram[10], testram[11], testram[4],
			       testram[5], testram[6], testram[7],
			       testram[12], testram[13], testram[14],
			       testram[15]);
		if (err)
			card->DRAMSize >>= 1;
	} while (err && (card->DRAMSize >= 0x00100000));
	printk(KERN_INFO LOGNAME ": DRAM Size: 0x%08X (%d kByte)\n",
	       card->DRAMSize, card->DRAMSize / 512);
	if (card->DRAMSize < 0x00100000) {	// minimum size
		printk(KERN_INFO LOGNAME
		       ": DRAM ERROR: Not enough memory on card!\n");
		return 1;
	}
	return 0;
}


void CardDeInit(struct cvdv_cards *card)
{
	CloseCard(card);
	MargiFlush(card);
	MargiFreeBuffers(card);

	L64014RemoveIntr(card);
	card_init(card);
}


static u_char read_lsi_status(struct cvdv_cards *card)
{
	margi_info_t *margi = (margi_info_t *) card->margi;
	return (inb(margi->link.io.BasePort1 + DIO_LSI_STATUS) & 15);

}

/*====================================================================*/

static void cs_error(client_handle_t handle, int func, int ret)
{
	error_info_t err = { func, ret };
	CardServices(ReportError, handle, &err);
}

/*======================================================================

    margi_attach() creates an "instance" of the driver, allocating
    local data structures for one device.  The device is registered
    with Card Services.

    The dev_link structure is initialized, but we don't actually
    configure the card at this point -- we wait until we receive a
    card insertion event.
    
======================================================================*/

static dev_link_t *margi_attach(void)
{
	margi_info_t *local;
	dev_link_t *link;
	client_reg_t client_reg;
	int ret, i;

	MDEBUG(0, "margi_attach()\n");

	for (i = 0; i < MAX_DEV; i++)
		if (dev_table[i] == NULL)
			break;
	if (i == MAX_DEV) {
		printk(KERN_NOTICE "margi_cs: no devices available\n");
		return NULL;
	}

	/* Allocate space for private device-specific data */
	local = kmalloc(sizeof(margi_info_t), GFP_KERNEL);
	if (!local)
		return NULL;
	memset(local, 0, sizeof(margi_info_t));
	link = &local->link;
	link->priv = local;
	local->card.margi = (void *) local;
	dev_table[i] = link;

	/* Initialize the dev_link_t structure */
	link->release.function = &margi_release;
	link->release.data = (u_long) link;

	/* Interrupt setup */
	link->irq.Attributes = IRQ_TYPE_EXCLUSIVE;
	link->irq.IRQInfo1 = IRQ_INFO2_VALID | IRQ_LEVEL_ID;
	if (irq_list[0] == -1)
		link->irq.IRQInfo2 = irq_mask;
	else
		for (i = 0; i < 4; i++)
			link->irq.IRQInfo2 |= 1 << irq_list[i];
	link->irq.Handler = NULL;

	/*
	   General socket configuration defaults can go here.  In this
	   client, we assume very little, and rely on the CIS for almost
	   everything.  In most clients, many details (i.e., number, sizes,
	   and attributes of IO windows) are fixed by the nature of the
	   device, and can be hard-wired here.
	 */
	link->conf.Attributes = 0;
	link->conf.Vcc = 50;
	
	if (!use_zv)
		link->conf.IntType = INT_MEMORY_AND_IO;
	else
		link->conf.IntType = INT_ZOOMED_VIDEO;

	/* Register with Card Services */
	link->next = dev_list;
	dev_list = link;
	client_reg.dev_info = &dev_info;
	client_reg.Attributes = INFO_IO_CLIENT | INFO_CARD_SHARE;
	client_reg.EventMask =
	    CS_EVENT_CARD_INSERTION | CS_EVENT_CARD_REMOVAL |
	    CS_EVENT_RESET_PHYSICAL | CS_EVENT_CARD_RESET |
	    CS_EVENT_PM_SUSPEND | CS_EVENT_PM_RESUME;
	client_reg.event_handler = &margi_event;
	client_reg.Version = 0x0210;
	client_reg.event_callback_args.client_data = link;
	ret = CardServices(RegisterClient, &link->handle, &client_reg);
	if (ret != CS_SUCCESS) {
		cs_error(link->handle, RegisterClient, ret);
		margi_detach(link);
		return NULL;
	}

	return link;
}				/* margi_attach */

/*======================================================================

    This deletes a driver "instance".  The device is de-registered
    with Card Services.  If it has been released, all local data
    structures are freed.  Otherwise, the structures will be freed
    when the device is released.

======================================================================*/

static void margi_detach(dev_link_t * link)
{
	dev_link_t **linkp;

	int nd;

	MDEBUG(0, "margi_detach(0x%p)\n", link);

	for (nd = 0; nd < MAX_DEV; nd++)
		if (dev_table[nd] == link)
			break;
	if (nd == MAX_DEV)
		return;

	/* Locate device structure */
	for (linkp = &dev_list; *linkp; linkp = &(*linkp)->next)
		if (*linkp == link)
			break;
	if (*linkp == NULL)
		return;

	/*
	   If the device is currently configured and active, we won't
	   actually delete it yet.  Instead, it is marked so that when
	   the release() function is called, that will trigger a proper
	   detach().
	 */
	if (link->state & DEV_CONFIG) {
		MDEBUG(2, "margi_cs: detach postponed, '%s' "
		       "still locked\n", link->dev->dev_name);
		link->state |= DEV_STALE_LINK;
		return;
	}

	/* Break the link with Card Services */
	if (link->handle)
		CardServices(DeregisterClient, link->handle);

	/* Unlink device structure, and free it */
	*linkp = link->next;
	/* This points to the parent struct cvdv_cards struct */
	dev_table[nd] = NULL;

	kfree(link->priv);

}				/* margi_detach */

/*======================================================================

    margi_config() is scheduled to run after a CARD_INSERTION event
    is received, to configure the PCMCIA socket, and to make the
    device available to the system.
    
======================================================================*/

#define CS_CHECK(fn, args...) \
while ((last_ret=CardServices(last_fn=(fn),args))!=0) goto cs_failed

#define CFG_CHECK(fn, args...) \
if (CardServices(fn, args) != 0) goto next_entry

static void margi_init(struct cvdv_cards *card)
{
	if (0xdd == read_indexed_register(card, IIO_ID)) {
		printk("L64014 Version %d in mode %d detected\n",
		       (read_indexed_register(card, IIO_MODE) & 248) >> 3,
		       read_indexed_register(card, IIO_MODE) & 7);
		write_indexed_register(card, IIO_GPIO_CONTROL, 0x07);
		
		L64014Init(card);
		
		// default: color bars
		VideoSetBackground(card, 1, 0, 0, 0);	// black
		SetVideoSystem(card);

		/*enable L64014 IRQ */
		write_indexed_register(card, IIO_IRQ_CONTROL,
				       IRQ_POL | IRQ_EN | VSYNC_EN);
//		write_indexed_register(card, IIO_IRQ_CONTROL, 0x24);

		OSDOpen(card, 50, 50, 150, 150, 2, 1);
		OSDTest(card);
	}

}

static void margi_config(dev_link_t * link)
{
	client_handle_t handle = link->handle;
	margi_info_t *dev = link->priv;
	struct cvdv_cards *card = &(dev->card);
	tuple_t tuple;
	cisparse_t parse;
	int last_fn, last_ret;
	u_char buf[64];
	config_info_t conf;
	win_req_t req;
	memreq_t map;

	MDEBUG(0, "margi_config(0x%p)\n", link);

	/*
	   This reads the card's CONFIG tuple to find its configuration
	   registers.
	 */
	tuple.DesiredTuple = CISTPL_CONFIG;
	tuple.Attributes = 0;
	tuple.TupleData = buf;
	tuple.TupleDataMax = sizeof(buf);
	tuple.TupleOffset = 0;
	CS_CHECK(GetFirstTuple, handle, &tuple);
	CS_CHECK(GetTupleData, handle, &tuple);
	CS_CHECK(ParseTuple, handle, &tuple, &parse);
	link->conf.ConfigBase = parse.config.base;
	link->conf.Present = parse.config.rmask[0];

	/* Configure card */
	link->state |= DEV_CONFIG;

	/* Look up the current Vcc */
	CS_CHECK(GetConfigurationInfo, handle, &conf);
	link->conf.Vcc = conf.Vcc;

	/*
	   In this loop, we scan the CIS for configuration table entries,
	   each of which describes a valid card configuration, including
	   voltage, IO window, memory window, and interrupt settings.

	   We make no assumptions about the card to be configured: we use
	   just the information available in the CIS.  In an ideal world,
	   this would work for any PCMCIA card, but it requires a complete
	   and accurate CIS.  In practice, a driver usually "knows" most of
	   these things without consulting the CIS, and most client drivers
	   will only use the CIS to fill in implementation-defined details.
	 */
	tuple.DesiredTuple = CISTPL_CFTABLE_ENTRY;
	CS_CHECK(GetFirstTuple, handle, &tuple);
	while (1) {
		cistpl_cftable_entry_t dflt = { 0 };
		cistpl_cftable_entry_t *cfg = &(parse.cftable_entry);
		CFG_CHECK(GetTupleData, handle, &tuple);
		CFG_CHECK(ParseTuple, handle, &tuple, &parse);

		if (cfg->flags & CISTPL_CFTABLE_DEFAULT)
			dflt = *cfg;
		if (cfg->index == 0)
			goto next_entry;
		link->conf.ConfigIndex = cfg->index;

		/* Does this card need audio output? */
		if (cfg->flags & CISTPL_CFTABLE_AUDIO) {
			link->conf.Attributes |= CONF_ENABLE_SPKR;
			link->conf.Status = CCSR_AUDIO_ENA;
		}

		/* Use power settings for Vcc and Vpp if present */
		/*  Note that the CIS values need to be rescaled */
		if (cfg->vcc.present & (1 << CISTPL_POWER_VNOM)) {
			if (conf.Vcc !=
			    cfg->vcc.param[CISTPL_POWER_VNOM] /
			    10000) goto next_entry;
		} else if (dflt.vcc.present & (1 << CISTPL_POWER_VNOM)) {
			if (conf.Vcc !=
			    dflt.vcc.param[CISTPL_POWER_VNOM] /
			    10000) goto next_entry;
		}

		if (cfg->vpp1.present & (1 << CISTPL_POWER_VNOM))
			link->conf.Vpp1 = link->conf.Vpp2 =
			    cfg->vpp1.param[CISTPL_POWER_VNOM] / 10000;
		else if (dflt.vpp1.present & (1 << CISTPL_POWER_VNOM))
			link->conf.Vpp1 = link->conf.Vpp2 =
			    dflt.vpp1.param[CISTPL_POWER_VNOM] / 10000;

		/*
		   Allocate an interrupt line.  Note that this does not assign a
		   handler to the interrupt, unless the 'Handler' member of the
		   irq structure is initialized.
		 */

		/* IO window settings */
		link->io.NumPorts1 = link->io.NumPorts2 = 0;
		if ((cfg->io.nwin > 0) || (dflt.io.nwin > 0)) {
			cistpl_io_t *io =
			    (cfg->io.nwin) ? &cfg->io : &dflt.io;
			link->io.Attributes1 = IO_DATA_PATH_WIDTH_AUTO;
			if (!(io->flags & CISTPL_IO_8BIT))
				link->io.Attributes1 =
				    IO_DATA_PATH_WIDTH_16;
			if (!(io->flags & CISTPL_IO_16BIT))
				link->io.Attributes1 =
				    IO_DATA_PATH_WIDTH_8;
			link->io.IOAddrLines =
			    io->flags & CISTPL_IO_LINES_MASK;
			link->io.BasePort1 = io->win[0].base;
			link->io.NumPorts1 = io->win[0].len;
			if (io->nwin > 1) {
				link->io.Attributes2 =
				    link->io.Attributes1;
				link->io.BasePort2 = io->win[1].base;
				link->io.NumPorts2 = io->win[1].len;
			}
		}

		/* This reserves IO space but doesn't actually enable it */
		CFG_CHECK(RequestIO, link->handle, &link->io);

		/*
		   Now set up a common memory window, if needed.  There is room
		   in the dev_link_t structure for one memory window handle,
		   but if the base addresses need to be saved, or if multiple
		   windows are needed, the info should go in the private data
		   structure for this device.

		   Note that the memory window base is a physical address, and
		   needs to be mapped to virtual space with ioremap() before it
		   is used.
		 */
		if ((cfg->mem.nwin > 0) || (dflt.mem.nwin > 0)) {
			cistpl_mem_t *mem =
			    (cfg->mem.nwin) ? &cfg->mem : &dflt.mem;
			req.Attributes =
			    WIN_DATA_WIDTH_16 | WIN_MEMORY_TYPE_CM;
			req.Attributes |= WIN_ENABLE;
			req.Base = mem->win[0].host_addr;
			req.Size = mem->win[0].len;
			req.AccessSpeed = 0;
			link->win = (window_handle_t) link->handle;
			CFG_CHECK(RequestWindow, &link->win, &req);
			map.Page = 0;
			map.CardOffset = mem->win[0].card_addr;
			CFG_CHECK(MapMemPage, link->win, &map);
		}
		/* If we got this far, we're cool! */
		break;
		
	next_entry:
		CS_CHECK(GetNextTuple, handle, &tuple);
	}

	/*
	   This actually configures the PCMCIA socket -- setting up
	   the I/O windows and the interrupt mapping, and putting the
	   card and host interface into "Memory and IO" mode.
	 */
	CS_CHECK(RequestConfiguration, link->handle, &link->conf);

	/*
	   We can release the IO port allocations here, if some other
	   driver for the card is going to loaded, and will expect the
	   ports to be available.
	 */
	if (free_ports) {
		if (link->io.BasePort1)
			release_region(link->io.BasePort1,
				       link->io.NumPorts1);
		if (link->io.BasePort2)
			release_region(link->io.BasePort2,
				       link->io.NumPorts2);
	}

	/*
	   At this point, the dev_node_t structure(s) need to be
	   initialized and arranged in a linked list at link->dev.
	 */

	first_card = card;
	card->next = NULL;
	card_init(card);

	sprintf(dev->node.dev_name, "margi");
	dev->node.major = 250;
	dev->node.minor = 0;
	link->dev = &dev->node;

	dvb_register(card);

	/* Finally, report what we've done */
	printk(KERN_INFO "%s: index 0x%02x: Vcc %d.%d",
	       dev->node.dev_name, link->conf.ConfigIndex,
	       link->conf.Vcc / 10, link->conf.Vcc % 10);
	if (link->conf.Vpp1)
		printk(", Vpp %d.%d", link->conf.Vpp1 / 10,
		       link->conf.Vpp1 % 10);
	if (link->conf.Attributes & CONF_ENABLE_IRQ)
		printk(", irq %d", link->irq.AssignedIRQ);
	if (link->io.NumPorts1)
		printk(", io 0x%04x-0x%04x", link->io.BasePort1,
		       link->io.BasePort1 + link->io.NumPorts1 - 1);
	if (link->io.NumPorts2)
		printk(" & 0x%04x-0x%04x", link->io.BasePort2,
		       link->io.BasePort2 + link->io.NumPorts2 - 1);
	if (link->win)
		printk(", mem 0x%06lx-0x%06lx", req.Base,
		       req.Base + req.Size - 1);
	printk("\n");

	link->state &= ~DEV_CONFIG_PENDING;
	margi_init(card);
	return;

      cs_failed:
	cs_error(link->handle, last_fn, last_ret);
	margi_release((u_long) link);

}				/* margi_config */

/*======================================================================

    After a card is removed, margi_release() will unregister the
    device, and release the PCMCIA configuration.  If the device is
    still open, this will be postponed until it is closed.
    
======================================================================*/

static void margi_release(u_long arg)
{
	dev_link_t *link = (dev_link_t *) arg;
	margi_info_t *dev = link->priv;
	struct cvdv_cards *card = &(dev->card);

	MDEBUG(0, "margi_release(0x%p)\n", link);
	/*
	   If the device is currently in use, we won't release until it
	   is actually closed, because until then, we can't be sure that
	   no one will try to access the device or its data structures.
	 */
	if (link->open) {
		MDEBUG(1, "margi_cs: release postponed, '%s' still open\n",
		      link->dev->dev_name);
		link->state |= DEV_STALE_CONFIG;
		return;
	}

	/* Unlink the device chain */
	link->dev = NULL;

	/*
	   In a normal driver, additional code may be needed to release
	   other kernel data structures associated with this device. 
	 */

	MDEBUG(1,": Unloading device driver\n");
	CardDeInit(card);

	CloseCard(card);
	dvb_unregister(card);
	/* Don't bother checking to see if these succeed or not */
	if (link->win)
	  CardServices(ReleaseWindow, link->win);
	CardServices(ReleaseConfiguration, link->handle);
	if (link->io.NumPorts1)
	  CardServices(ReleaseIO, link->handle, &link->io);

	link->state &= ~DEV_CONFIG;

	if (link->state & DEV_STALE_LINK)
		margi_detach(link);

}				/* margi_release */

/*======================================================================

    The card status event handler.  Mostly, this schedules other
    stuff to run after an event is received.

    When a CARD_REMOVAL event is received, we immediately set a
    private flag to block future accesses to this device.  All the
    functions that actually access the device should check this flag
    to make sure the card is still present.
    
======================================================================*/

static int margi_event(event_t event, int priority,
		       event_callback_args_t * args)
{
	dev_link_t *link = args->client_data;
	margi_info_t *dev = link->priv;

	MDEBUG(1, "margi_event(0x%06x)\n", event);

	switch (event) {
	case CS_EVENT_CARD_REMOVAL:
		link->state &= ~DEV_PRESENT;
		if (link->state & DEV_CONFIG) {
			((margi_info_t *) link->priv)->stop = 1;
			link->release.expires = jiffies + HZ / 20;
			add_timer(&link->release);
		}
		break;
	case CS_EVENT_CARD_INSERTION:
		link->state |= DEV_PRESENT | DEV_CONFIG_PENDING;
		dev->card.bus = args->bus;
		margi_config(link);
		break;
	case CS_EVENT_PM_SUSPEND:
		link->state |= DEV_SUSPEND;
		/* Fall through... */
	case CS_EVENT_RESET_PHYSICAL:
		/* Mark the device as stopped, to block IO until later */
		dev->stop = 1;
		if (link->state & DEV_CONFIG)
			CardServices(ReleaseConfiguration, link->handle);
		break;
	case CS_EVENT_PM_RESUME:
		link->state &= ~DEV_SUSPEND;
		/* Fall through... */
	case CS_EVENT_CARD_RESET:
		if (link->state & DEV_CONFIG)
			CardServices(RequestConfiguration, link->handle,
				     &link->conf);
		margi_init(&dev->card);
		dev->stop = 0;
		/*
		   In a normal driver, additional code may go here to restore
		   the device state and restart IO. 
		 */
		break;
	}
	return 0;
}				/* margi_event */

/*====================================================================*/

static int __init init_margi_cs(void)
{
	servinfo_t serv;
	MDEBUG(0, "%s\n", version);
	CardServices(GetCardServicesInfo, &serv);
	if (serv.Revision != CS_RELEASE_CODE) {
		printk(KERN_NOTICE "margi_cs: Card Services release "
		       "does not match!\n");
		return -1;
	}
	register_pccard_driver(&dev_info, &margi_attach, &margi_detach);
	return 0;
}

static void __exit exit_margi_cs(void)
{
	MDEBUG(0, "margi_cs: unloading\n");
	unregister_pccard_driver(&dev_info);
	while (dev_list != NULL) {
		if (dev_list->state & DEV_CONFIG)
			margi_release((u_long) dev_list);
		margi_detach(dev_list);
	}
}

module_init(init_margi_cs);
module_exit(exit_margi_cs);
