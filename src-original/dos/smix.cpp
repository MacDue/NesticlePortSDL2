/*      SMIXW is Copyright 1995 by Ethan Brodsky.  All rights reserved      */

/* € smix.c v1.24 €€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€ */

extern "C" {
void __cdecl loades();
void __cdecl SILENCEASM(signed short *x,unsigned int num);
void __cdecl COPYSOUND8(unsigned char *,signed short *,unsigned char *);
void __cdecl MIXASM(signed short *,signed char *,int num, signed short *vol);       
void __cdecl COPYASM(signed short *,signed  char *,int num, signed short *vol);       
};

#include "..\sound.h"
#include "smix.h"

void *low_malloc(int size, short int *sel);
void low_free(short int sel);


#define BLOCK_LENGTH    512   /* Length of digitized sound output block     */
#define VOICES          6     /* Number of available simultaneous voices    */
#define VOLUMES         2    /* Number of volume levels for sound output   */

volatile long intcount;               /* Current count of sound interrupts  */
volatile int  voicecount;             /* Number of voices currently in use  */

int dspversion;
int   autoinit;
int   sixteenbit;

/* €€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€ */

#include <dos.h>
#include <i86.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>



#define BUFFER_LENGTH BLOCK_LENGTH*2

#define BYTE unsigned char

#define lo(value) (unsigned char)((value) & 0x00FF)
#define hi(value) (unsigned char)((value) >> 8)

#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) > (b)) ? (b) : (a))

static int resetport;
static int readport;
static int writeport;
static int pollport;
static int ackport;

static int pic_rotateport;
static int pic_maskport;

static int dma_maskport;
static int dma_clrptrport;
static int dma_modeport;
static int dma_addrport;
static int dma_countport;
static int dma_pageport;

static char irq_startmask;
static char irq_stopmask;
static char irq_intvector;

static char dma_startmask;
static char dma_stopmask;
static char dma_mode;

static void (interrupt far *oldintvector)(void);

static int handler_installed;

static void write_dsp(BYTE value)
  {
    while ((inp(writeport) & 0x80));
    outp(writeport, value);
  }

static BYTE read_dsp(void)
  {
    while (!(inp(pollport) & 0x80));
    return(inp(readport));
  }

static int reset_dsp(void)
  {
    int i;

    outp(resetport, 1);
//    for (i=0; i < 300; i++)    /* The delay function doesn't work correctly */
//      { };
    delay(300);
    outp(resetport, 0);

    i = 100;
    while ((i-- > 0) && (read_dsp() != 0xAA));

    return(i > 0);
  }

void install_handler(void);
void uninstall_handler(void);
void smix_exitproc(void);

