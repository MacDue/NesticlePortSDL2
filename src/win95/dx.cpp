#define INITGUID
//#define WIN32_LEAN_AND_MEAN
// #include <windows.h>
#ifndef _WIN32
#define _WIN32
#endif
#ifndef WIN32
#define WIN32
#endif

//#define NOSOUND

//#define TIMERCALLBACK

#include <stdio.h>
// #include <ddraw.h>
// #include <dsound.h>
#include <stdlib.h>

#include "r2img.h"
#include "dd.h"

#include "keyb.h"

#include "mouse.h"
#include "message.h"

#include "sound.h"


#include "config.h"
#include "command.h"

#include "nesvideo.h"

#include "timing.h"

typedef int HWND;
int hwnd = -1;

//functions that need to be supplied by the game
int  initgame();
void updatescreen();
void terminategame();
void gametimer();

int PITCH=0;

// DWORD playMIDIFile(HWND hWndNotify, LPSTR lpszMIDIFileName);


//Global variables and shit
// extern char appname[];
// extern char configfile[];
// HINSTANCE hInst;
//
// //direct draw objects
// LPDIRECTDRAW DDO1       =0;
// LPDIRECTDRAW2 DDO       =0;
//
// LPDIRECTDRAWSURFACE ddprimary=0;  //DD primary surface
// LPDIRECTDRAWSURFACE ddbackbuffer=0,ddoff=0;  //DD backbuffer surface
// LPDIRECTDRAWPALETTE ddpalette=0; //DD palette
// PALETTEENTRY palentries[256]; //palette entries for all colors
//
// int surfacelost=0; //count of surfaces lost
//
// //direct sound objects
// LPDIRECTSOUND DSOUND=0; //directsound object
// PCMWAVEFORMAT pcmwf; //wave format of sounds
// LPDIRECTSOUNDBUFFER dsprimary; //primary sound buffer
//
// //window handle
// HWND        hwnd ;
// //window function
// LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM) ;
//
// //timer stuff
// UINT timerid=0;
// static DWORD lasttime;
// static int lastgettime;
// int timeperframe;
//
// BOOL  ActiveApp; //Is this program active?
// char *video;   // Pointer to video memory
// char *screen; // Pointer to virtual screen
//
//
// //mouse
// mouse m;
//
// //commandline
// commandline *cline;

//message box printf
void winprintf(char *format, ...)
{
// char s[200];
//  va_list argptr;
//  va_start(argptr,format);
//  vsprintf(s,format,argptr);
//  va_end(argptr);
// MessageBox(hwnd,s,appname,MB_OK|MB_SETFOREGROUND);
}


char errstr[80];
void cleanexit(int x)
{
 // x=x;
 // DestroyWindow( hwnd);
}

//sets the video mode
#ifndef SDL2
int setddrawmode(int xw,int yw)
{
 // if (!DDO) return -1;
 //
 // //set video mode
 // HRESULT err;
 // if ((err=DDO->SetDisplayMode(xw,yw,8,0,0))!=DD_OK) return err;
 //
 // SCREENX=xw; SCREENY=yw;
 //
 // //clip cursor
 // RECT r;
 // r.left=0; r.right=SCREENX;
 // r.top=0; r.bottom=SCREENY;
 // ClipCursor(&r);
 // return 0;
 return -1;
}
#endif
//creates primary and backbuffer ddraw surfaces
int createsurfaces()
{
 // if (!DDO) return -1;
 //  //create surface(s)
 // DDSURFACEDESC   ddsd;
 // memset(&ddsd,0,sizeof(ddsd));
 // ddsd.dwSize = sizeof ( ddsd );
 // ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
 // ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
 // ddsd.dwBackBufferCount=2; //1;
 // HRESULT err;
 // if ((err=DDO->CreateSurface( &ddsd,&ddprimary, NULL ))!=DD_OK)
 //  {msg.error("Unable to create ddraw primary surface"); return -3;}
 //
 //  //get pointer to backbuffer
 // DDSCAPS ddscaps;
 // ddscaps.dwCaps=DDSCAPS_BACKBUFFER;
 // if (ddprimary->GetAttachedSurface(&ddscaps,&ddbackbuffer)!=DD_OK) return -4;
 //
 // //set palette
 // ddprimary->SetPalette(ddpalette);
 //
 // return 0;
 return -1;
}

