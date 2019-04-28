/*      SMIXB is Copyright 1995 by Ethan Brodsky.  All rights reserved.     */

/* лл DETECT.C лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл */

#define TRUE  1
#define FALSE 0

int detect_settings(int *baseio, int *irq, int *dma, int *dma16);
  /* Detects sound card settings using BLASTER environment variable */
  /* Parameters:                                                    */
  /*   baseio    Sound card base IO address                         */
  /*   irq       Sound card IRQ                                     */
  /*   dma       Sound card 8-bit DMA channel                       */
  /*   dma16     Sound card 16-bit DMA channel (0 if none)          */
  /* Returns:                                                       */
  /*   TRUE      Sound card settings detected successfully          */
  /*   FALSE     Sound card settings could not be detected          */

/* лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл */

#include <ctype.h>
#include <dos.h>
#include <stdlib.h>
#include <string.h>

#define DECIMAL 0
#define HEX     1

int get_setting(char *str, char id, int hex, int *value)
  {
    char *paramstart;
    char buf1[128];
    char buf2[128];

    strcpy(buf1, str);
    if (strchr(buf1, id) != NULL)
      {
        paramstart = strchr(buf1, id) + 1;

        if (strchr(paramstart, ' ') != NULL)
          *(strchr(paramstart, ' ')) = '\0';

        if (hex)
          strcpy(buf2, "0x");
        else
          strcpy(buf2, "");

        strcat(buf2, paramstart);

        *value = strtoul(buf2, NULL, 0);
        return(TRUE);
      }
    else
      {
        value = 0;
        return(FALSE);
      }
  }

int detect_settings(int *baseio, int *irq, int *dma, int *dma16)
  {
    char blaster[128];

    if (getenv("BLASTER") == NULL)
      {
        *baseio = 0;
        *irq    = 0;
        *dma    = 0;
        *dma16  = 0;
        return (FALSE);
      }

    strupr(strcpy(blaster, getenv("BLASTER")));

    if (!get_setting(blaster, 'A', HEX,     baseio)) return(FALSE);
    if (!get_setting(blaster, 'I', DECIMAL, irq))    return(FALSE);
    if (!get_setting(blaster, 'D', DECIMAL, dma))    return(FALSE);
    get_setting(blaster, 'H', DECIMAL, dma16);

    return(TRUE);
  }