int init_sb(int baseio, int irq, int dma, int dma16)
  {
   /* Sound card IO ports */
    resetport  = baseio + 0x006;
    readport   = baseio + 0x00A;
    writeport  = baseio + 0x00C;
    pollport   = baseio + 0x00E;


   /* Reset DSP, get version, choose output mode */
    _disable();
    if (!reset_dsp())
      return(FALSE);
    write_dsp(0xE1);  /* Get DSP version number */
    dspversion = read_dsp()*100;  dspversion += read_dsp();
    autoinit   = (dspversion > 200);
    sixteenbit = (dspversion > 400) && (dma16 != 0) && (dma16 > 3);
    _enable();

//    cprintf("DSP version %d.%02d...",dspversion/100,dspversion%100);

   /* Compute interrupt controller ports and parameters */
    if (irq < 8)
      { /* PIC1 */
        irq_intvector  = 0x08 + irq;
        pic_rotateport = 0x20;
        pic_maskport   = 0x21;
      }
    else
      { /* PIC2 */
        irq_intvector  = 0x70 + irq-8;
        pic_rotateport = 0xA0;
        pic_maskport   = 0xA1;
      }
    irq_stopmask  = 1 << (irq % 8);
    irq_startmask = ~irq_stopmask;

   /* Compute DMA controller ports and parameters */
    if (sixteenbit)
      { /* Sixteen bit */
//          cprintf("16-bit DAC...");
        dma_maskport   = 0xD4;
        dma_clrptrport = 0xD8;
        dma_modeport   = 0xD6;
        dma_addrport   = 0xC0 + 4*(dma16-4);
        dma_countport  = 0xC2 + 4*(dma16-4);
        switch (dma16)
          {
            case 5:
              dma_pageport = 0x8B;
              break;
            case 6:
              dma_pageport = 0x89;
              break;
            case 7:
              dma_pageport = 0x8A;
              break;
          }
        dma_stopmask  = dma16-4 + 0x04;  /* 000001xx */
        dma_startmask = dma16-4 + 0x00;  /* 000000xx */
        if (autoinit)
          dma_mode = dma16-4 + 0x58;     /* 010110xx */
        else
          dma_mode = dma16-4 + 0x48;     /* 010010xx */
        ackport = baseio + 0x00F;
      }
    else
      { /* Eight bit */
//        cprintf("8-bit DAC...");          
        dma_maskport   = 0x0A;
        dma_clrptrport = 0x0C;
        dma_modeport   = 0x0B;
        dma_addrport   = 0x00 + 2*dma;
        dma_countport  = 0x01 + 2*dma;
        switch (dma)
          {
            case 0:
              dma_pageport = 0x87;
              break;
            case 1:
              dma_pageport = 0x83;
              break;
            case 2:
              dma_pageport = 0x81;
              break;
            case 3:
              dma_pageport = 0x82;
              break;
          }
        dma_stopmask  = dma + 0x04;      /* 000001xx */
        dma_startmask = dma + 0x00;      /* 000000xx */
        if (autoinit)
          dma_mode    = dma + 0x58;      /* 010110xx */
        else
          dma_mode    = dma + 0x48;      /* 010010xx */
        ackport = baseio + 0x00E;
      }
    install_handler();
    atexit(smix_exitproc);

    return(TRUE);
  }

void shutdown_sb(void)
  {
    if (handler_installed) uninstall_handler();
    reset_dsp();
  }

/* Voice control */

typedef struct
  {
    SOUND *sound;
    int   index;
    int   volume;
    int   loop;
    long  curpos;
    int   done;
  } VOICE;

static int   inuse[VOICES];
static VOICE voice[VOICES];

static int curblock;

/* Volume lookup table */
static short (*volume_table)[VOLUMES][256];

/* Mixing buffer */
static signed short int  mixingblock[BLOCK_LENGTH];  /* Signed 16 bit */

/* Output buffers */
static unsigned char       (*outmemarea)                = NULL;
static unsigned char (*out8buf)[2][BLOCK_LENGTH]  = NULL;
static signed  short (*out16buf)[2][BLOCK_LENGTH] = NULL;

static void *blockptr[2];

static short int outmemarea_sel;              /* Selector for output buffer */

/* Addressing for auto-initialized transfers (Whole buffer)   */
static unsigned long buffer_addr;
static unsigned char buffer_page;
static unsigned int  buffer_ofs;

/* Addressing for single-cycle transfers (One block at a time */
static unsigned long block_addr[2];
static unsigned char block_page[2];
static unsigned int  block_ofs[2];


static unsigned char sound_volume;

/* 8-bit clipping */

static unsigned char *clip_8_buf;
static unsigned char *clip_8;

