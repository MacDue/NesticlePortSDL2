//dos interface functions
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <i86.h>


//#define CALLBACK 

#include "dos\keyb.h"
#include "dos\ttimer.h"

#include "sound.h"
#include "dos\smix.h"
int detect_settings(int *baseio, int *irq, int *dma, int *dma16);


#include "r2img.h"
#include "mouse.h"
#include "input.h"
#include "config.h"
#include "message.h"

#include "dd.h"

#include "timing.h"

#include "command.h"

extern "C" {
 void __cdecl initmouse();
 void __cdecl setmouseextent(int xw,int yw);
 int  __cdecl readmouse(int *x,int*y);

 #include "dos\svga.h"
};

//modex set
void set256x224x256_X(void);

#include "nesvideo.h"

//graphics mode stuff
char vesainstalled=0;
char linear=0;
char mode13=0;
char invesamode=0;

VBE_modeInfo info;
int initvesa();
void terminatevesa();
int setvideomode(int x,int y,int colordepth);

//dos graphics
void mode13h();
void modetext();


//dos sound
char sbinstalled=0;
char sbdetected=0;
int baseio,irq,dma,dma16;

extern char configfile[];
commandline *cline;

extern "C" {
int PITCH=320;
};

char *video=(char *)0xA0000;       // Pointer to video memory (dos)
char *linearvideo=0; //pointer to linear video buffer
char *screen;        // Pointer to virtual screen

mouse m;


int timeperframe=0;

char errstr[80];
void cleanexit(int x)
{
 modetext();
 printf("Error: %s\n",errstr);
 exit(x);
}

void playsound(SOUND *s)
{
 if (!sbinstalled) return;
 start_sound(s,0,255,0);
}




VBE_palette palentries[256]; //palette entries for all colors
void refreshpalette()
{
 if (!invesamode) loadpalette6(palentries,0,256); //use our load palette
  else
     {
      SV_set6BitDAC();
      SV_setPalette(0,256,palentries,-1); ///use their spiffy one
     }
}

void setpalette(PALETTE *p) //assumed 8-bit palette
{
  //copy from COLOR format to VESA format
 for (int i=0; i<256; i++)
  {
   palentries[i].blue=p->c[i].b>>2;
   palentries[i].green=p->c[i].g>>2;
   palentries[i].red=p->c[i].r>>2;
   palentries[i].alpha=0;
  }
 refreshpalette();
}

//set individual entry
void setpalettenum(int index,COLOR *c) //assumed 8 bit palette
{
  //copy from color format to vesa format
 palentries[index].red=c->r>>2;
 palentries[index].green=c->g>>2;
 palentries[index].blue=c->b>>2;

 //set individual color 
 if (!invesamode)  loadpalette6(&palentries[index],index,1);
            else  SV_setPalette(index,1,&palentries[index],-1);
// msg.printf(2,"palette %d=%X,%X,%X",index,c->r,c->g,c->b);
}    


void changeresolution(int x,int y)
{
  if (screen) free(screen);
  setvideomode(x,y,8);
  screen=(char *)malloc(SCREENY*PITCH);
  refreshpalette();
}





static int quit=0;

void quitgame()
{
 quit=1;
}

extern int  (*keyboardhandler)(void);  //pointer to keyboard handler



#define min(a,b) (a<b) ? a : b
#define banksize 0x10000

void copyscreentovideo()
{
 //dos mode 13h copy
 if (!invesamode) {memcpy(video,screen,64000); return;}

 //banked copy
 if (!linear)
 {
  int bytes=SCREENY*PITCH;
  char *src=screen;

  for (int bank=0; bytes>0; bank++)
   {
    SV_setBank(bank);
    memcpy(video,src,min(bytes,banksize));
    src+=banksize;
    bytes-=banksize;
   }
  return;
 }

 //linear copy
 if (linear) memcpy(linearvideo,screen,SCREENY*PITCH);

}



#ifndef CALLBACK
unsigned ticks=0;
unsigned lasttime=0;
void ticktimer()
{
 ticks++;
}
#endif


