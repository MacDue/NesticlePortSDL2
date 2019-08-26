//emulation of NES registers....

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
// #include <process.h>

#include "types.h"

#include "message.h"
#include "file.h"

#include "keyb.h"

#define FREE(x) if (x) {free(x); x=0;}
#define DELETE(x) if (x) {delete x; x=0;}

#include "m6502.h"
#include "timing.h"

#include "nesvideo.h"

#include "nes.h"

#include "input.h"
#include "dd.h"

extern struct m6502MemoryWrite NESwrite[];
extern struct m6502MemoryRead NESread[];
extern int blah,blah2;
//---------------------------------
//hardware internal registers
word ppumemaddr; //address for next write in ppu memory
char ppureadfuckup;  //first mem fuck up

byte spritememaddr; //next write in sprite memory

byte scrollx;
byte scrolly;
int scrollscanline;

volatile byte vblank; //are we in a vblank ?
volatile byte hitflag; //are we in hitflag?
volatile byte frame; //are we scanning frame?

int nttoshow; //name table to show....

byte mirroring; //ppu name table mirroring
#define HMIRROR 0
#define VMIRROR 1

byte VROMswitch;
byte KONAMIswitch;

//-------------------------
//reset nes
int resetNEShardware()
{
 if (!NESinitialized) return -1;
 scrollx=scrolly=0;
 ppumemaddr=0; ppureadfuckup=0;
 spritememaddr=0;
 nttoshow=0;

 //erase memory
 ppu->clear();
 memset(ram,0,0x10000);
 memset(spritemem,0,256);

 //set default mirroring
 mirroring=romhdr.mirroring;

 if (!numrom) {msg.error("No rom data!"); return -1;}

 //copy last page of rom to end of cpu address space
 memcpy(ram+0xC000,ROM[numrom-1],sizeof(ROMPAGE));

 //copy first page of rom to 0x8000
 if (numrom>=2)  memcpy(ram+0x8000,ROM,sizeof(ROMPAGE));

 //copy vrom to ppu address space (if it exists)
 if (numvrom>0) memcpy(ppu,VROM,sizeof(VROMPAGE));

 //set up 6502 CPU emulator
 m6502Base=(uchar *)ram; //set base of emu
 m6502reset();   //reset cpu

 m6502MemoryRead=NESread; //set r/w trap handlers
 m6502MemoryWrite=NESwrite;

 //MMC type
 switch (romhdr.banktype)
 {
  case MMC_NONE: //no mapping chips
   break;

  case MMC_SEQ: //sequential mapper
   msg.printf(2,"Sequential mapper initialized");
   break;

  case MMC_VROMSWITCH:
   msg.printf(2,"VROM switch initialized");
   VROMswitch=0;
   break;
  case MMC_KONAMI:
   KONAMIswitch=0;
   msg.printf(2,"Konami mapper initialized");
   break;

  default:
   msg.error("Unsupported mapping type");
//   return -1;
 }

 //reset emulated video
 nv->resetpalette();
 nv->resetpatterncache();

 CPUpaused=0;
 msg.printf(1,"NES hardware reset");
 return 0;
}


//--------------------------------
//PPU emulation

NES_ppumemory::NES_ppumemory()
{
 clear();
 msg.printf(2,"%dK PPU address space created",sizeof(*this)/1024);
}

void NES_ppumemory::clear()
{
 memset(this,0,sizeof(*this));
}

void NES_ppumemory::write(word a,byte d)
{
//msg.printf(1,"ppu data write %X %X",ppumemaddr,d);

 //pallete write ?
 if (a>=0x3F00 && a<0x3F20)
  {
   if (((byte *)this)[a]==d) return; //no change
  // msg.printf(1,"pal[%X]=%X",a-0x3f00,d);

   if (!(a&0xF)) //it's a background palette...
    {
     for (int i=0; i<8; i++)
      {
       ((byte *)this)[0x3F00+i*4]=d;
        nv->palupdateidx[i*4]=1;
      }
     nv->paletteupdated=1;
    } else       //normal palette
    if (a&3)
    {
     ((byte *)this)[a]=d; //write it then...
     nv->palupdateidx[a-0x3F00]=1;
     nv->paletteupdated=1;
    }
   return;
  }

  //write byte to ppu memory
 ((byte *)this)[a]=d;

 //pattern table was written to ?
 if (a<0x2000)
 {
  nv->ptn[a/0x1000][(a&0xFFF)/sizeof(NES_pattern)].updated=1;
  nv->patternupdated=1;
//  blah2++;
//  msg.printf(1,"pattern %d.%d updated",a/0x1000,(a&0xFFF)/sizeof(NES_pattern));
  return;
 }

 //name/attribute table was written to..
 if (a<0x3000)
  { //we must mirror it
   if (mirroring==HMIRROR)
        ((byte *)this)[a^0x400]=d; //horizontal mirroring
   else ((byte *)this)[a^0x800]=d; //vertical mirroring
   blah++;
  }

}

