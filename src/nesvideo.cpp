//functions for NES display
#include <string.h>
#include <stdlib.h>

#include "r2img.h"
#include "font.h"

#include "nesvideo.h"
#include "nes.h"

#include "gui.h"
#include "message.h"

#include "config.h"

#include "dd.h"

//color index from which all nes colors are based
#define CBASE 224

nesvideo *nv; //global nes video

#include "file.h"


//void bitmap8x8::draw_tile(char *dest,int x,int y)
//void bitmap8x8::draw_sprite(char *dest,int x,int y,int o)


#define min(a,b) (a)<(b) ? (a) : (b)


//--------------------------
//palette stuff

#include "nespal.h"

void initializenespalette()
{
/* FILEIO f;
 f.open("d:\\emu\\ines\\nesa.com");
 f.setpos(0xB4e);
 for (int i=0; i<0x40; i++)
  {
   char c;
   f.read(&nespal[i],3); //read palette
   f.read(&c,1);
   msg.printf(2,"%X: %X %X %X",i,nespal[i].r,nespal[i].g,nespal[i].b);
  }
 f.close();

 f.create("nes.pal");
 f.write(nespal,256*3);
 f.close();*/

 FILEIO f;
 f.open("anes.pal");
 f.read(nespal,256*3);
 f.close();

 msg.printf(2,"NES Palette initialized");
}

//32 different indexes
void updatepalette(int palidx,byte paldata)
{
 setpalettenum(CBASE+palidx,&nespal[paldata]);
//  msg.printf(1,"palette[%d]=%X",palidx,paldata);
}

void nesvideo::refreshpalette()
{
 if (!paletteupdated) return;
 //update bg pals
 for (int i=0; i<32; i++)
  if (palupdateidx[i])
   {
    updatepalette(i,ppu->bgpal.c[i]);
    palupdateidx[i]=0;
   }
 paletteupdated=0;
}

void nesvideo::resetpalette()
{
 for (int i=0; i<32; i++) palupdateidx[i]=1;
 paletteupdated=1;
}

//--------------------------------
//pattern stuff

byte bitmap8x8::create(NES_pattern *np)
{
 memset(s,0,8*8); //clear it
 byte issolid=0;
 //create 256 color pattern out of nes bit pattern
 for (int y=0; y<8; y++)
  for (int x=0; x<8; x++)
   {
    if (np->low[y] &(0x80>>x)) s[y][x]|=1;
    if (np->high[y]&(0x80>>x)) s[y][x]|=2;
    issolid|=np->low[y]|np->high[y];
   }
 return !issolid;
}

void bitmap8x8::add_sprite(byte b)
{
 char *t=(char *)s;
 for (int i=0; i<64; i++,t++)
  if (*t) *t|=b;
}

void bitmap8x8::add_tile(byte b)
{
 char *t=(char *)s;
 for (int i=0; i<64; i++,t++) *t|=b;
}

/*
void bitmap8x8::draw_tile(char *dest,int x,int y)
{
 if (x<0 || y<0) return; //clip left
 if (x+8>SCREENX || y+8>SCREENY) return; //clip bottomright
 dest+=y*PITCH+x;
 for (int i=0; i<8; i++,dest+=PITCH)
//   for (int j=0; j<8; j++)  dest[j]=s[i][j];
      memcpy(dest,&s[i],8);
} */

/*
void bitmap8x8::draw_sprite(char *dest,int x,int y,int o)
{
 if (x<0 || y<0) return; //clip left
 if (x+8>SCREENX || y+8>SCREENY) return; //clip bottomright
 dest+=y*PITCH+x;
 switch (o)
 {
  case 0: //normal
   {
    for (int i=0; i<8; i++,dest+=PITCH)
     for (int j=0; j<8; j++)
      if (s[i][j])  dest[j]=s[i][j];
   }
   break;
  case 1: //flipx
   {
   for (int i=0; i<8; i++,dest+=PITCH)
    for (int j=0; j<8; j++)
      if (s[i][j])  dest[j^7]=s[i][j];
   }
   break;
  case 2: //flipy
   {
   for (int i=7; i>=0; i--,dest+=PITCH)
    for (int j=0; j<8; j++)
      if (s[i][j])  dest[j]=s[i][j];
   }
   break;
  case 3: //flipx|flipy
   {
   for (int i=7; i>=0; i--,dest+=PITCH)
    for (int j=0; j<8; j++)
      if (s[i][j])  dest[j^7]=s[i][j];
   }
   break;
 }
} */



void pattern::setpatternptr(int tablenum,int index)
{
 np=&ppu->pt[tablenum].p[index];
 updated=1;
}