void releasesurfaces()
{
 // if (ddbackbuffer) {ddbackbuffer->Release(); ddbackbuffer=0;}
 // if (ddprimary) {ddprimary->Release(); ddprimary=0;}
}

void terminateddraw();
//directdraw initialization/termination funcs
//returns <0 on failure
int initddraw(HWND hwnd)
{
 // HRESULT err;
 //
 // //create ddraw object
 // if (DirectDrawCreate(NULL,&DDO1,NULL)!=DD_OK ||
 //     DDO1->QueryInterface(IID_IDirectDraw2, (LPVOID *) &DDO)!=DD_OK)
 //   {
 //    msg.error("Unable to create DirectDraw object");
 //    return -1;
 //   }
 //
 //  //set cooperative mode
 // if ((err=DDO->SetCooperativeLevel( hwnd,
 //  DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_ALLOWMODEX|DDSCL_ALLOWREBOOT))!=DD_OK) return err;
 //
 // //set video mode
 // if (setddrawmode(SCREENX,SCREENY))
 //  {msg.error("Unable to set ddraw video mode"); return -1;}
 //
 // //create palette
 // DDO->CreatePalette(DDPCAPS_8BIT|DDPCAPS_ALLOW256,palentries,&ddpalette,0);
 //
 // //create surfaces
 // if (createsurfaces()) return -1;
 //
 // msg.printf(2,"DirectDraw initialized. %dx%dx%d",SCREENX,SCREENY,8);
 //
 // return 0;
 return -1;
}

void terminateddraw()
{
 // if (!DDO) return;
 // releasesurfaces();
 // if (ddpalette) {ddpalette->Release(); ddpalette=0;} //free palette
 //
 //   //free ddraw objects
 // DDO->RestoreDisplayMode();
 // DDO->Release(); DDO=0;
 // DDO1->Release(); DDO1=0;
 //
 // ClipCursor(0);
}

void setpalette(PALETTE *pal)
{
 // //copy from color format to palleteentry format
 // for (int i=0; i<256; i++)
 //  {
 //   palentries[i].peRed=pal->c[i].r;
 //   palentries[i].peGreen=pal->c[i].g;
 //   palentries[i].peBlue=pal->c[i].b;
 //   palentries[i].peFlags=0;
 //  }
 // ddpalette->SetEntries(0,0,256,palentries);
 // ddprimary->SetPalette(ddpalette);
}

//set individual palette index
#ifndef SDL2
void setpalettenum(int index,COLOR *c)
{
//  PALETTEENTRY p;
//  p.peRed=c->r;
//  p.peGreen=c->g;
//  p.peBlue=c->b;
//  p.peFlags=0;
//
//  ddpalette->SetEntries(0,index,1,&p);
// // msg.printf(1,"palentry[%d]=%d,%d,%d",index,c->r,c->g,c->b);
}
#endif


void ddrawinfo()
{
//  if (!DDO) return;
//  int totalmem,freemem;
//  DDSCAPS caps;
//  memset(&caps,0,sizeof(caps));
// // caps.dwCaps=DDSCAPS_SURFACE;
//
//  DDO->GetAvailableVidMem(&caps,(LPDWORD)&totalmem,(LPDWORD)&freemem);
//  msg.printf(2,"Total vid mem: %dK",totalmem/1024);
//  msg.printf(2,"Free vid mem: %dK",freemem/1024);
//
//  DDCAPS driver,hel;
//  memset(&driver,0,sizeof(DDCAPS));
//  driver.dwSize=sizeof(driver);
//  memset(&hel,0,sizeof(DDCAPS));
//  hel.dwSize=sizeof(hel);
//
//  DDO->GetCaps(&driver,&hel);
//  msg.printf(2,"Hardware BLT: %s",(driver.dwCaps&DDCAPS_BLT) ? "yes" : "no");
//  msg.printf(2,"Hardware color fill: %s",(driver.dwCaps&DDCAPS_BLTCOLORFILL) ? "yes" : "no");
//  msg.printf(2,"Bank Switched: %s",(driver.dwCaps&DDCAPS_BANKSWITCHED) ? "yes" : "no");
//
//  memset(&caps,0,sizeof(caps));
//  ddprimary->GetCaps(&caps);
//  msg.printf(2,"Primary surface: %s",(caps.dwCaps&DDSCAPS_VIDEOMEMORY) ? "video memory" : "system memory");
//
//  memset(&caps,0,sizeof(caps));
//  ddbackbuffer->GetCaps(&caps);
//  msg.printf(2,"Backbuffer surface: %s",(caps.dwCaps&DDSCAPS_VIDEOMEMORY) ? "video memory" : "system memory");
}