void disable() {}
void enable() {}

char *getclipboardtext() {return 0;}

void *mapphysicaladdress(unsigned a,unsigned size);

int option_sb(char *p)
{
 dma16=0;
 if (sscanf(p,"%x %d %d",&baseio,&irq,&dma)==3) sbdetected=1;
 return sbdetected;
}

int option_sb16(char *p)
{
 dma=0;
 if (sscanf(p,"%x %d %d",&baseio,&irq,&dma16)==3) sbdetected=1;
 return sbdetected;
}


//dos main
void main(int argc,char *arg[])
{
 //detect sound blaster
 sbdetected=detect_settings(&baseio,&irq,&dma,&dma16);

  //get configuration
 cfg=new config();
 cfg->load(configfile);

 //execute command line...
 cline=new commandline(argc-1,arg+1);
 cline->execute(0);

 //initialize sound blaster
 if (!getoption("nosound") && sbdetected)
  if (init_sb(baseio,irq,dma,dma16)) //initialize sb
  {
   init_mixing();
   set_sound_volume(255);
   sbinstalled=1;
   msg.printf(2,"Soundblaster initialized. %s IRQ%d DMA%d",
    sixteenbit ? "16bit" : "8bit",irq,sixteenbit ? dma16 : dma);
  } else msg.error("ERROR: error initializing soundblaster");


 //start up keyboard handler
 init_keyboard();
 keybios=0;
 keyrepeat=1;
 keyqueue=0; //1;

 //setup timer
 InitializeTimers();


 if (!getoption("novesa")) initvesa();

 if (!setvideomode(SCREENX,SCREENY,8))  setvideomode(320,200,8);

 //allocate space for offscreen buffer
 screen=(char *)malloc(SCREENY*PITCH);


 //initialize game data
 if (initgame())
  {
   modetext();
   printf("Error initializing game\n");
   return;
  }

 //execute post system startup
 cline->execute(1);

 //start up timer
 SetTimerSpeed(TIMERSPEED);
 #ifdef CALLBACK
 SetTimerFunc(gametimer);
 #else
 SetTimerFunc(ticktimer);
 #endif

 do
 {
  //refresh palette
  if (nv) nv->refreshpalette();
     
  //update mouse
  {
   int x,y;
   m.reset();
   m.updatebut(readmouse(&x,&y));
   m.updatexy(x,y);
  }

 //process keys
  while (keyhit())
   {
    kbscan=getkey();
    keyboardhandler();
   }

  if (!cfg->get(CFG_NOFILLEDDESKTOP))
      memset(screen,9*16+14,SCREENY*PITCH);

  //update screen  
  updatescreen();

  //copy to video buffer
  copyscreentovideo();

  #ifndef CALLBACK
   while (ticks>lasttime+1)
    {
     gametimer();
     lasttime++;
    }
  #endif
  
 } while (!quit);


 SetTimerFunc(0);
 set_keyboard_func(0);

 terminategame();
 if (vesainstalled) terminatevesa();
     else modetext();
}    

//dos surface
surface::surface(int txw,int tyw) {s=SCR::newSCR(txw,tyw);}
surface::~surface() {free(s);}

char *surface::lock()
{
 if (!s) return 0;

 oldscrx=SCREENX; oldscry=SCREENY; oldpitch=PITCH;
 SCREENX=s->xw;
 SCREENY=s->yw;
 PITCH=s->xw;

 return s->data();
}

void surface::unlock()
{
 if (!s) return;
 SCREENX=oldscrx; SCREENY=oldscry; PITCH=oldpitch;
}


int surface::blt(char *dest,int x,int y)
{
 if (s) {s->draw(dest,0,0); return 1;}
 return 0;
}




//--------------------------------------------
//hardware specific functions

//dos mode changes
void mode13h();
#pragma  aux mode13h =  \
  "mov eax,13h"            \
  "int 10h"               \
  parm   []               \
  modify [eax];

void modetext();
#pragma  aux modetext =  \
  "mov eax,3h"            \
  "int 10h"               \
  parm   []               \
  modify [eax];