byte NES_ppumemory::read(word a)
{
 //msg.printf(1,"ppu data read %X",ppumemaddr);
 return ((byte *)this)[a];
}



//---------------------------------------

void startframe()
{
 frame=1;     //during scanning of frame
 vblank=0;    //not in vblank
}

void startvblank()
{
 vblank=1;   //we are in vblank
 hitflag=0;  //reset hitflag
 frame=0;
 scrollscanline=0; //scanline of when scroll was read
}



//-------------------------------------------------------
//memory write trap handlers

//write to lower ports
int neswrite2000(WORD a,BYTE d)
{
  switch (a)
  {
   case 0x2000: //PPU control register 1
//   msg.printf(1,"write[%X]=%X",a,d);
      if (getscanline()>=scrollscanline) nttoshow=d&3;
      ram[a]=d;
      return 0;
   case 0x2001: //PPU control register 2
      ram[a]=d;
      return 0;
   case 0x2003: //sprite memory address
      spritememaddr=d;
//      msg.printf(1,"sprite mem addr set to %X",spritememaddr);
      return 0;
   case 0x2004: //sprite mem data write
//      msg.printf(1,"spritewrite %d",spritememaddr);
      ((byte *)spritemem)[spritememaddr++]=d;
      return 0;
   case 0x2005: //Background scroll
//     if (vblank) return 0;
     {
      int scanline=getscanline();
      if (scanline>=scrollscanline)
       {
        if (!ram[0x2005])  scrollx=d;
               else
          if (d<=239)
          {
           scrolly=d;
          // msg.printf(2,"line=%d %d,%d",scanline,scrollx,scrolly);
          }
        scrollscanline=scanline;
       }
     }
     ram[0x2005]^=1;
     return 0;

   case 0x2006: //PPU memory address
     ram[0x2006]^=1;
     ((byte *)&ppumemaddr)[ram[0x2006]]=d;
     ppureadfuckup=1;

//      if (!ram[0x2006]) msg.printf(1,"ppu mem addr set to %X",ppumemaddr);
      return 0;
   case 0x2007: //PPU data write
      ppu->write(ppumemaddr,d);
      ppumemaddr+=(ram[0x2000]&4) ? 32 : 1;
      return 0;
   default:
    msg.printf(1,"unsupported write: %X %X",a,d);
    return 0;
  }
};

//write to upper ports
int neswrite4000(WORD a,BYTE d)
{
// msg.error("upper port write!!");
 switch (a)
 {
  case 0x4014: //DMA sprite copy
//     msg.printf(3,"DMA sprite copy from %X",d*0x100);
     memcpy(spritemem,ram+(d*0x100),0x100); //copy memory
    return 0;
  case 0x4015: //sound switch
     ram[0x4015]=d;
    return 0;
  case 0x4016: //joystick
  case 0x4017: //joystick
     ram[0x4016]=1; //reset joystick
     ram[0x4017]=1; //reset joystick
    return 0;
  default:
//    msg.printf(1,"unsupported write: %X %X",a,d);
   return 0;
 };
}

//write to expansion module
int neswrite5000(WORD a,BYTE d)
{
 msg.error("expansion write!!");
 return 0;
}

int neswrite0000(WORD a,BYTE d)
{
// if (a>=0x800) msg.error("weird write");
 ram[a&0x7FF]=d;
 return 0;
}