#ifndef SDL2
void changeresolution(int xw,int yw)
{
 // if (!DDO) return;
 //
 // releasesurfaces();
 // if (setddrawmode(xw,yw))
 //  {
 //  msg.error("%dx%d mode not supported",xw,yw);
 //   setddrawmode(SCREENX,SCREENY);
 //  }
 // createsurfaces();
 //
 // msg.printf(2,"DirectDraw mode set: %dx%dx%d",SCREENX,SCREENY,8);
}
#endif

//initialize direct sound
int initdsound(HWND hwnd)
{
 // HRESULT err;
 // //create directsound object
 // if ((err=DirectSoundCreate(NULL,&DSOUND,NULL))!=DS_OK)
 //   {DSOUND=0; return err;}
 //
 // DSOUND->SetCooperativeLevel(hwnd, DSSCL_EXCLUSIVE);
 //
 // //get caps
 // DSCAPS dscaps;
 // dscaps.dwSize=sizeof(dscaps);
 // if ((err=DSOUND->GetCaps(&dscaps))!=DS_OK)
 //  {DSOUND=0; return err;}
 //
 // // Set up primary sound wave format structure.
 // memset(&pcmwf, 0, sizeof(PCMWAVEFORMAT));
 // pcmwf.wf.wFormatTag = WAVE_FORMAT_PCM;
 // pcmwf.wf.nChannels = 2;
 // pcmwf.wf.nSamplesPerSec = SOUNDFREQ;
 // #ifdef DS8BIT
 // pcmwf.wf.nBlockAlign = 2;
 // pcmwf.wBitsPerSample =8;
 // #else
 // pcmwf.wf.nBlockAlign =4;
 // pcmwf.wBitsPerSample =16;
 // #endif
 // pcmwf.wf.nAvgBytesPerSec = pcmwf.wf.nSamplesPerSec * pcmwf.wf.nBlockAlign;
 //
 // //create primary sound buffer
 // DSBUFFERDESC dsbdesc;
 // memset(&dsbdesc, 0, sizeof(DSBUFFERDESC)); // Zero it out.
 // dsbdesc.dwSize = sizeof(DSBUFFERDESC);
 // dsbdesc.dwFlags = DSBCAPS_PRIMARYBUFFER;
 // dsbdesc.dwBufferBytes = 0;
 // dsbdesc.lpwfxFormat = NULL;
 // err = DSOUND->CreateSoundBuffer(&dsbdesc, &dsprimary, NULL);
 // if (err!=DS_OK)
 //  {msg.error("Unable to create primary sound buffer"); DSOUND=0; return -1;}
 // err=dsprimary->SetFormat((LPWAVEFORMATEX)&pcmwf); //set primary wave format
 // if (err!=DS_OK)
 //  {
 //   pcmwf.wf.nChannels = 1;
 //   pcmwf.wf.nBlockAlign =1;
 //   pcmwf.wBitsPerSample =8;
 //   pcmwf.wf.nAvgBytesPerSec = pcmwf.wf.nSamplesPerSec * pcmwf.wf.nBlockAlign;
 //   err=dsprimary->SetFormat((LPWAVEFORMATEX)&pcmwf); //set primary wave format
 //
 //   if (err!=DS_OK)
 //    {msg.error("Unable to set primary wave format");  return -1;}
 //  }
 //
 // dsprimary->Play(0,0,0);
 //
 // DWORD pcmsize;
 // dsprimary->GetFormat((LPWAVEFORMATEX)&pcmwf,sizeof(pcmwf),&pcmsize);
 //
 // msg.printf(2,"DirectSound initialized %dhz %dbit %s",pcmwf.wf.nSamplesPerSec,pcmwf.wBitsPerSample,
 //    pcmwf.wf.nChannels==1 ? "mono" : "stereo");
 //
 //
 //  // Set up wave format structure for secondary sounds.
 // memset(&pcmwf, 0, sizeof(PCMWAVEFORMAT));
 // pcmwf.wf.wFormatTag = WAVE_FORMAT_PCM;
 // pcmwf.wf.nChannels = 1;
 // pcmwf.wf.nSamplesPerSec = SOUNDFREQ;
 // pcmwf.wf.nBlockAlign = 1;
 // pcmwf.wf.nAvgBytesPerSec = pcmwf.wf.nSamplesPerSec * pcmwf.wf.nBlockAlign;
 // pcmwf.wBitsPerSample =8;
 // return 0;
 return -1;
}