void __cdecl start_dac(void)
  {
    _disable();
    if (autoinit)
      { /* Auto init DMA */
        outp(dma_maskport,   dma_stopmask);
        outp(dma_clrptrport, 0x00);
        outp(dma_modeport,   dma_mode);
        outp(dma_addrport,   lo(buffer_ofs));
        outp(dma_addrport,   hi(buffer_ofs));
        outp(dma_countport,  lo(BUFFER_LENGTH-1));
        outp(dma_countport,  hi(BUFFER_LENGTH-1));
        outp(dma_pageport,   buffer_page);
        outp(dma_maskport,   dma_startmask);
      }
    else
      { /* Single cycle DMA */
        outp(dma_maskport,   dma_stopmask);
        outp(dma_clrptrport, 0x00);
        outp(dma_modeport,   dma_mode);
        outp(dma_addrport,   lo(buffer_ofs));
        outp(dma_addrport,   hi(buffer_ofs));
        outp(dma_countport,  lo(BLOCK_LENGTH-1));
        outp(dma_countport,  hi(BLOCK_LENGTH-1));
        outp(dma_pageport,   buffer_page);
        outp(dma_maskport,   dma_startmask);
      }

    if (sixteenbit)
      { /* Sixteen bit auto-initialized: SB16 and up (DSP 4.xx)             */
        write_dsp(0x41);                /* Set sound output sampling rate   */
        write_dsp(hi(SOUNDFREQ));
        write_dsp(lo(SOUNDFREQ));
        write_dsp(0xB6);                /* 16-bit cmd  - D/A - A/I - FIFO   */
        write_dsp(0x10);                /* 16-bit mode - signed mono        */
        write_dsp(lo(BLOCK_LENGTH-1));
        write_dsp(hi(BLOCK_LENGTH-1));
      }
    else
      { /* Eight bit */
        if (autoinit)
          { /* Eight bit auto-initialized:  SBPro and up (DSP 2.00+)        */
            write_dsp(0xD1);            /* Turn on speaker                  */
            write_dsp(0x40);            /* Set sound output time constant   */
            write_dsp(256 - (1000000 / SOUNDFREQ));             /*  = 256 - (1000000 / rate)        */
            write_dsp(0x48);            /* Set DSP block transfer size      */
            write_dsp(lo(BLOCK_LENGTH-1));
            write_dsp(hi(BLOCK_LENGTH-1));
            write_dsp(0x1C);            /* 8-bit auto-init DMA mono output  */
          }
        else
          { /* Eight bit single-cycle:  Sound Blaster (DSP 1.xx+)           */
            write_dsp(0xD1);            /* Turn on speaker                  */
            write_dsp(0x40);            /* Set sound output time constant   */
            write_dsp(256 - (1000000 / SOUNDFREQ));             /*  = 256 - (1000000 / rate)        */
            write_dsp(0x14);            /* 8-bit single-cycle DMA output    */
            write_dsp(lo(BLOCK_LENGTH-1));
            write_dsp(hi(BLOCK_LENGTH-1));
          }
      }

    _enable();  
  }

static void stop_dac(void)
  {
   _disable();

    if (sixteenbit)
      write_dsp(0xD5);                  /* Pause 16-bit DMA sound I/O       */
    else
      {
        write_dsp(0xD0);                /* Pause 8-bit DMA sound I/O        */
      }
        write_dsp(0xD3);                /* Turn off speaker                 */

    outp(dma_maskport, dma_stopmask);   /* Stop DMA                         */
   _enable(); 
  }

/* Volume control */

static void init_volume_table(void)
  {
    signed  int  volume;
    signed   int  insample;
    signed   char invalue;

    volume_table = (short (*)[VOLUMES][256])malloc(VOLUMES * 256 * sizeof(signed short int));

    for (volume=0; volume < VOLUMES; volume++)
      for (insample = -128; insample <= 127; insample++)
        {
          invalue = insample;
          (*volume_table)[volume][(unsigned char)invalue] =
//            (((float)volume/(float)(VOLUMES-1)) * 32 * invalue);
            (volume  * 75 * invalue  /(VOLUMES-1));
  
        }

    sound_volume = 255;
  }

void set_sound_volume(unsigned char new_volume)
  {
    sound_volume = new_volume;
  }

/* Mixing initialization */

static void init_clip8(void)
  {
    int i;
    int value;

    clip_8_buf =(unsigned char *) malloc(256*VOICES);
    clip_8     =(unsigned char *)clip_8_buf + 128*VOICES;

    for (i = -128*VOICES; i < 128*VOICES; i++)
      {
        value = i;
        value = max(value, -128);
        value = min(value, 127);

        clip_8[i] = value + 128;
      }
  }

static unsigned long linear_addr(void *ptr)
  {
    return((unsigned long)(ptr));
  }

void deallocate_voice(int voicenum);

