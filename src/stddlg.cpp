#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>

#include "types.h"

#include "r2img.h"
#include "font.h"
#include "dd.h"

#include "mouse.h"
#include "message.h"
#include "gui.h"
#include "guimenu.h"
#include "guicolor.h"


#include "keyb.h"

#include "config.h"

#include "guivol.h"

//----------------------
//  GUI messagebox
//----------------------
#define MSGYW 8 //height in lines

class GUImessage:public GUIcontents
{
 class msgbuffer *m;
 GUIvscrollbar *scroll;
 public:
 void setrange()
 {
  int yw=m->num-MSGYW;
  if (yw<0) yw=0;
  scroll->setrange(0,yw);
  scroll->setpos(yw);
  m->updated=0;
 }
 GUImessage(class msgbuffer *tm):
  GUIcontents(261,MSGYW*10)
  {
   m=tm;
   scroll=new GUIvscrollbar(this,width()-12,0,height());
   setrange();
  }

 virtual void draw(char *dest)
 {
  if (m->updated) setrange();
  fill(CLR_BOX);
  m->draw(x1+2,y1,scroll->getpos(),MSGYW);
  GUIrect::draw(dest);
//  font[0]->printf(50,50,"%d",m->num);
 }
 virtual char *getname() {return "message";}
 virtual ~GUImessage() {}
};

GUIrect *newguimessage(msgbuffer *tm)
 {return new GUImessage(tm);}

class systemmessages:public GUImessage
{
 public:
 systemmessages(msgbuffer *tm) :GUImessage(tm) {}
 //open functions
 static DLGPOS pos; //saved last position of dialog
 static void open()
  {
   pos.open(new GUIbox(guiroot,"Messages",new systemmessages(&msg),0,0));
//   cfg->set(CFG_SHOWMESSAGE,1);
  };
 virtual ~systemmessages() {pos.close((GUIbox *)parent); } //cfg->set(CFG_SHOWMESSAGE,0);}
};
DLGPOS systemmessages::pos;


//--------------------
//   about box
//----------------------
extern char buildtime[],builddate[],buildcompiler[];
extern int buildcompilerversionhigh,buildcompilerversionlow;
extern float version; //game version
class aboutdlg:public GUIcontents
{
 public:
 aboutdlg():GUIcontents(290,65)
 {
  char s[80];

  new GUIstaticimage(this,guivol.about,5,5);
  new GUIstatictext(this,2,"Buddy says:",70,5);
  #ifdef WIN95
  new GUIstatictext(this,3,"aNES Win95",70,20);
  #endif
  #ifdef DOS
  new GUIstatictext(this,3,"aNES DOS",70,20);
  #endif

  sprintf(s,"V%d.%02d",(unsigned)version,((unsigned)(version*100))%100);
  new GUIstatictext(this,5,s,250,20);

  new GUIstatictext(this,3,"Copyright (C) 1997 Bloodlust Software",70,50);
  sprintf(s,"%s %d.%d %s %s",buildcompiler,buildcompilerversionhigh,buildcompilerversionlow,builddate,buildtime);
  new GUIstatictext(this,3,s,70,30);
 }

 virtual int sendmessage(GUIrect *c,int guimsg) {return 1;};
 virtual int acceptfocus() {return 1;}
 virtual char *getname() {return "aboutdlg";}
 virtual void draw(char *dest) {fill(CLR_BOX); GUIrect::draw(dest);};

 //open functions
 static DLGPOS pos; //saved last position of dialog
 static void open()
  {
   pos.open(new GUIonebuttonbox(guiroot,"About",new aboutdlg(),"Thanks Buddy!",0,0));
  };
 virtual ~aboutdlg() {pos.close((GUIbox *)parent);}
};
DLGPOS aboutdlg::pos;


//--------------------------------------
void disablegui();



#include "nesvideo.h"

void setres(int xw,int yw)
{
 //deactivate shit

 changeresolution(xw,yw);

 root->resize(SCREENX,SCREENY);

 if (nv && SCREENX<=320)
  ((GUImaximizebox *)nv->parent)->maximize();

 //reactivate shit
}


void res256x224() {setres(256,224);}
void res320x200() {setres(320,200);}
void res320x240() {setres(320,240);}
void res640x400() {setres(640,400);}
void res640x480() {setres(640,480);}
void res800x600() {setres(800,600);}
void res1024x768() {setres(1024,768);}