void terminatedsound()
{
 // if (!DSOUND) return;
 // dsprimary->Release();
 // DSOUND->Release();
 // DSOUND=0;
}


 //creates a directsound buffer from existing old SOUND format
LPDIRECTSOUNDBUFFER createdsoundbuffer(struct SOUND *s)
{
  // if (!DSOUND) return 0;
  // HRESULT err;
  //
  // // Set up DSBUFFERDESC structure.
  // DSBUFFERDESC dsbdesc;
  // memset(&dsbdesc, 0, sizeof(DSBUFFERDESC)); // Zero it out.
  // dsbdesc.dwSize = sizeof(DSBUFFERDESC);
  // dsbdesc.dwFlags = DSBCAPS_CTRLPAN; //DSBCAPS_STATIC | DSBCAPS_CTRLPAN;
  // dsbdesc.dwBufferBytes = s->soundsize;
  // dsbdesc.lpwfxFormat = (LPWAVEFORMATEX)&pcmwf;
  //
  // // Create buffer.
  // LPDIRECTSOUNDBUFFER dsb;
  // err = DSOUND->CreateSoundBuffer(&dsbdesc, &dsb, NULL);
  // if (err!=DS_OK)  return 0;
  //
  // //lock memory
  // LPVOID b,b2;
  // DWORD size,size2;
  // err=dsb->Lock(0,s->soundsize,&b,&size,&b2,&size2,0);
  // for (int i=0; i<size; i++) ((char *)b)[i]=s->soundptr[i]+0x80;
  // dsb->Unlock(b,size,b2,size2);
  // return dsb;
  return -1;
}


void playsoundlooped(struct SOUND *s)
{
 // if (!DSOUND) return;
 // static LPDIRECTSOUNDBUFFER dsb=0;
 // if (dsb) freesound(dsb); dsb=0;
 //
 // dsb=createdsoundbuffer(s);
 // if (dsb) dsb->Play(0,0,DSBPLAY_LOOPING);
}

void playsound(struct IDirectSoundBuffer *dsb,int pan)
{
 // if (!DSOUND) return;
 // static LPDIRECTSOUNDBUFFER dscache[8]={0,0,0,0,0,0,0,0};
 //
 // //find free cache
 // for (int i=0; dscache[i] && i<8; i++);
 //
 // if (i==8)
 //  {
 //   //clear cache of stopped buffers
 //   for (i=0; i<8; i++)
 //    {
 //     DWORD status;
 //     dscache[i]->GetStatus(&status);
 //     if (!(status&DSBSTATUS_PLAYING))    //if not playing...
 //      {dscache[i]->Release(); dscache[i]=0;} //free it!
 //    }
 //   //find free cache (again)
 //   for (i=0; dscache[i] && i<8; i++);
 //   if (i==8) return; //still full, return
 //  }
 // //duplicate original sound
 // DSOUND->DuplicateSoundBuffer(dsb,&dscache[i]);
 // if (!dscache[i]) return;
 // if (pan>10000) pan=10000;
 // if (pan<-10000) pan=-10000;
 //
 // dscache[i]->SetPan(pan);
 // dscache[i]->Play(0,0,0);
}

void freesound(struct IDirectSoundBuffer *dsb)
{
 // if (!DSOUND) return;
 // dsb->Release();
}

int getsoundsize(struct IDirectSoundBuffer *dsb)
{
//  if (!dsb) return 0;
//  DSBCAPS caps;
//  memset(&caps,0,sizeof(caps));
//  caps.dwSize=sizeof(caps);
//  HRESULT err=dsb->GetCaps(&caps);
// // msg.printf(2,"soundsize=%d err=%X",caps.dwBufferBytes,err);
//  return caps.dwBufferBytes;
 return 0;
}


