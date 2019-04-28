/*      SMIXW is Copyright 1995 by Ethan Brodsky.  All rights reserved      */

/* лл SMIX.H лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл */

#define TRUE  1
#define FALSE 0

#define ON  1
#define OFF 0

int  init_sb(int baseio, int irq, int dma, int dma16);
  /* Initializes control parameters, resets DSP and installs int. handler   */
  /*  Parameters:                                                           */
  /*   baseio    Sound card base IO address                                 */
  /*   irq       Sound card IRQ setting                                     */
  /*   dma       Sound card 8-bit DMA channel                               */
  /*   dma16     Sound card 16-bit DMA channel                              */
  /*  Returns:                                                              */
  /*   TRUE      Sound card successfully initialized                        */
  /*   FALSE     Sound card could not be initialized                        */

void shutdown_sb(void);
  /* Removes interrupt handler and resets DSP                               */


void init_mixing(void);
  /* Allocates internal buffers and starts digitized sound output           */

void shutdown_mixing(void);
  /* Deallocates internal buffers and stops digitized sound output          */


void start_sound(SOUND *sound, int index, unsigned char volume, int loop);
  /* Starts playing a sound                                                 */
  /*  Parameters:                                                           */
  /*   sound     Pointer to sound data structure                            */
  /*   index     A number to keep track of the sound with (Used to stop it) */
  /*   loop      Indicates whether sound should be continuously looped      */

void stop_sound(int index);
  /* Stops playing a sound                                                  */
  /*  Parameters:                                                           */
  /*   index     Index of sound to stop (All with given index are stopped)  */

int  sound_playing(int index);
  /* Checks if a sound is still playing                                     */
  /*  Parameters:                                                           */
  /*   index     Index used when the sound was started                      */
  /*  Returns:                                                              */
  /*   TRUE      At least one sound with the specified index is playing     */
  /*   FALSE     No sounds with the specified index are playing             */

void set_sound_volume(unsigned char new_volume);
  /* Sets overall sound volume                                              */
  /*  Parameters:                                                           */
  /*   new_volume  New overall sound volume (0-255)                         */

extern volatile long intcount;         /* Current count of sound interrupts */
extern volatile int  voicecount;       /* Number of voices currently in use */

extern int dspversion;
extern int   autoinit;
extern int   sixteenbit;

/* лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл */

