#ifndef _NESVIDEO_
#define _NESVIDEO_

#include "guirect.h"
#include "ppu.h"

extern "C" {
 void __cdecl draw_tile_asm(struct bitmap8x8 *b,char *dest,int x,int y);
 void __cdecl draw_sprite_asm(struct bitmap8x8 *b,char *dest,int x,int y,int o);
};

#ifdef __WATCOMC__
#pragma aux draw_tile_asm modify[eax ecx edx];
#pragma aux draw_sprite_asm modify[eax ecx edx];
#endif

//8x8 bitmap (for pattern tables
struct bitmap8x8
{
 byte s[8][8];
 void draw_tile(char *dest,int x,int y)
  {draw_tile_asm(this,dest,x,y);} //draw as tile
 void draw_sprite(char *dest,int x,int y,int o) //draw as sprite (transparency and orientation)
  {draw_sprite_asm(this,dest,x,y,o);} //draw as sprite

 void add_tile(byte b); //add b to every pixel in bitmap
 void add_sprite(byte b); //add b to every pixel in bitmap (except 0)

 //create 256 bitmap from nes pattern
 byte create(NES_pattern *np); //returns 1 if all transparent pixels
};

//optimized pattern
struct pattern
{
 //optimized pattern data 8x8 for tiles
 bitmap8x8 tile[4];    //one for each possible attribute

 //optimized pattern data 8x8 for tiles
 bitmap8x8 sprite[4];  //one for each possible attribute

 struct NES_pattern *np; //pointer to nes pattern that this represents
 char updated;    //has cpu altered pattern in name table?
 char issolid;    //is it a solid tile of pixel 0?
 char padding[2];

 void setpatternptr(int tablenum,int index);
 void refresh(); //create optimized pattern from nes data

// void drawtile(char *dest,int x,int y); //draw pattern to screen

// void drawsprite(char *dest,int x,int y,int o); //draw pattern as sprite
                      //1=flipx 2=flipy
};



//class for output to GUI
class nesvideo:public GUIcontents
{
 int xw,yw; //xw and yw of video
 public:
 int maximized;

 byte paletteupdated; //has any of the pal been updated?
 byte palupdateidx[32]; //which ones were updated?
 void refreshpalette();

 pattern ptn[2][256]; //optimized patterns
 byte patternupdated;

 void drawbg(char *dest);
 void drawsprites_8x8(char *dest);
 void drawsprites_8x16(char *dest);

 void reset();
 void resetpatterncache();
 void resetpalette();

 char forcedesktopfill;

 nesvideo(char *romfile);
 virtual ~nesvideo();

 //gui related bullshit
 virtual int acceptfocus() {return 1;}
// virtual void receivefocus();
 virtual void losefocus() { return; } //workspace never loses focus
 virtual int keyhit(char kbscan,char key);
 virtual char *getname() {return "nesvideo";};
 virtual void restore();
 virtual void maximize();
 void resize(int xw,int yw);
 virtual void draw(char *dest);
 virtual GUIrect *nesvideo::click(mouse &m);
};

void updatepalette(int palidx,byte paldata); //update palette number #

//current nes video
extern nesvideo *nv;

#endif
