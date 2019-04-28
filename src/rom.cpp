//routines for handling roms

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

#include "nes.h"
#include "nesvideo.h"

//rom stuff
NESROMHEADER romhdr;
char *trainer;
int numrom,numvrom;
ROMPAGE *ROM;
VROMPAGE *VROM;

//
char *ram; //CPU address space
NES_ppumemory *ppu; //PPU address space
NES_sprite *spritemem; //Sprite memory

int NESinitialized=0;

int romloaded=0;
void freerom();

//-----------------------------------------------------
// NES rom loader

void loadrom(char *romfile)
{
 if (!romfile) return;
 char s[64];
 strcpy(s,romfile);
 if (!strchr(s,'.')) strcat(s,".nes"); //add extension

 //free old rom
 freerom();

 msg.printf(2,"Loading rom %s...",s);
 FILEIO f;
 if (f.open(s)) {msg.error("Unable to open file %s",s); return;}

  //read header
 if (f.read(&romhdr,sizeof(romhdr))) {msg.error("Unable to read header"); return;}

 if (!romhdr.validate()) {msg.error("Bad ROM header"); return;}

// if (romhdr.banktype!=0)
//  {msg.error("No memory mappers supported (yet)"); return;}

 //load trainer
 if (romhdr.trainer) trainer=(char *)f.readalloc(512);

 //read ROM
 numrom=romhdr.num16k;
 ROM=(ROMPAGE *)f.readalloc(numrom*sizeof(ROMPAGE));
 if (ROM) msg.printf(1,"%dK ROM read",numrom*sizeof(ROMPAGE)/1024);

 //read VROM
 numvrom=romhdr.num8k;
 VROM=(VROMPAGE *)f.readalloc(numvrom*sizeof(VROMPAGE)); //read vrom
 if (VROM) msg.printf(1,"%dK VROM read",numvrom*sizeof(VROMPAGE)/1024);

 f.close();
 msg.printf(2,"%s loaded",romfile);
 romloaded=1;

 //initialize nes hardware....
 initNEShardware();

 //create video space
 new nesvideo(romfile);
}

void freerom()
{
 terminateNEShardware();
 FREE(trainer);
 FREE(ROM);
 FREE(VROM);

 if (romloaded) msg.printf(2,"ROM freed");
 romloaded=0;

 //delete video space
 if (nv && nv->parent)  delete nv->parent;
}

void m_showmessages();
void m_getrominfo()
{
 if (!romloaded) msg.error("ROM not loaded");
  else  romhdr.printinfo(); //print info about rom
 m_showmessages();
}

void m_free()
{
 if (!romloaded) msg.error("ROM not loaded");
  else freerom();
}


//create data structures for nes hardware
void initNEShardware()
{
 if (NESinitialized) return;
 ppu=new NES_ppumemory;

 ram=(char *)malloc(0x10000);
 msg.printf(2,"64K CPU address space created");

 spritemem=(NES_sprite *)malloc(256);
 msg.printf(2,"256 byte sprite mem created");
 NESinitialized=1;
}

//free data structures for nes hardware
void terminateNEShardware()
{
 if (!NESinitialized) return;
 m_stop();
 DELETE(ppu);
 FREE(ram);
 FREE(spritemem);
 NESinitialized=0;
}

//---------------------------------------------------------
//NES rom header

char *banktypestr[8]=
{
 "None",    //0
 "Sequential",  //1
 "Konami",   //2
 "VROM Switch", //3
 "5202 Chip", //4
 "Unknown", //5
 "Unknown", //5
 "Unknown", //5
};

int NESROMHEADER::validate() {return (str[0]=='N' && str[1]=='E' && str[2]=='S');}
void NESROMHEADER::printinfo()
{
 msg.printf(1,"# of 16K ROM banks: %d",num16k);
 msg.printf(1,"# of  8K VROM banks: %d",num8k);

 msg.printf(1,"%s mirroring",mirroring ? "Vertical" : "Horizontal");
 if (battery) msg.printf(1,"Battery backed RAM");
 if (trainer) msg.printf(1,"Trainer");
 if (reservedbit) msg.printf(1,"reserved bit set");


 msg.printf(1,"MMC #%d: %s",banktype,banktypestr[banktype]);
}
//-----------------------------------------------