//write to 0x8000-0xFFFF
int neswrite8000(WORD a,BYTE d)
{
 switch (romhdr.banktype)
 {
  case MMC_NONE: //no memory mapper...
    msg.error("ROM write!!");
   break;
  case MMC_SEQ:
    msg.error("Seq[%X]=%X",a,d);
   break;
  case MMC_VROMSWITCH:
    msg.error("VROM[%X]=%X",a,d);
    d&=3;
    if (d<numvrom) memcpy(ppu,&VROM[d],sizeof(VROMPAGE));
   break;
  case MMC_KONAMI:
   if (d!=KONAMIswitch && d<numrom)
    {
     //msg.error("KONAMI[%X]=%X",a,d);
//     blah++;
     memcpy(ram+0x8000,&ROM[d],sizeof(ROMPAGE));
     KONAMIswitch=d;
    }
   break;
 }
 return 0;
}

//-------------------------------------------------------
//memory read trap handlers

WORD nesread2000(WORD a)
{
//  msg.printf(3,"%X read",a);
  switch (a)
  {
   case 0x2000: //PPU control register 1
      return ram[a];
   case 0x2001: //PPU control register 2
      return ram[a];
   case 0x2002: //PPU status register
    {
      byte s=0;
      if (vblank) {s|=0x80; vblank=0;} //reset vblank after read
        //see if we've scanned past sprite #0's y coordinate
      if (!hitflag)
       if (spritemem[0].y < getscanline()) {s|=0x40; hitflag=1; }
//      msg.printf(2,"PPU status read %X",ram[0x2002]);
     return s;
    }

   case 0x2004: //sprite mem data read
//      msg.printf(1,"spriteread %d",spritememaddr);
      return ((byte *)spritemem)[spritememaddr++];
   case 0x2005: msg.error("bgscroll read!");  return 0;
   case 0x2007: //PPU data read
     {
      if (ppureadfuckup) {ppureadfuckup=0; return 0;}
      byte d=ppu->read(ppumemaddr);
      ppumemaddr+=(ram[0x2000]&4) ? 32 : 1;
      return d;
     }
   default: return 0;
  }
}

WORD nesread4000(WORD a)
{
// msg.error("upper port read!!");
 switch (a)
 {
  case 0x4016: //joystick
  case 0x4017:
   {
    int joynum=a-0x4016; //get joystick number
    int stat=inputdevice[joynum]->stat; //get current status of dirs
    int but=inputdevice[joynum]->but; //get current status of buts
    switch (ram[a]++)
    {
     case 1: return (but&ID_BUT1) ? 1 : 0; //A
     case 2: return (but&ID_BUT0) ? 1 : 0; //B
     case 3: return (but&ID_BUT2) ? 1 : 0; //Select
     case 4: return (but&ID_BUT3) ? 1 : 0; //Start
     case 5: return (stat&ID_UP ) ? 1 : 0; //
     case 6: return (stat&ID_DOWN) ? 1 : 0; //
     case 7: return (stat&ID_LEFT) ? 1 : 0; //
     case 8: return (stat&ID_RIGHT) ? 1 : 0; //
    }
   }
 }
 return 0;
}

WORD nesread0000(WORD a)
{
// if (a>=0x800) msg.error("weird read %X",a);
 return ram[a&0x7FF];
}

WORD nesread5000(WORD a)
{
 msg.error("expansion read!!");
 return 0;
}

WORD nesread8000(WORD a)
{
// msg.error("direct rom read: %X!!",a);
 return ram[a];
}


//---------------------------------------------
// trap handler tables....



//standard read trap table
struct m6502MemoryRead NESread[]=
{
 {0x0800,0x1FFF, nesread0000}, //low ram
 {0x2000,0x3FFF, nesread2000}, //lower ports
 {0x4000,0x4FFF, nesread4000}, //higher ports
 //{0x5000,0x7FFF, nesread5000}, //expansion
 {0x8000,0xFFFF, nesread8000}, //rom read
 {(WORD) -1,(WORD) -1,NULL}
};

//standard write trap table
struct m6502MemoryWrite NESwrite[]=
{
 {0x0800,0x1FFF, neswrite0000}, //ram
 {0x2000,0x3FFF, neswrite2000}, //lower ports
 {0x4000,0x4FFF, neswrite4000}, //higher ports
 //{0x5000,0x7FFF, neswrite5000}, //expansion
 {0x8000,0xFFFF, neswrite8000}, //rom
 {(WORD) -1,(WORD) -1,NULL}
};