void init_mixing(void)
  {
    int i;

    for (i=0; i < VOICES; i++)
      deallocate_voice(i);
    voicecount = 0;

    if (sixteenbit)
      {
       /* Find a block of memory that does not cross a page boundary */
       do
       {
        outmemarea = (unsigned char *)low_malloc(4*BUFFER_LENGTH, &outmemarea_sel);
        out16buf = (signed short int (*)[2][BLOCK_LENGTH]) outmemarea;

       } while ((((linear_addr(out16buf) >> 1) % 65536) + BUFFER_LENGTH) > 65536);

        for (i=0; i<2; i++)
          blockptr[i] = &((*out16buf)[i]);

       /* DMA parameters */
        buffer_addr = linear_addr(out16buf);
        buffer_page = buffer_addr        / 65536;
        buffer_ofs  = (buffer_addr >> 1) % 65536;

        memset(out16buf, 0x00, BUFFER_LENGTH * sizeof(signed short));
      }
    else
      {
       /* Find a block of memory that does not cross a page boundary */
       do
       {
        outmemarea = (unsigned char *) low_malloc(2*BUFFER_LENGTH, &outmemarea_sel);
        out8buf =(unsigned char (*)[2][BLOCK_LENGTH]) outmemarea;
       } while (((linear_addr(out8buf) % 65536) + BUFFER_LENGTH) > 65536);
//             ((char *)out8buf) += BUFFER_LENGTH;

        for (i=0; i<2; i++)
          blockptr[i] = &((*out8buf)[i]);
//printf("initdma\n");

       /* DMA parameters */
        buffer_addr = linear_addr(out8buf);
        buffer_page = buffer_addr / 65536;
        buffer_ofs  = buffer_addr % 65536;
//printf("initfuck\n");

        for (i=0; i<2; i++)
          {
            block_addr[i] = linear_addr(blockptr[i]);
            block_page[i] = block_addr[i] / 65536;
            block_ofs[i]  = block_addr[i] % 65536;
          }
//printf("initfuck2 %X %X %X\n",outmemarea,out8buf,BUFFER_LENGTH * sizeof(unsigned char));

        memset(out8buf, 0x80, BUFFER_LENGTH * sizeof(unsigned char));
//printf("initclip8\n");

        init_clip8();

      }

    curblock = 0;
    intcount = 0;
//printf("initvoltable\n");
    init_volume_table();
//printf("startdac\n");
    start_dac();
//printf("done\n");
    
/*
   while (kbhit()) getch();
   printf("\n");
    do
    {
        printf("DMACOUNT %5d\r",inp(dma_countport));
    } while (!kbhit());
*/
 }

void shutdown_mixing(void)
  {
    stop_dac();

    free(volume_table);

    if (!sixteenbit) free(clip_8_buf);

    low_free(outmemarea_sel);
  }


/* Voice maintainance */

static void deallocate_voice(int voicenum)
  {
    inuse[voicenum] = FALSE;
    voice[voicenum].sound  = NULL;
    voice[voicenum].index  = -1;
    voice[voicenum].volume = 0;
    voice[voicenum].curpos = -1;
    voice[voicenum].loop   = FALSE;
    voice[voicenum].done   = FALSE;
  }

void start_sound(SOUND *sound, int index, unsigned char volume, int loop)
  {
    int i, voicenum;

    voicenum = -1;
    i = 0;

    do
      {
        if (!inuse[i])
          voicenum = i;
        i++;
      }
    while ((voicenum == -1) && (i < VOICES));

    if (voicenum != -1)
      {
        voice[voicenum].sound  = sound;
        voice[voicenum].index  = index;
        voice[voicenum].volume = volume;
        voice[voicenum].curpos = 0;
        voice[voicenum].loop   = loop;
        voice[voicenum].done   = FALSE;

        inuse[voicenum] = TRUE;
        voicecount++;
      }
  }

void stop_sound(int index)
  {
    int i;

    for (i=0; i < VOICES; i++)
      if (voice[i].index == index)
        {
          voicecount--;
          deallocate_voice(i);
        }
  }

int  sound_playing(int index)
  {
    int i;

   /* Search for a sound with the specified index */
    for (i=0; i < VOICES; i++)
      if (voice[i].index == index)
        return(TRUE);

   /* Sound not found */
    return(FALSE);
  }

static void update_voices(void)
  {
    int voicenum;

    for (voicenum=0; voicenum < VOICES; voicenum++)
      {
        if (inuse[voicenum])
          {
            if (voice[voicenum].done)
              {
                voicecount--;
                deallocate_voice(voicenum);
              }
          }
      }
  }

/* Mixing */

static int   mixlength;
static signed char *sourceptr;
static signed short int *volume_lookup;
static int chunklength;
static int destindex;

static int wassilenced[2]={0,0};
static int donefirst;
static signed short *dest;
static int vol;