//create pattern from NES_pattern
void pattern::refresh()
{
 bitmap8x8 b;
 issolid=b.create(np); //create bitmap

// if (issolid) msg.printf(2,"solid tile!");
 //create bg tiles
 for (int i=0; i<4; i++)
  {
   tile[i]=b; //copy bitmap
   tile[i].add_tile(CBASE+(i<<2)); //add attribute and cbase
  }

 //create sprites
 for (int i=0; i<4; i++)
  {
   sprite[i]=b; //copy bitmap
   sprite[i].add_sprite(CBASE+16+(i<<2)); //add attribute and cbase
  }
 updated=0;
}

void nesvideo::resetpatterncache()
{
 //reset patterns
 for (int i=0; i<256; i++)
  {
   ptn[0][i].setpatternptr(0,i);
   ptn[1][i].setpatternptr(1,i);
  }
 patternupdated=1;
}


//---------------------------------
//main video interface
void initattriblookup();

static DLGPOS dlgpos;
 void cleardesktop();

nesvideo::nesvideo(char *romfile):
 GUIcontents(256,224),maximized(0)
{
 if (nv) delete nv->parent; //delete old video

 xw=width(); yw=height();

 initializenespalette();
 initattriblookup();
 resetpatterncache();

 //wrap us up in box
 GUImaximizebox *p=new GUImaximizebox(guiroot,romfile,this,0,0);
 dlgpos.open(p);
 if (SCREENX<=320)   p->maximize();

 msg.printf(2,"NES video initialized: %dx%d",xw,yw);
 nv=this;
 cleardesktop();
}

nesvideo::~nesvideo()
{
 cfg->set(CFG_NOFILLEDDESKTOP,0); //force desktop to be drawn

 msg.printf(2,"NES video destroyed");
 nv=0;

 freerom();
 dlgpos.close((GUIbox *)parent);
}


//----------------------------------------------

extern byte scrollx,scrolly;


//rotation lookup
byte attriblookup[4][4];

void initattriblookup()
{
 for (int x=0; x<4; x++)
  for (int y=0; y<4; y++)
   {
    byte square=(x>>1) | (y&2);
    attriblookup[x][y]=square*2;
   }
}

inline byte NES_attributetable::getat(int x,int y)
{
 return (a[y/4][x/4]>>attriblookup[x&3][y&3])&3;
}


//draws a name table at a x,y (will do the clipping)
void NES_natable::draw(char *dest,int sx,int sy,pattern *p)
{
 int xstart,xend;
 int ystart,yend;

 if (sx<0) {xstart=(-sx)/8; xend=32;}//left clip
    else   {xstart=0; xend=min((SCREENX-sx)/8+1,32); }
 if (sy<0) {ystart=(-sy)/8; yend=30;}//top clip
    else   {ystart=0; yend=min((SCREENY-sy)/8+1,30);}

 for (int y=ystart; y<yend; y++)
  for (int x=xstart; x<xend; x++)
   p[nt.t[y][x]&0xff].tile[at.getat(x,y)].draw_tile(dest,sx+x*8,sy+y*8);
}

extern int nttoshow;
extern byte mirroring;

void nesvideo::drawbg(char *dest)
{
 int sx=-scrollx;
 int sy=-scrolly-8; //-8 for top clipping

 //find screen pattern table to show...
 pattern *p=(pattern *)&nv->ptn[(ram[0x2000]&16) ? 1 : 0];

 //find name/attribute table to show
 int natnum=nttoshow;//ram[0x2000]&3;

 //draw all name tables
 ppu->nat[natnum]  .draw(dest,sx,sy,p);
 ppu->nat[natnum^1].draw(dest,sx+32*8,sy,p);
 ppu->nat[natnum^2].draw(dest,sx,sy+30*8,p);
 ppu->nat[natnum^3].draw(dest,sx+32*8,sy+30*8,p);

//font[0]->printf(50,50,"scroll=%d,%d",scrollx,scrolly);
//font[0]->printf(50,60,"nametable=%d ptable=%d",ram[0x2000]&3,(ram[0x2000]&16) ? 1 : 0);
}

extern int blah;
inline void NES_sprite::draw_8x8(char *dest,pattern *pt)
{
 if (x<SCREENX && y-8<SCREENY)
  if (!pt[p].issolid)
   pt[p].sprite[attrib].draw_sprite(dest,x,y+1-8,flipx|(flipy<<1));
}

