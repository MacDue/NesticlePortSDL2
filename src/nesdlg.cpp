#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "types.h"

#include "r2img.h"
#include "font.h"
#include "dd.h"

#include "mouse.h"
#include "message.h"
#include "gui.h"
#include "guimenu.h"
#include "guicolor.h"

#include "file.h"

#include "keyb.h"

#include "config.h"

#include "guivol.h"

#include "nesvideo.h"

void loadrom(char *filename);


//------------------------------
//dir listing box

int filldirlistbox(char *filename,class GUIdirlistbox *b);

class GUIdirlistbox:public GUIstringlistbox
{
 public:
 GUIdirlistbox(GUIrect *p,int x,int y,int xw,int iy,char *path)
  :GUIstringlistbox(p,x,y,xw,iy,10)
  {
   enumdir(path,(DIRFUNCPTR)filldirlistbox,this);
  }

 virtual char *getname() {return "guidirlist";}
};

int filldirlistbox(char *filename,GUIdirlistbox *b)
{
 int num=b->getnumitems();
 char **s=(char **)b->resizeitems(num+1);
 s[num]=strdup(filename);
 return 1;
}



//----------------------
//   load rom
//----------------------
class loaddlg:public GUIcontents
{
 GUIstringlistbox *list;

 public:
 loaddlg():GUIcontents(140,100)
 {
//  list=new GUIstringlistbox(this,5,5,width()-10,9,10);
  list=new GUIdirlistbox(this,5,5,width()-10,9,"*.nes");
 }
 virtual char *getname() {return "loadrom";}

 int sendmessage(GUIrect *c,int guimsg)
  {
   if (guimsg==GUIMSG_OK || guimsg==GUIMSG_LISTBOXDBLCLICKED)
    {
     loadrom(list->getselptr());
     return 1;
    }
   return 1;
  };
 virtual void draw(char *dest) {fill(CLR_BOX); GUIrect::draw(dest);}

 //open functions
 static DLGPOS pos; //saved last position of dialog
 virtual ~loaddlg() {pos.close((GUIbox *)parent);}
};
DLGPOS loaddlg::pos;

void m_load()
{
 loaddlg::pos.open(new GUIonebuttonbox(guiroot,"Load .NES ROM",new loaddlg(),"Load",0,0));
}

//--------------------------------------------------



//----------------------
//   view pattern tables
//----------------------

//shows a particular pattern table
class patterntableview: public GUIrect
{
 int pnum; //index to pattern table...

 public:

 patterntableview(GUIrect *_parent,int _pnum)
   :GUIrect(_parent,0,0,16*8,16*8) {pnum=_pnum;}

 virtual void draw(char *dest)
  {
   if (!nv) return;
   pattern *p=(pattern *)&nv->ptn[pnum];

   CLIP clip(dest,x1,y1,x2,y2);

   for (int i=0; i<256; i++)
     p[i].tile[0].draw_tile(dest,(i&15)*8,(i/16)*8);
  }

 virtual char *getname() {return "patterntable";}
};



class patterndlg:public GUIcontents
{
 patterntableview *p1,*p2;

 public:
 patterndlg():GUIcontents(16*8,16*8*2+8)
 {
  p1=new patterntableview(this,0);
  p2=new patterntableview(this,1);
  p2->moveto(p1->x1,p1->y2+8);
 }
 virtual char *getname() {return "patterndlg";}

 int sendmessage(GUIrect *c,int guimsg)
  {
   return 1;
  };
// virtual void draw(char *dest) {fill(CLR_BOX); GUIrect::draw(dest);}

 //open functions
 static DLGPOS pos; //saved last position of dialog
 virtual ~patterndlg() {pos.close((GUIbox *)parent);}
};
DLGPOS patterndlg::pos;

void m_patternview()
{
 patterndlg::pos.open(new GUIbox(guiroot,"Pattern tables",new patterndlg(),0,0));
}


//-------------------------------------------------------------



//----------------------
//   view name tables
//----------------------

#include "nes.h"

class namedlg:public GUIcontents
{
 public:
 namedlg():GUIcontents(32*2*8,30*2*8) {}
 virtual char *getname() {return "nametabledlg";}

 int sendmessage(GUIrect *c,int guimsg){ return 1; };
 virtual void draw(char *dest)
  {
   if (!nv) return;
   CLIP clip(dest,x1,y1,x2,y2);

   //find screen pattern table to show...
   pattern *p=(pattern *)&nv->ptn[(ram[0x2000]&16) ? 1 : 0];

   ppu->nat[0].draw(dest,0,0,p);
   ppu->nat[1].draw(dest,32*8,0,p);
   ppu->nat[2].draw(dest,0,30*8,p);
   ppu->nat[3].draw(dest,32*8,30*8,p);
  }

 //open functions
 static DLGPOS pos; //saved last position of dialog
 virtual ~namedlg() {pos.close((GUIbox *)parent);}
};
DLGPOS namedlg::pos;

void m_nametableview()
{
 namedlg::pos.open(new GUIbox(guiroot,"Name tables",new namedlg(),0,0));
}




//----------------------
//   view palette
//----------------------

//first index of nes color
#define CBASE 224

class paletteviewdlg:public GUIcontents
{
 public:
 paletteviewdlg():GUIcontents(16*16,16*2+4) {}
 virtual char *getname() {return "paletteviewdlg";}

 int sendmessage(GUIrect *c,int guimsg){ return 1; };
 virtual void draw(char *dest)
  {
   if (!nv) return;

   int x;
   //bg pal
   for (x=0; x<16; x++)
    drawrect(dest,CBASE|x,x1+x*16,y1,16,16);

   //sprite pal
   for (x=0; x<16; x++)
    drawrect(dest,CBASE|16|x,x1+x*16,y1+16+4,16,16);

 }

 //open functions
 static DLGPOS pos; //saved last position of dialog
 virtual ~paletteviewdlg() {pos.close((GUIbox *)parent);}
};
DLGPOS paletteviewdlg::pos;

void m_paletteview()
{
 paletteviewdlg::pos.open(new GUIbox(guiroot,"Palettes",new paletteviewdlg(),0,0));
}

















