void m_heapcheck()
{
  // switch(_heapset(0x69))
  // {
  //  case _HEAPBADNODE: msg.printf(5,"ERROR: Bad heap node"); return;
  //  case _HEAPEMPTY: msg.printf(5,"ERROR: Heap empty"); return;
  //  case _HEAPOK: msg.printf(1,"Heap OK"); return;
  // };
}


void m_showfps()
{
 msg.printf(1,"Show FPS %s",cfg->toggle(CFG_SHOWFPS) ? "on" :"off");
}

extern char configfile[];
void m_saveconfig(){cfg->save(configfile);}

void editinputdevice0();
void editinputdevice1();


#ifdef WIN95
void ddrawinfo();
void m_ddrawinfo()
{
 systemmessages::open();
 ddrawinfo();
}
#endif

void m_quit()
{
 quitgame();
}




void cleardesktop();
GUImenu *gmenu=0;
int guienabled=0;
void enablegui()
{
 if (guienabled) return;
 guienabled=1;
 cleardesktop();
}

void disablegui()
{
 if (!guienabled) return;
 GUIrect::setfocus(gmenu);
 guienabled=0;
 cleardesktop();
}

void togglegui()
{
 if (guienabled) disablegui(); else enablegui();
}

void m_showmessages()
{if (!systemmessages::pos.opened) systemmessages::open();}

void m_load();

void m_execute();
void m_stop();
void m_free();
void m_getrominfo();

void m_pause();
void m_resume();
void m_patternview();
void m_nametableview();
void m_paletteview();

//------------

//*************************************
//           menu definition
//*************************************


menu filemenu=
{
 {
  {"Load ROM...",m_load,'l',0},
  {"Free ROM",m_free,0,0},
  {"Get ROM info",m_getrominfo,0,0},
  {"-----",0,0,0},
  {"Exit",m_quit,'q',0},
  {0,0,0,0},
 }
};

menu NESmenu=
{
 {
  {"Execute",m_execute,0,0},
  {"Stop",m_stop,0,0},
  {"-----",0,0,0},
  {"Pause",m_pause,0,0},
  {"Resume",m_resume,0,0},
  {0,NULL,0,0},
 }
};


menu viewmenu=
{
 {
  {"Messages",m_showmessages,0,0},
  {"-----",0,0,0},
  {"Pattern tables",m_patternview,0,0},
  {"Name tables",m_nametableview,0,0},
  {"Palette",m_paletteview,0,0},
  {0,NULL,0,0},
 }
};

menu resmenu =
{
 {
  {"256x224",res256x224,0,0},
  {"320x200",res320x200,0,0},
  {"320x240",res320x240,0,0},
  {"640x400",res640x400,0,0},
  {"640x480",res640x480,0,0},
  {"800x600",res800x600,0,0},
  {"1024x768",res1024x768,0,0},
  {0,NULL,0,0},
 }
};


menu redefineinputmenu=
{
 {
  {"Device 1",editinputdevice0,0,0},
  {"Device 2",editinputdevice1,0,0},
  {0,NULL,0,0},
 }
};

menu settingsmenu=
{
 {
  {"Redefine input",0,0,&redefineinputmenu},
  {"Resolution",0,0,&resmenu},
  {"-----",0,0,0},
  {"Save config",m_saveconfig,0,0},
  {0,NULL,0,0},
 }
};

menu miscmenu=
{
 {
  {"Hide GUI",disablegui,0,0},
  {"Show FPS",m_showfps,0,0},
  {"Heap check",m_heapcheck,0,0},
  #ifdef WIN95
  {"Directdraw Info",m_ddrawinfo,0,0},
  #endif
  {"-----",0,0,0},
  {"About aNES...",aboutdlg::open,0,0},
  {0,NULL,0,0},
 }
};




menu mainmenu=
{
 {
  {"ROM",0,0,&filemenu},
  {"CPU",0,0,&NESmenu},
  {"View",0,0,&viewmenu},
  {"Settings",0,0,&settingsmenu},
  {"Misc",0,0,&miscmenu},
  {0,NULL,0,0},
 }
};


void initdefaultgui()
{
 gmenu=new GUIhmenu(guiroot,&mainmenu,0,0);
 enablegui();
}
