//CPU handler  starts/stops cpu etc

//routines for handling roms

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
// #include <process.h>

#include "dd.h"

#include "types.h"

#include "message.h"
#include "file.h"

#include "keyb.h"

//#include "6502.h"
#include "m6502.h"

#include "nes.h"

byte CPUpaused=0; //flag for when cpu is paused
byte CPURunning=0; //flag for when cpu is running

int  volatile cycles;

void startframe();
void startvblank();

#include "timing.h"

extern volatile byte frame;

//are we currently in a virtual frame ? (emulating)
volatile byte inemu=0;

//get current virtual scanline being draw
int getscanline()
{
 if (frame) return m6502clockticks*232/FRAMECYCLES; //during frame (assume 232 lines pre-vblank)
      else return 0; //0 during vblank
}


//ticked (once per virtual frame)
void tickemu()
{
 if (!CPURunning) return;
 if (inemu) return;
 inemu=1;


 if (ram[0x2000]&0x80) //generate vblank?
   m6502nmi();

 //execute during vblank
 startvblank();
 m6502exec(VBLANKCYCLES);


 //execute during virtual frame...
 m6502clockticks=0; //reset clock ticks
 startframe();
 m6502exec(FRAMECYCLES);

 //done
 cycles+=CYCLESPERTICK;
 inemu=0;
}


void m_execute()
{
 if (!romloaded) {msg.error("ROM not loaded"); return;}
 if (CPURunning) {msg.error("CPU already running"); return;}
 if (resetNEShardware()!=0)
   { msg.error("Unable to reset NES hardware"); return;}

 msg.printf(3,"CPU emulation started");
 CPURunning=1;
 CPUpaused=0;
}

void m_stop()
{
 if (!romloaded) return;
 CPUpaused=0;
 if (CPURunning) msg.printf(3,"CPU emulation stopped");
 CPURunning=0;
}

void m_resume()
{
 if (!romloaded) {msg.error("ROM not loaded"); return;}
 if (CPURunning) {msg.error("CPU already running"); return;}
 if (!CPUpaused) {msg.error("CPU not paused"); return;}

 msg.printf(3,"CPU emulation resumed");
 CPURunning=1;
 CPUpaused=0;
}

void m_pause()
{
 if (!romloaded) return;
 if (!CPURunning) {msg.error("CPU not running"); return;}
 CPUpaused=1;
 CPURunning=0;
 msg.printf(3,"CPU emulation paused");
}

//-----------------------------------------------------------
//-----------------------------------------------------------