//TIMER STUFF
volatile int timerbusy=0,timerdisabled=0;

void disable() {timerdisabled=1;}
void enable() {timerdisabled=0;}

// void CALLBACK mmtimer(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1,DWORD dw2)
// {
//  if (timerbusy || timerdisabled || !ActiveApp) return;
//  timerbusy=1;
//  gametimer();
//  timerbusy=0;
// }


//----------------------------------------------------
//main initialization function
//initializes config, inputs, ddraw, dsound, timer
//----------------------------------------------------
void cleanup();

#ifndef SDL2
int initialize()
{
 // //initialize directdraw
 // int err;
 // if ((err=initddraw(hwnd))!=0)
 //    {
 //     cleanup();
 //     winprintf("Failure to initialize DirectDraw %X",err);
 //     return -1;
 //    }
 //
 // //initialize directsound
 // if (!getoption("nosound"))
 //  if ((err=initdsound(hwnd))!=0)
 //     msg.printf(5,"Failure to initialize DirectSound");
 //
 // #ifdef TIMERCALLBACK
 // //initialize timer
 // timerid=(UINT)timeSetEvent(1000/TIMERSPEED,5,mmtimer,0,TIME_PERIODIC);
 // if (!timerid)
 //     {
 //      cleanup();
 //      winprintf("Unable to initialize multimedia timer");
 //      return -1;
 //     }
 // #else
 // lastgettime=lasttime=timeGetTime();
 // #endif
 //
 // //Success!
 // return 0;
}

void cleanup()
{
 // terminategame();
 // terminatedsound();
 // terminateddraw();
 // if (timerid) {timeKillEvent(timerid); timerid=0;}
 // ShowCursor(TRUE);
}
#endif


//fills backbuffer with color
#ifndef SDL2
void drawrect(char *dest,int color,int x,int y,int xw,int yw)
{
// // dest=dest;
//
//   // Unlock the video memory.
//  ddbackbuffer->Unlock( NULL );
//
//  DDBLTFX bltfx;
//  bltfx.dwSize=sizeof(bltfx);
//  bltfx.dwFillColor=color;
//  RECT r;
//  r.left=(x>=0) ? x : 0; r.top=(y>=0) ? y : 0;
//  xw+=x; yw+=y;
//  r.right=(xw<SCREENX) ? xw : SCREENX;
//  r.bottom=(yw<SCREENY) ? yw : SCREENY;
//  ddbackbuffer->Blt(&r,NULL,NULL,DDBLT_COLORFILL|DDBLT_WAIT,&bltfx);
//
//  //relock video memory
//  DDSURFACEDESC  ddsd;
//  memset(&ddsd,0,sizeof(ddsd));
//  ddsd.dwSize = sizeof( ddsd );
//  ddbackbuffer->Lock( NULL, &ddsd, DDLOCK_WAIT, NULL );
}
#endif