inline void NES_sprite::draw_8x16(char *dest)
{
 if (x<SCREENX && y-8<SCREENY)
 {
  int pnum=p&(~1)^flipy;
  pattern *pt=(pattern *)&nv->ptn[p&1];

  pt[pnum].  sprite[attrib].draw_sprite(dest,x,y+1-8,flipx|(flipy<<1));
  pt[pnum^1].sprite[attrib].draw_sprite(dest,x,y+9-8,flipx|(flipy<<1));
 }
}



void nesvideo::drawsprites_8x8(char *dest)
{
 //find sprite pattern table to show...
 pattern *p=(pattern *)&nv->ptn[(ram[0x2000]&8) ? 1 : 0];

 //draw every sprite
// for (int i=0; i<64; i++)
 for (int i=63; i>=0; i--)
  spritemem[i].draw_8x8(dest,p);
}

void nesvideo::drawsprites_8x16(char *dest)
{
 pattern *p1,*p2;

 //draw every sprite
// for (int i=0; i<64; i++)
 for (int i=63; i>=0; i--)
  spritemem[i].draw_8x16(dest); //,p1,p2);

//font[0]->printf(50,50,"8x16");
//font[0]->printf(50,60,"ptable=%d",(ram[0x2000]&8) ? 1 : 0);
//font[0]->printf(50,50,"%d,%d",scrollx,scrolly);
}




//----------------------------------------------

int ccx,ccy;
int docolorcheck;

void cleardesktop()
{
 #ifdef WIN95
 if (nv) nv->forcedesktopfill=8;
 #else
 if (nv) nv->forcedesktopfill=1;
 #endif
}

extern volatile byte inemu;

//update screen
void nesvideo::draw(char *dest)
{
// //pause till frame is done
// while (CPURunning && inemu) {}

 cfg->set(CFG_NOFILLEDDESKTOP,maximized);

 if (forcedesktopfill>0) {fill(0); forcedesktopfill--;}
    else
 //blank screen...
 if (!(ram[0x2001]&8))
  {
   fill(0);
   return;
  }

 //update patterns
 if (patternupdated)
 {
  for (int i=0; i<256; i++)
  {
   if (ptn[0][i].updated) ptn[0][i].refresh();
   if (ptn[1][i].updated) ptn[1][i].refresh();
  }
  patternupdated=0;
 }


 //force clipping
 CLIP clip(dest,x1+(width()-xw)/2,y1+(height()-yw)/2,x1+(width()+xw)/2,y1+(height()+yw)/2);

 //draw background
 drawbg(dest);

 //draw sprites
 if ((ram[0x2001]&16)) //show sprites
  if (ram[0x2000]&32) //8x16 sprites ?
         drawsprites_8x16(dest);
   else  drawsprites_8x8(dest);

// font[0]->printf(50,50,"hitflag=%d",spritemem[0].y);
//  font[0]->printf(50,60,"0x2000=%X",ram[0x2000]);
// if (ram[0x2000]&0x40) font[0]->printf(50,60,"hitint!!!");

  /*
 if (docolorcheck)
  {
   byte c=dest[ccy*PITCH+ccx];
   c-=CBASE;
   if (c>=0 && c<32)
    {
     COLOR *r=&nespal[ppu->bgpal.c[c]];
     msg.printf(0,"(%d,%d): %s pixel=%X pal=%X RGB=%X,%X,%X",ccx,ccy,
        c>=16 ? "sprite" : "bg",c&0xF,ppu->bgpal.c[c],
        r->r,r->g,r->b);
    }
   docolorcheck=0;
  } */
}

int nesvideo::keyhit(char kbscan,char key)
{
 return 0;
}

#include "mouse.h"
GUIrect * nesvideo::click(mouse &m)
{
/* ccx=m.x-x1;
 ccy=m.y-y1;
 docolorcheck=1;*/
 return 0;
}


//--------------------------
//GUI related bullshit
void enablegui();
void nesvideo::restore()
 {
//msg.printf(2,"nesvideo restore");
  cfg->set(CFG_NOFILLEDDESKTOP,0); //force desktop to be drawn
  maximized=0;
  resize(xw,yw);
  enablegui();
  cleardesktop();
 }

void nesvideo::maximize()
 {
//msg.printf(2,"nesvideo maximize");
  cfg->set(CFG_NOFILLEDDESKTOP,1); //force desktop to not be drawn
  moveto(0,0);
  maximized=1;
  resize(SCREENX,SCREENY);
  cleardesktop();
 }
void nesvideo::resize(int txw,int tyw)
{
//msg.printf(2,"nesvideo resize %d,%d",txw,tyw);
 GUIrect::resize(txw,tyw);
 if (maximized && parent)
   ((GUImaximizebox *)parent)->reposmaxbutton();
}
