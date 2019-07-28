//structures representing data in the PPU memory space
#ifndef _PPU_
#define _PPU_

#include "types.h"

//struct representing a pattern in the pattern table of the PPU memory
struct __attribute__ ((packed)) NES_pattern
{
 char low[8];  //low bits of pixel data
 char high[8]; //high bits of pixel data
};

//struct representing a whole pattern table of the PPU memory
struct __attribute__ ((packed)) NES_patterntable
{
 NES_pattern p[256];
};

//struct representing a name table
struct __attribute__ ((packed)) NES_nametable
{
 char t[30][32]; //indices into pattern table
};

//struct representing attribute table
struct __attribute__ ((packed)) NES_attributetable
{
 byte a[8][8];
 byte getat(int x,int y); //get attribute of tile at x,y
};

struct __attribute__ ((packed)) NES_palette
{
 char c[16]; //??? dunno how this is stored
};

//combination name /attribute table
struct __attribute__ ((packed)) NES_natable
{
 NES_nametable nt;
 NES_attributetable at;

 //draw a section of this name table to dest at x,y
 //tx1,ty1 is UL tile to draw, tx2,ty2 is the LR tile to draw
 //uses pattern table *p
 void draw(char *dest,int sx,int sy,struct pattern *p);

};

struct __attribute__ ((packed)) NES_ppumemory
{
 NES_patterntable pt[2]; //2 pattern tables

 NES_natable nat[4]; //4 name/attribute tables

 char empty[0xF00];

 NES_palette bgpal;
 NES_palette spritepal;

 char empty2[0xE0];

 NES_ppumemory();
 void clear();
 byte read(word a);
 void write(word a,byte d);
};

struct __attribute__ ((packed)) NES_sprite
{
 byte y; //y position -1
 byte p; //pattern number

 char attrib:2;       //attribute for color...
 char unknown:3; //???
// char unknown1:1; //???
// char unknown2:1; //???
// char unknown3:1; //???
 char behindbg:1; //behind bg
 char flipx:1;    //flip horizontally
 char flipy:1;    //flip vertically

 byte x; //x position

 //draw sprite!
 void draw_8x8(char *dest,struct pattern *p);
 void draw_8x16(char *dest); //,struct pattern *p,struct pattern *p2);
};


#endif