extern int blah;
extern int fps; void fuck();
//Windows main func
// int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
//                     PSTR szCmdLine, int iCmdShow)
//   {
//    if (hPrevInstance) return -1;
//    hInst=hInstance;
//
//     //get configuration
//    cfg=new config();
//    cfg->load(configfile);
//
//     //process commandline
//    cline=new commandline(szCmdLine);
//    cline->execute(0);
//
//    //register window class
//    WNDCLASS  wndclass;
//    //wndclass.cbSize        = sizeof (wndclass) ;
//    wndclass.style         = 0;
//    wndclass.lpfnWndProc   = WndProc ;
//    wndclass.cbClsExtra    = 0 ;
//    wndclass.cbWndExtra    = 0 ;
//    wndclass.hInstance     = hInstance ;
//    wndclass.hIcon         = LoadIcon (NULL, IDI_APPLICATION) ;
//    wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW) ;
//    wndclass.hbrBackground = NULL;
//    wndclass.lpszMenuName  = NULL;
//    wndclass.lpszClassName = appname;
//    RegisterClass (&wndclass) ;
//
//    //create window for app
//    hwnd = CreateWindowEx(
//         WS_EX_TOPMOST,
//         appname, appname,
//         WS_POPUP, 0, 0,
//         GetSystemMetrics(SM_CXSCREEN),
//         GetSystemMetrics(SM_CYSCREEN),
//         NULL, NULL, hInstance, NULL );
//
//    if (!hwnd) return (FALSE);
//
//    UpdateWindow (hwnd) ;
//    SetFocus(hwnd);
//    ShowCursor( FALSE );
//
//     //initialze system shit
//    if (initialize()!=0) return FALSE;
//
//     //initialize game
//    if (initgame()!=0)
//     {
//      cleanup();
//      winprintf("Unable to initialize nes");
//      return -1;
//     };
//
//    cline->execute(1);
//
//    //Main loop
//    MSG  msg;
//    HRESULT lockerr;
//    int done=0;
//    while (!done)
//     {
//      m.reset();
//      if (!ActiveApp) WaitMessage();
//      while (PeekMessage( &msg, NULL, 0, 0,PM_REMOVE))
//         {
//          if (msg.message == WM_QUIT) {done=1; ActiveApp=0; break;}
//          TranslateMessage(&msg);
//          DispatchMessage(&msg);
//         }
//      #ifdef NETWORK
//      //update net connections
//      if (nc) nc->tick();
//      #endif
//
//      //refresh palette
//      if (nv) nv->refreshpalette();
//
//      if( ActiveApp)
//         {
//          if (!cfg->get(CFG_NOFILLEDDESKTOP))
//          {
//           //clear backbuffer
//          DDBLTFX bltfx;
//          bltfx.dwSize=sizeof(bltfx);
//          bltfx.dwFillColor=9*16+14;
//          ddbackbuffer->Blt(NULL,NULL,NULL,DDBLT_COLORFILL |DDBLT_WAIT,&bltfx);
//          }
//
//          #ifndef TIMERCALLBACK
//          {
//           timeperframe+=timeGetTime()-lastgettime; //ms
//           lastgettime=timeGetTime();
//          }
//          while (timeGetTime()>lasttime+(1000/TIMERSPEED))
//           {
//            gametimer();
//            lasttime+=(1000/TIMERSPEED);
//           }
//          #endif
//
//          //draw next frame
//          DDSURFACEDESC  ddsd;
//          memset(&ddsd,0,sizeof(ddsd));
//          ddsd.dwSize = sizeof( ddsd );
//
//          //Lock it
//          lockerr=ddbackbuffer->Lock( NULL, &ddsd, DDLOCK_WAIT, NULL );
//          if (lockerr==DD_OK)
//          {
//           screen=video=(char *)ddsd.lpSurface; //get pointer to video memory
//           PITCH=ddsd.lPitch; //set pitch
//
//           //draw next frame
//           updatescreen();
//           // Unlock the video memory.
//           ddbackbuffer->Unlock( NULL );
//
//           //Flip the surfaces
//          // if (ddsd.ddsCaps.dwCaps&DDSCAPS_MODEX)
//             ddprimary->Flip(NULL,0); //DDFLIP_WAIT);
//          //  else
//          //  {
//          //   RECT r={0,0,SCREENX,SCREENY};
//          //   ddprimary->BltFast(0,0,ddbackbuffer,&r,DDBLTFAST_NOCOLORKEY|DDBLTFAST_WAIT);
//          //  }
//          } else
//          if (lockerr==DDERR_SURFACELOST)
//           {
//            ddprimary->Restore();
//            surfacelost++;
//           }
//           else
//           break; //error locking surface
//         }
//
//     }
//
//
//
//  //cleanup everything
//  cleanup();
//  if (lockerr!=DD_OK) winprintf("DDraw Lock error: %X",lockerr);
//  return msg.wParam;
// }

#ifndef SDL2
void quitgame()
{
 // DestroyWindow( hwnd );
 // ActiveApp=0;
}
#endif

