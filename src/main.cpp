//6502 nes emulator front end
//main platform independant
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
// #include <mem.h>
#include <stdlib.h>
// #include <direct.h>

#include "config.h"

#include "keyb.h"

#include "types.h"
#include "r2img.h"
#include "font.h"

#include "file.h"
#include "vol.h"
#include "mouse.h"
#include "message.h"

#include "dd.h"

#include "guivol.h"
#include "gui.h"
/*
#include "guirect.h"
#include "guiroot.h"
#include "guicolor.h"
  */
#include "uutimer.h"

#include "nesvideo.h"
#include "nes.h"
#include "timing.h"

#include "command.h"

char appname[]="aNES";
char configfile[]="aNES.cfg";

float version=0.0;

#ifdef WIN95
int SCREENX=640;
int SCREENY=480;
#else
int SCREENX=256;
int SCREENY=224;
#endif

//config info
config *cfg;

//input devices
input *inputdevice[2];
inputdevicesettings *ids;


GUIVOL guivol;
int GUIVOL::size() {return sizeof(GUIVOL);}

FONT *font[10]; //fonts
COLORMAP *shadowmap; //shaded colormapping

volatile unsigned uu=1,su=1;

//buffer for messages
msgbuffer msg;

//root of a32
class ROOT *root=0;

//main root of gui tree
GUIroot *guiroot=0;

//objectspace *objspace=0;

extern int guienabled;
void enablegui();
void disablegui();
void togglegui();


int keyboard();
void initdefaultgui();


 //create all the fonts and colors
void createfonts()
{
 for (int i=0; i<7; i++) font[i]=guivol.font->duplicate();
  //peach
 font[0]->convertcolor((char)0xca,(char)0xc6); font[0]->convertcolor((char)0xdb,(char)0xd8);
   //white
 font[1]->convertcolor((char)0xca,(char)0xf); font[1]->convertcolor((char)0xdb,(char)0x0);
  //green
 font[2]->convertcolor((char)0xca,(char)6*16); font[2]->convertcolor((char)0xdb,(char)0x0);
  //grey something
 font[3]->convertcolor((char)0xca,(char)19);   font[3]->convertcolor((char)0xdb,(char)0x0);
   //black
 font[4]->convertcolor((char)0xca,(char)0x0);  font[4]->convertcolor((char)0xdb,(char)15);
  //red
 font[5]->convertcolor((char)0xca,(char)2*16+1);   font[5]->convertcolor((char)0xdb,(char)2*16+14);
   // ???
 font[6]->convertcolor((char)0xca,(char)10*16+3);   font[5]->convertcolor((char)0xdb,(char)0);
 }

//initialize
int initgame()
{
 //set up keyboard handler
 set_keyboard_func(keyboard);

 //initialize input devices
 ids=&cfg->ids; //copy over input settings

 inputdevice[0]=newinputdevice(cfg->pinput[0]);
 inputdevice[1]=newinputdevice(cfg->pinput[1]);

 //load up graphics and shit
 if (!guivol.read("gui.vol")) return -1;

 cfg->set(CFG_NOFILLEDDESKTOP,0); //force desktop to be drawn

 //fixup pal to 8bit
 for (int i=0; i<256; i++)
 {
  guivol.pal->c[i].r<<=2;
  guivol.pal->c[i].g<<=2;
  guivol.pal->c[i].b<<=2;
 }

 setpalette(guivol.pal);

 //create fonts
 createfonts();

 //create root
 root=new ROOT;

 //create root of all GUI
 new GUIroot(root);

 initdefaultgui();
 return 0;
}


void m_stop();
void terminategame()
{
 m_stop();
 delete root;
 guivol.free();
 delete cfg;
}

void tickemu(); //tick emulation
void gametimer()
{
 uu++;
 input::refreshtimer();
 inputdevice[0]->read();
 inputdevice[1]->read();
 tickemu();
}


int keyboard()
{
// if (kbscan==KB_ESC) {quitgame(); return 1;}
 if (kbscan==KB_ESC) {togglegui(); return 1;}

 if (input::refreshkeyboard(kbscan)) return 1; //dont add key to queue

 if (kbscan&0x80) return 1; //release

 char key=scan2ascii(kbscan);
 if (root && root->keyhit(kbscan,key)) enablegui();
 return 1;
}



extern int timeperframe;
uutimer fpstimer(200);
int fps=0,spf=0;
int calcfps()
{
 //calculate fps
 if (fpstimer.check())
   {
    fps=su*TIMERSPEED/fpstimer.dur;
    spf=timeperframe*(1000/TIMERSPEED)/su;
    fpstimer.reset();
    timeperframe=0;
    su=0;
   }
 return fps;
}



int blah=0,blah2=0;
extern  volatile int cycles;
uutimer cycletimer(100);
float cps;

void updatescreen()
{
 //draw message buffer
 input::refreshmain();

 if (guienabled)
 {
   //draw everything
  root->draw(screen);
  m.draw(screen); //draw cursor
 } else if (nv) nv->draw(screen);


 if (cycletimer.check())
  {
   cps=((float)cycles)*TIMERSPEED/cycletimer.dur/1000000;
   cycletimer.reset();
   cycles=0;
  }

 su++;
 if (cfg->get(CFG_SHOWFPS))
  {
   font[0]->printf(SCREENX-70,50,"fps=%d",calcfps());
   font[0]->printf(SCREENX-70,60,"spf=.%04d",spf);
  font[0]->printf(SCREENX-70,70,"Mcps=%.2f",cps);
  }


//  font[0]->printf(SCREENX-150,80,"%d",surfacelost);
//  font[0]->printf(SCREENX-150,80,"%d",blah);
// font[0]->printf(SCREENX-150,80,"objspace:%p",objspace);
//font[0]->printf(SCREENX-150,80,"%d %d",blah,blah2);
}



//------------------------------
//       ROOT node
//------------------------------

ROOT::ROOT():GUIrect(0,0,0,SCREENX,SCREENY){}
ROOT::~ROOT() {}

void ROOT::resize(int xw,int yw)
{
 if (guiroot) guiroot->resize(xw,yw);
 if (nv && nv->maximized) nv->resize(xw,yw);
/*
 if (nv && nv->maximized)
  {
   ((GUImaximizebox *)nv->parent)->restore();
   ((GUImaximizebox *)nv->parent)->maximize();
  }*/
 GUIrect::resize(xw,yw);
}

void ROOT::refresh(int r,void *c)
 {for (GUIrect *g=child; g; g=g->next) g->refresh(r,c);};


 //-----------------




int cmd_loadrom(char *p)
{
 char s[128];
 if (sscanf(p,"%s",s)<1) return 0;
 loadrom(p);
 return 1;
}

int cmd_runrom(char *p)
{
 char s[128];
 if (sscanf(p,"%s",s)<1) return 0;
 loadrom(p);
 m_execute();
 return 1;
}