//vesa stuff
extern "C"
{
void _cdecl VBE_fatalError(char *msg)
{
 strcpy(errstr,msg);
 cleanexit(-1);
}
};


int setvideomode(int x,int y,int colordepth)
{
 if (vesainstalled)
 {
  //go through all video modes
  for (int i=0; modeList[i]!=0xFFFF; i++)
  {
  int modenum=modeList[i];
  VBE_getModeInfo(modenum,&info);

  if (info.XResolution==x && info.YResolution==y && info.BitsPerPixel==colordepth)
    {
     linear=(info.ModeAttributes&vbeMdLinear) && !getoption("banked");
     if (!linear && getoption("linear")) return 0; //not a linear

     if (linear) modenum|=vbeLinearBuffer;
     if (SV_setMode(modenum)==FALSE)
      {
       msg.error("Failure setting vesa mode %dx%dx%d",SCREENX,SCREENY,colordepth);
       vesainstalled=0;
       return setvideomode(320,200,8);
      }

     if (linear && !linearvideo) //take care of linear buffer shit
     {
      linearvideo=(char *)mapphysicaladdress(info.PhysBasePtr,memory*1024);
      if (!linearvideo)
       {
        msg.printf(5,"ERROR: cannot map linear buffer");
        vesainstalled=0;
        setvideomode(320,200,8);
       } else  msg.printf(1,"linear buffer: %X -> %X\n",info.PhysBasePtr,linearvideo);
     }

     SCREENX=info.XResolution;
     SCREENY=info.YResolution;
     PITCH=info.BytesPerScanLine;
     invesamode=1;

     initmouse();
     setmouseextent(SCREENX,SCREENY);

     msg.printf(2,"VESA mode set: %dx%dx%d %s",SCREENX,SCREENY,colordepth,linear ? "linear" : "banked");
     return 1;
    }
  }
 }

 //see if we can use mode 13 instead....
 if (x==320 && y==200 && colordepth==8)
  {
   mode13h();
   SCREENX=x; SCREENY=y; PITCH=SCREENX;
   initmouse();
   setmouseextent(SCREENX,SCREENY);
   linear=0; invesamode=0;
   msg.printf(2,"Mode 13h initialized. %dx%dx%d",SCREENX,SCREENY,8);
   return 1;
  }


 //see if we can use modex instead....
 if (x==256 && y==224 && colordepth==8)
  {
   set256x224x256_X();
   SCREENX=x; SCREENY=y; PITCH=SCREENX;
   initmouse();
   setmouseextent(SCREENX,SCREENY);
   linear=0; invesamode=0;
   msg.printf(2,"ModeX mode initialized. %dx%dx%d",SCREENX,SCREENY,8);
   return 1;
  }


 msg.printf(5,"%dx%dx%d mode not supported",x,y,colordepth);
 return 0; //mode not even found
}

int initvesa()
{
 if (vesainstalled) return 1;
 if (!SV_init())
  {
   msg.printf(2,"VESA not installed");
   vesainstalled=0;
   return 0;
  }
 msg.printf(2,"VESA Version %X.%X detected.",VBEVersion>>8,VBEVersion&0xFF);
 msg.printf(2,"%s",OEMString);

 vesainstalled=1;
 return 1;
}

void terminatevesa()
{
 if (!vesainstalled) return;
 SV_restoreMode();
}    





void *mapphysicaladdress(unsigned a,unsigned size)
{
 union REGS regs;
 struct SREGS sregs;

 regs.x.eax = 0x800;
 regs.x.ecx = a;
 regs.x.ebx = a>>16;
 regs.x.edi = size;
 regs.x.esi = size>>16;

 memset( &sregs, 0, sizeof(sregs) );
 int386x( 0x31, &regs, &regs, &sregs );
 if (regs.x.eax&0x8000) return 0;
 return (void *) ((regs.x.ebx<<16)|(regs.x.ecx&0xFFFF));
}

#ifdef __WATCOMC__
int random(int x)
{
 return rand()*x/32768;
}    
#endif























