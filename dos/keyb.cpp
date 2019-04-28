#include <stdlib.h>
#include <dos.h>
#include <i86.h>
#include <conio.h>
#include <ctype.h>


#include "keyb.h"


void loades();
#pragma  aux loades =  \
  "push ds"            \
  "pop  es"               \
  parm   []               \
  modify [];


volatile char kbint=0;
volatile char kbscan=0;
volatile char kbstat=0; 
char oldkbscan=0;

int keyboardinstalled=0;
void (interrupt far *oldkeybintvector)(void); //pointer to old bios vector
int  (*keyboardhandler)(void)=0;  //pointer to keyboard handler

int keyqueue=1; //whether or not to store keys in the queue
int keybios=1;  //whether or not to just call old bios
int keyrepeat=1; //whether or not to repeat

int checkkbstat();

volatile char keydown[128];

void interrupt __loadds KEYBOARDIRQ()
{
// outp(0x20,0x20);
// return;  
 if (keybios)
   {
    oldkeybintvector(); //call old bios
    return;
   }
    
 kbscan=inp(0x60);
 if ((!keyrepeat && oldkbscan==kbscan))// || kbscan==42)
  {outp(0x20,0x20); return;} //no repeat

 oldkbscan=kbscan;

 loades();    
 _enable();
 kbint=1;

 //store keydown status
 keydown[kbscan&0x7f]=(kbscan&0x80)^0x80;

 if (!checkkbstat()) //check shift/alt/ctrl
 {
  if (keyboardhandler)  //call handler
   { //0=add key to cache, 1=dont add key to cache
//     if (!(*keyboardhandler)() && keyqueue) //if key should be added
        pushkey(kbscan); //push key
   }
  else if (keyqueue) pushkey(kbscan); //push key
 }

 kbint=0;
 _disable();

 outp(0x20,0x20);
}


volatile char kbscanbuf[16];
volatile int  kbhead=0,kbtail=0;

int checkkbstat()
{
register char k=kbscan&0x7f;
 if (k==42 || k==54) {if (kbscan&0x80) kbstat&=~KB_SHIFT; else kbstat|=KB_SHIFT; return 1;}
 if (k==29)  {if (kbscan&0x80) kbstat&=~KB_CTRL; else kbstat|=KB_CTRL; return 1;}
 if (k==56)  {if (kbscan&0x80) kbstat&=~KB_ALT;  else kbstat|=KB_ALT; return 1;}
return 0;
}    

//stores a scan code in the buffer
void pushkey(char kbscan)
{
// if (kbscan&0x80) return;
 kbscanbuf[kbtail]=kbscan; //store at tail
 kbtail++; kbtail&=15;
}


//gets next scan code from buffer
char getkey()
{
if (kbhead==kbtail) return 0;

char kb=kbscanbuf[kbhead];
kbhead++; kbhead&=15;
return kb;
}

int keyhit()
{
return (kbtail!=kbhead);
}    

//waits for next scan code from buffer
char waitkey()
{
while (!keyhit());

char kb=kbscanbuf[kbhead];
kbhead++; kbhead&=15;
return kb;
}



void init_keyboard()
{
 if (keyboardinstalled) return;
 keybios=1;
 _disable();
 oldkeybintvector = _dos_getvect(9);
 _dos_setvect(9, KEYBOARDIRQ);
 _enable();

 keyboardinstalled=1; 
 atexit(terminate_keyboard);
}
void terminate_keyboard()
{
 if (!keyboardinstalled) return;    

 _disable();
 _dos_setvect(9, oldkeybintvector);
 _enable();
 keyboardinstalled=0;
}    


void set_keyboard_func(int (*kfunc)())
{
 keyboardhandler=kfunc;
}



char s2a[]=
{
0,27,49,50,51,52,53,54,55,56,57,48,45,61,8,9,113,119,101,114,116,121,117,105,111,
112,91,93,13,0,97,115,100,102,103,104,106,107,108,59,39,96,0,92,122,120,99,118, //47
98,110,109,44,46,47,0,42,0,32,128,129,130,131,132,133,134,135,136,137,0,0,0,0,0,45, //74
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};    

char s2ashift[]=
{
0,27,33,64,35,36,37,94,38,42,40,41,95,43,8,9, 81, 87, 69, 82, 84, 89, 85, 73, 79,
 80,123,125,13,0,65, 83, 68, 70, 71, 72, 74, 75, 76,58,34,126,0,124,90, 88,67, 86, //47
66, 78, 77,60,62,63,0,42,0,32,128,129,130,131,132,133,134,135,136,137,0,0,0,0,0,45, //74
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};    

//converts a scancode to ascii character
char scan2ascii(char s)
{
 if (kbstat&KB_SHIFT)  return s2ashift[s];
 return s2a[s];
}



struct keyname {
    char scancode;
    char *name;
};
keyname keynames[]=
{
  {0x48, "Up"},
  {0x50, "Down"},
  {0x4B, "Left"},
  {0x4D, "Right"},
  {0x47, "Home"},
  {0x4F, "End"},
  {0x49, "Page Up"},
  {0x51, "Page Down"},
  {0x52, "Insert"},
  {0x53, "Delete"},
  {0xE0, "\\"},
  {0x37, "*"},
  {0x4A, "-"},
  {0x4E, "+"},

  {0x29, "`"},
  {0x02, "1"},
  {0x03, "2"},
  {0x04, "3"},
  {0x05, "4"},
  {0x06, "5"},
  {0x07, "6"},
  {0x08, "7"},
  {0x09, "8"},
  {0x0A, "9"},
  {0x0B, "0"},
  {0x0C, "-"},
  {0x0D, "-"},

  {0x1a, "["},
  {0x1b, "]"},
  {0x27, ";"},
  {0x28, "'"},
  {0x2b, "\\"},
  {0x33, ","},
  {0x34, "."},
  {0x35, "/"},

  {0x10, "q"},
  {0x11, "w"},
  {0x12, "e"},
  {0x13, "r"},
  {0x14, "t"},
  {0x15, "y"},
  {0x16, "u"},
  {0x17, "i"},
  {0x18, "o"},
  {0x19, "p"},

  {0x1E, "a"},
  {0x1F, "s"},
  {0x20, "d"},
  {0x21, "f"},
  {0x22, "g"},
  {0x23, "h"},
  {0x24, "j"},
  {0x25, "k"},
  {0x26, "l"},

  {0x2c, "z"},
  {0x2d, "x"},
  {0x2e, "c"},
  {0x2f, "v"},
  {0x30, "b"},
  {0x31, "n"},
  {0x32, "m"},

  {0,"<none>"}
};

char *getkeyname(char key)
{
for (int i=0; keynames[i].scancode; i++)
 if (keynames[i].scancode==key) break;

return(keynames[i].name);
}