// LRESULT CALLBACK WndProc (HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
//      {
//      HDC         hdc ;
//
//      switch (iMsg)
//           {
//       case WM_ACTIVATEAPP:
//             ActiveApp =wParam;
//             if (ActiveApp) lastgettime=lasttime=timeGetTime(); //reset timer
//            break;
//
//       case WM_KEYDOWN:
// //         if (lParam&(1<<30)) break; //it was previously down
//          wm_keydown((lParam>>16)&0xFF);
//         break;
//
//       case WM_KEYUP:
//          wm_keyup((lParam>>16)&0xFF);
//          break;
//
//       case WM_MOUSEMOVE:   //update mouse coords
//           m.updatexy(LOWORD(lParam),HIWORD(lParam));
//          return 0;
//
//       case WM_LBUTTONDOWN:
//       case WM_LBUTTONUP:
//       case WM_RBUTTONDOWN:
//       case WM_RBUTTONUP:
//         {
//          int mb=0;  //update mouse buttons
//          if (wParam&MK_LBUTTON) mb|=1;
//          if (wParam&MK_RBUTTON) mb|=2;
//          m.updatebut(mb);
//         }
//        return 0;
//
//   /*
//       case WM_TCPSOCKET:
//         if (!WSAGETSELECTERROR(lParam))
//              wm_tcpsocket(wParam,WSAGETSELECTEVENT(lParam));
//         else wm_socketerror(wParam,WSAGETSELECTEVENT(lParam),WSAGETSELECTERROR(lParam));
//        break;
// */
//  #ifdef NETWORK
//       case WM_UDPSOCKET:
//         if (!WSAGETSELECTERROR(lParam))
//              wm_udpsocket(wParam,WSAGETSELECTEVENT(lParam));
//         else wm_socketerror(wParam,WSAGETSELECTEVENT(lParam),WSAGETSELECTERROR(lParam));
//        break;
//  #endif
//  /*    case WM_SERVERSOCKET:
//         if (!WSAGETSELECTERROR(lParam))
//              wm_serversocket(wParam,WSAGETSELECTEVENT(lParam));
//         else wm_socketerror(wParam,WSAGETSELECTEVENT(lParam),WSAGETSELECTERROR(lParam));
//        break;
//     */
//       case WM_CREATE :
//        break;
//       case WM_DESTROY :
// //       closeMusic();
//        PostQuitMessage (0) ;
//        if (timerid) {timeKillEvent(timerid); timerid=0;}
//        ::hwnd=0;
//       break;
//
//       }
//
//
//    return DefWindowProc (hwnd, iMsg, wParam, lParam) ;
// }

  /*
//loads a resource from the module
void *loadresource(char *name)
{
HRSRC r=FindResource(hInst,name,RT_RCDATA);
if (!r)  return 0;
HGLOBAL h=LoadResource(hInst,r);
if (!h)  return 0; //failure
//return (void *)h;

//allocate memory to copy it
int size=SizeofResource(hInst,r);       //get size
char *t=(char *)malloc(size); //get memory
if (!t) return 0;
memcpy(t,h,size); //copy it
return t;
}
*/




//----------------------------
//directdraw surface wrapper
surface::surface(int txw,int tyw)
{
//  xw=txw; yw=tyw;
//  //create surface
//  DDSURFACEDESC   ddsd;
//  memset(&ddsd,0,sizeof(ddsd));
//  ddsd.dwSize = sizeof ( ddsd );
//  ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PITCH;
//  ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
//  ddsd.dwWidth = xw;
//  ddsd.dwHeight = yw;
//  ddsd.lPitch=xw;
//  HRESULT err= DDO->CreateSurface(&ddsd,&dds, NULL );
//  if (err!=DD_OK) dds=0;
//  else
//  {
//   DDSCAPS caps;
//   dds->GetCaps(&caps);
// //  msg.printf(2,"surface created (%d,%d) %s",ddsd.dwWidth,ddsd.dwHeight,
// //  (caps.dwCaps&DDSCAPS_VIDEOMEMORY) ? "vidmem" : "sysmem");
//  }
}

surface::~surface()
{
//  if (dds) dds->Release();
// // msg.printf(2,"surface freed");
}

char *surface::lock()
{
//  if (!dds) return 0;
//  DDSURFACEDESC  ddsd;
//  memset(&ddsd,0,sizeof(ddsd));
//  ddsd.dwSize = sizeof( ddsd );
//  HRESULT lockerr=dds->Lock( NULL, &ddsd, DDLOCK_WAIT, NULL );
//  if (lockerr==DDERR_SURFACELOST)
//   {
//    dds->Restore();
//    lockerr=dds->Lock( NULL, &ddsd, DDLOCK_WAIT, NULL );
//    if (lockerr!=DD_OK) return 0;
// //   msg.printf(2,"surface restored",ddsd.dwWidth,ddsd.dwHeight);
//   }
//
//  oldscrx=SCREENX; oldscry=SCREENY; oldpitch=PITCH;
//  SCREENX=xw; SCREENY=yw; PITCH=ddsd.lPitch;
// // msg.printf(2,"lock %d,%d %d %p",ddsd.dwWidth,ddsd.dwHeight,PITCH,ddsd.lpSurface);
//
//  return (char *)ddsd.lpSurface;
return 0;
}


