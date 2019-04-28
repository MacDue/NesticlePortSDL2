#ifndef _SOUND_
#define _SOUND_
//frequency
#define SOUNDFREQ 22050


//standard sound structure, 8-bit signed
struct SOUND
{
  unsigned long soundsize;
  signed   char soundptr[];
};

struct SOUND *ReadWavFile(char *name);
#endif

