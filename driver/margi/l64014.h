/* 
    l64014.h

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

#ifndef L64014_h
#define L64014_h

#include <linux/kernel.h>
#include <linux/config.h>
#include <linux/pci.h>


#define DIO_CONTROL_INDEX          0x00
#define DIO_CONTROL_DATA           0x02
#define DIO_LSI_STATUS             0x04
#define DIO_LSI_CHANNEL_DATA       0x04
#define DIO_LSI_INDEX_LOW          0x08
#define DIO_LSI_DATA               0x0A
#define DIO_LSI_INDEX_HIGH         0x0C

#define LSI_READY                  0x08
#define LSI_WAIT                   0x04
#define LSI_ARQ                    0x02
#define LSI_VRQ                    0x01

#define IIO_ID                     0x00
#define IIO_MODE                   0x01
#define IIO_IRQ_CONTROL            0x02
#define IIO_IRQ_STATUS             0x03
#define IIO_LSI_CONTROL            0x06
#define IIO_OSC_AUD                0x08
#define IIO_VIDEO_CONTROL0         0x09
#define IIO_VIDEO_CONTROL1         0x0A
#define IIO_VIDEO_LOOKUP           0x0B
#define IIO_EEPROM_CONTROL         0x0C
#define IIO_VIDEO_HOR_DELAY        0x0D
#define IIO_VIDEO_HOR_ACTIVE       0x0E
#define IIO_VIDEO_HOR_HIGH         0x0F
#define IIO_GPIO_CONTROL           0x10
#define IIO_GPIO_PINS              0x11
#define IIO_CSS_COMMAND            0x12
#define IIO_CSS_STATUS             0x13
#define IIO_CSS_KEY                0x14

#define SCL                        0x02
#define SDA                        0x04

#define CS_CONTROL0                0x00
#define CS_CONTROL1                0x01
#define CS_CONTROL2                0x02
#define CS_DAC                     0x04
#define CS_STATUS                  0x07
#define CS_BKG_COL                 0x08
#define CS_GPIO_CTRL               0x09
#define CS_GPIO_DATA               0x0A
#define CS_C_AMP                   0x0D
#define CS_Y_AMP                   0x0E
#define CS_I2C_ADR                 0x0F
#define CS_SC_AMP                  0x10
#define CS_SC_SYNTH0               0x11
#define CS_SC_SYNTH1               0x12
#define CS_SC_SYNTH2               0x13
#define CS_SC_SYNTH3               0x14
#define CS_HUE_LSB                 0x15
#define CS_HUE_MSB                 0x16
#define CS_CC_EN                   0x18
#define CS_CC_21_1                 0x19
#define CS_CC_21_2                 0x1A
#define CS_CC_284_1                0x1B
#define CS_CC_284_2                0x1C
#define CS_INT_EN                  0x3B
#define CS_INT_CLR                 0x3C
#define CS_ID_REG                  0x3D


#define   CSS_COMMAND             0x12
#define   CSS_STATUS              0x13
#define   CSS_KEY                 0x14

#define   L14_CSS_NONE            0x00
#define   L14_CSS_PASSTHRU        0x01
#define   L14_CSS_DESCRAM         0x05
#define   L14_CSS_GEN_CH          0x08
#define   L14_CSS_RD_CH           0x09
#define   L14_CSS_WR_CH           0x0a
#define   L14_CSS_WR_DRVREF       0x0b
#define   L14_CSS_DRVAUTH         0x0c
#define   L14_CSS_DECAUTH         0x0d
#define   L14_CSS_DISCKEY         0x0e
#define   L14_CSS_TITLEKEY        0x0f
#define   L14_CSS_CMD_START       0x10

#define   L14_CSS_BUSY            0x01
#define   L14_CSS_SUCCESS         0x02

#define DSVC                       0x40
#define RR                         0x20
#define DR                         0x01
#define AF1                        0x20
#define AF0                        0x10
#define SLEEP                      0x08
#define AFS2                       0x04
#define AFS1                       0x02
#define AFS0                       0x01

#define   ZVCLK13             0x04
#define   ZVCLKINV            0x08
#define   ZV16BIT             0x10
#define   ZVVREF_INVERT       0x08
#define   ZVHREF_INVERT       0x10
#define   HSYNC_INVERT        0x20
#define   ZV_OVERRIDE         0x40
#define   ZV_ENABLE           0x80


#define IRQ_EN                     0x04
#define IRQ_MSK                    0x08
#define IRQ_POL                    0x10
#define DEC_EN                     0x20
#define DEC_INT                    0x10
#define VSYNC_EN                   0x80
#define VSYNC_INT                  0x40

#define VMS_NOSY                   0x00
#define VMS_NTSC                   0x01
#define VMS_PAL                    0x02
#define VMS_PAL24                  0x03

#define MAUDIO_PAUSE 0
#define MAUDIO_PLAY 1
#define MAUDIO_FAST 2
#define MAUDIO_SLOW 3


#define RegisterReadByte(card,where) read_indexed_register(&(card->link),(where))
#define RegisterWriteByte(card,where,what) write_indexed_register(&(card->link),where,what)
#define RegisterMaskByte(card,where,mask,bits) RegisterWriteByte(card,where,(RegisterReadByte(card,where)&~(mask))|(bits))
#define RegisterSetByte(card,where,bits) RegisterWriteByte(card,where,RegisterReadByte(card,where)|(bits))
#define RegisterDelByte(card,where,mask) RegisterWriteByte(card,where,RegisterReadByte(card,where)&~(mask))

#define RegisterReadWord(card,where) (\
  (u16)RegisterReadByte(card,where)|\
  ((u16)RegisterReadByte(card,(where)+1)<<8))
#define RegisterWriteWord(card,where,what) {\
  RegisterWriteByte(card,where,(what) & 0xFF);\
  RegisterWriteByte(card,(where)+1,((what)>>8) & 0xFF);}

// 3-byte-wide (medium word, 24 Bit) access to the card's registers, LSB first
#define RegisterReadMWord(card,where) (\
  (u32)RegisterReadByte(card,where)|\
  ((u32)RegisterReadByte(card,(where)+1)<<8)|\
  ((u32)RegisterReadByte(card,(where)+2)<<16))
#define RegisterWriteMWord(card,where,what) {\
  RegisterWriteByte(card,where,(what) & 0xFF);\
  RegisterWriteByte(card,(where)+1,((what)>>8) & 0xFF);\
  RegisterWriteByte(card,(where)+2,((what)>>16) & 0xFF);}

// double-word-wide access to the card's registers, LSB first
//#define RegisterReadDWord(card,where) le32_to_cpu(readl(card->addr+(where)))
//#define RegisterWriteDWord(card,where,what) writel(cpu_to_le32(what),card->addr+(where))
#define RegisterReadDWord(card,where) (\
  (u32)RegisterReadByte(card,where)|\
  ((u32)RegisterReadByte(card,(where)+1)<<8)|\
  ((u32)RegisterReadByte(card,(where)+2)<<16)|\
  ((u32)RegisterReadByte(card,(where)+3)<<24))
#define RegisterWriteDWord(card,where,what) {\
  RegisterWriteByte(card,where,(what) & 0xFF);\
  RegisterWriteByte(card,(where)+1,((what)>>8) & 0xFF);\
  RegisterWriteByte(card,(where)+2,((what)>>16) & 0xFF);\
  RegisterWriteByte(card,(where)+3,((what)>>24) & 0xFF);}

#endif