void surface::unlock()
{
//  if (!dds) return;
//
//  dds->Unlock(0);
//  SCREENX=oldscrx; SCREENY=oldscry; PITCH=oldpitch;
// }
//
// int surface::blt(char *dest, int x,int y)
// {
//  if (!dds) return 0;
//  // Unlock the video memory.
//  ddbackbuffer->Unlock( NULL );
//
// // RECT r={0,0,xw,yw};
//  HRESULT err=ddbackbuffer->BltFast(x,y,dds,0,DDBLTFAST_NOCOLORKEY|DDBLTFAST_WAIT);
// // if (err!=0) msg.printf(2,"%X",err);
//
//  //relock video memory
//  DDSURFACEDESC  ddsd;
//  memset(&ddsd,0,sizeof(ddsd));
//  ddsd.dwSize = sizeof( ddsd );
//  ddbackbuffer->Lock( NULL, &ddsd, DDLOCK_WAIT, NULL );
//
//  return err==DD_OK;
}



//---------------------------------
//clipboard paste

char *getclipboardtext()
{
 // if (!IsClipboardFormatAvailable(CF_TEXT)) return 0;
 // OpenClipboard(hwnd);
 // HGLOBAL h=GetClipboardData(CF_TEXT);
 // if (!h) return 0;
 // char *s=(char *)malloc(GlobalSize(h));
 // strcpy(s,(char *)GlobalLock(h));
 // GlobalUnlock(h);
 // CloseClipboard();
 // return s;
 return 0;
}


/*
DWORD playMIDIFile(HWND hWndNotify, LPSTR lpszMIDIFileName)
{
    UINT wDeviceID;
    DWORD dwReturn;
    MCI_OPEN_PARMS mciOpenParms;
    MCI_PLAY_PARMS mciPlayParms;
    MCI_STATUS_PARMS mciStatusParms;
    MCI_SEQ_SET_PARMS mciSeqSetParms;

    // Open the device by specifying the device and filename.
    // MCI will attempt to choose the MIDI mapper as the output port.
    mciOpenParms.lpstrDeviceType = "sequencer";
    mciOpenParms.lpstrElementName = lpszMIDIFileName;
    if ((dwReturn = mciSendCommand(NULL, MCI_OPEN,
        MCI_OPEN_TYPE | MCI_OPEN_ELEMENT,
        (DWORD)(LPVOID) &mciOpenParms))!=0)
    {
        // Failed to open device. Don't close it; just return error.
        return (dwReturn);
    }

    // The device opened successfully; get the device ID.
    wDeviceID = mciOpenParms.wDeviceID;

    // Check if the output port is the MIDI mapper.
    mciStatusParms.dwItem = MCI_SEQ_STATUS_PORT;
    if
    (
    (dwReturn = mciSendCommand(wDeviceID, MCI_STATUS,MCI_STATUS_ITEM, (DWORD)(LPVOID) &mciStatusParms))!=0
    )
    {
        mciSendCommand(wDeviceID, MCI_CLOSE, 0, NULL);
        return (dwReturn);
    }

    // Begin playback. The window procedure function for the parent
    // window will be notified with an MM_MCINOTIFY message when
    // playback is complete. At this time, the window procedure closes
    // the device.
    mciPlayParms.dwCallback = (DWORD) hWndNotify;
    if ((dwReturn = mciSendCommand(wDeviceID, MCI_PLAY, MCI_NOTIFY,
        (DWORD)(LPVOID) &mciPlayParms))!=0)
    {
        mciSendCommand(wDeviceID, MCI_CLOSE, 0, NULL);
        return (dwReturn);
    }

    return (0L);
}
*/



#ifdef __WATCOMC__
int random(int x)
{
 return rand()*x/32768;
}
#endif