static void mix_voice(int voicenum)
  {
   SOUND *sound;
   if (sixteenbit) dest=(short *)blockptr[curblock];
      else         dest=mixingblock;
   
    if (sixteenbit) wassilenced[curblock]=0;
      else          wassilenced[0]=0;

   /* Initialization */
    sound = voice[voicenum].sound;

    sourceptr = sound->soundptr + voice[voicenum].curpos;
    destindex = 0;

   /* Compute mix length */
    if (voice[voicenum].loop)
      mixlength = BLOCK_LENGTH;
    else
      mixlength =
       min(BLOCK_LENGTH, sound->soundsize - voice[voicenum].curpos);

   vol=voice[voicenum].volume ;
 //  if (voicecount==2) vol=vol*2/3;
//   if (voicecount>=3) vol=vol*2/3+1;
   
    volume_lookup =
     (signed short int *)(&((*volume_table)[sound_volume* vol/256 / (256/VOLUMES)]));
//     (signed short int *)volume_table[0];

//     (signed int *)(&((*volume_table)[(unsigned char)((((sound_volume/256.0) * voice[voicenum].volume) * (VOLUMES/256.0)))]));
   

    do
      {
       /* Compute the max consecutive samples that can be mixed */
        chunklength =
         min(mixlength, sound->soundsize - voice[voicenum].curpos);

       /* Update the current position */
        voice[voicenum].curpos += chunklength;

       /* Update the remaining samples count */
        mixlength -= chunklength;

       /* Mix samples until end of mixing or end of sound data is reached */
     if (chunklength)  
       if (!donefirst) //if we haven't mixed any yet
        {              //then copy the voice to the buffer, instead of mixing
//         COPYASM(&mixingblock[destindex],sourceptr,chunklength,volume_lookup);
         COPYASM(&dest[destindex],sourceptr,chunklength,volume_lookup);
         donefirst=1;
         sourceptr+=chunklength; destindex+=chunklength;
         chunklength=0;

         if (destindex<BLOCK_LENGTH)
           SILENCEASM(&dest[destindex],BLOCK_LENGTH-destindex);
         
        }
       else           //we've mixed already, so keep mixing
        {
         MIXASM(&dest[destindex],sourceptr,chunklength,volume_lookup);
         sourceptr+=chunklength; destindex+=chunklength;
         chunklength=0;
        }

       /* If we've reached the end of the block, wrap to start of sound */
        if (sourceptr == (sound->soundptr + sound->soundsize))
          {
            if (voice[voicenum].loop)
              {
                voice[voicenum].curpos = 0;
                sourceptr = sound->soundptr;
              }
            else
              {
                voice[voicenum].done = TRUE;
              }
          }
      }
    while (mixlength); /* Wrap around to finish mixing if necessary */
  }

static void silenceblock()
  {
//    memset(&mixingblock, 0x00, BLOCK_LENGTH*sizeof(signed int));
  if (sixteenbit)
   SILENCEASM((short *)blockptr[curblock],BLOCK_LENGTH);
  else 
   SILENCEASM(mixingblock,BLOCK_LENGTH);
  }


/*
void __cdecl SILENCEASM(signed short *x,unsigned int num);
void __cdecl COPYSOUND8(unsigned char *,signed short *,unsigned char *);
void __cdecl MIXASM(signed short *,unsigned char *,unsigned num, unsigned char *vol);       
void __cdecl COPYASM(signed short *,unsigned char *,unsigned num, unsigned char *vol);       
*/

static void mix_voices(void)
  {
    int i;

  if (!voicecount)  //no voices to mix, so just do silence
  {
   if (sixteenbit)
   {
    if (!wassilenced[curblock])     //if was silenced from last mix, dont bother
      {silenceblock(); wassilenced[curblock]=1;}  //silence the block
   } else
   {
    if (!wassilenced[0])     //if was silenced from last mix, dont bother
      {silenceblock(); wassilenced[0]=1;}  //silence the block
   }   
   
  }
    else           //There are voices to mix...
  {
    donefirst=0;   //flag to tell if first mix was done  
    for (i=0; i < VOICES; i++)
      if (inuse[i])
        mix_voice(i);
  }      
}
/*
static void copy_sound16(void)
  {
    int i;
    signed short *destptr;

    destptr   = blockptr[curblock];

    for (i=0; i < BLOCK_LENGTH; i++)
      destptr[i] = mixingblock[i];
  }

static void copy_sound8(void)
  {
    int i;
    unsigned char *destptr;

    destptr   = blockptr[curblock];

    for (i=0; i < BLOCK_LENGTH; i++)
      destptr[i] = (*clip_8)[mixingblock[i] >> 5];
  }
*/
static void copy_sound(void)
  {
    if (sixteenbit) {}
     // copy_sound16();
//     MemoryCopy(blockptr[curblock],mixingblock,BLOCK_LENGTH*2);
    else
    //  copy_sound8();
     COPYSOUND8((unsigned char *)blockptr[curblock],mixingblock,clip_8);
  }

static void startblock_sc(void)     /* Starts a single-cycle DMA transfer   */
  {
    outp(dma_maskport,   dma_stopmask);
    outp(dma_clrptrport, 0x00);
    outp(dma_modeport,   dma_mode);
    outp(dma_addrport,   lo(block_ofs[curblock]));
    outp(dma_addrport,   hi(block_ofs[curblock]));
    outp(dma_countport,  lo(BLOCK_LENGTH-1));
    outp(dma_countport,  hi(BLOCK_LENGTH-1));
    outp(dma_pageport,   block_page[curblock]);
    outp(dma_maskport,   dma_startmask);
    write_dsp(0x14);                /* 8-bit single-cycle DMA sound output  */
    write_dsp(lo(BLOCK_LENGTH-1));
    write_dsp(hi(BLOCK_LENGTH-1));
  }

volatile int smixbusy=0;

union REGS      r;
struct SREGS    sr;


static void interrupt inthandler(void)
  {
 intcount++;
 if (!smixbusy) // && !timerbusy)
 {
   smixbusy=1;
   loades();

    if (!autoinit)   /* Start next block quickly if not using auto-init DMA */
      {
        startblock_sc();
        copy_sound();
        curblock = !curblock;  /* Toggle block */
      }

    update_voices();
    mix_voices();

    if (autoinit)
      {
        copy_sound();
        curblock = !curblock;  /* Toggle block */
      }
    smixbusy=0;  
 }

inp(ackport);       /* Acknowledge interrupt with sound card */
outp(0xA0, 0x20);   /* Acknowledge interrupt with PIC2 */
outp(0x20, 0x20);   /* Acknowledge interrupt with PIC1 */
}

static void install_handler(void)
  {
    _disable();  /* CLI */
    outp(pic_maskport, (inp(pic_maskport) | irq_stopmask));
    oldintvector = _dos_getvect(irq_intvector);
    _dos_setvect(irq_intvector, inthandler);
    outp(pic_maskport, (inp(pic_maskport) & irq_startmask));
    _enable();   /* STI */
    handler_installed = TRUE;
  }

static void uninstall_handler(void)
  {
    _disable();  /* CLI */
    outp(pic_maskport, (inp(pic_maskport) | irq_stopmask));
   _dos_setvect(irq_intvector, oldintvector);
    _enable();   // STI 
    handler_installed = FALSE;
  }

static void smix_exitproc(void)
  {
    stop_dac();
    shutdown_sb();
  }

/* €€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€ */



void dos_memalloc(short int para, short int *seg, short int *sel);
#pragma  aux dos_memalloc = \
  "push  ecx"               \
  "push  edx"               \
  "mov   ax, 0100h"         \
  "int   31h"               \
  "pop   ebx"               \
  "mov   [ebx], dx"         \
  "pop   ebx"               \
  "mov   [ebx], ax"         \
  parm   [bx] [ecx] [edx]   \
  modify [ax ebx ecx edx];

/* ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒ */

void dos_memfree(short int sel);
#pragma  aux dos_memfree =  \
  "mov   ax, 0101h"         \
  "int   31h"               \
  parm   [dx]               \
  modify [ax dx];

/* ÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕÕ */

void  *low_malloc(int size, short int *sel)
  {
    short int seg;

    dos_memalloc((size >> 4) + 1, &seg, sel);
    return((char *)(seg << 4));
  }

/* ƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒƒ */

void low_free(short int sel)
  {
    dos_memfree(sel);
  }

