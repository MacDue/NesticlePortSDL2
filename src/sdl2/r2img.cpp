#include <SDL.h>
#include <cstdint>
#include "r2img.h"
#include "guivol.h"
#include "wip/stubs.h"


extern GUIVOL guivol;

#define GetAValue(argb)  (static_cast<uint8_t>((argb) >> 24))
#define GetRValue(argb)  (static_cast<uint8_t>((argb) >> 16))
#define GetGValue(argb)  (static_cast<uint8_t>((argb) >> 8))
#define GetBValue(argb)  (static_cast<uint8_t>((argb)))


void __cdecl drawhline(char *d,int pallete_idx,int x,int y,int x2) {
  SDL_Renderer* renderer = reinterpret_cast<SDL_Renderer*>(d);
  PALETTE* palette = guivol.pal;
  pallete_idx &= 0xff;
  COLOR color = palette->c[pallete_idx];
  SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
  SDL_RenderDrawLine(renderer, x, y, x2, y);
}

void __cdecl drawvline(char *d,int pallete_idx,int x,int y,int y2) {
  SDL_Renderer* renderer = reinterpret_cast<SDL_Renderer*>(d);
  PALETTE* palette = guivol.pal;
  pallete_idx &= 0xff;
  COLOR color = palette->c[pallete_idx];
  SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
  SDL_RenderDrawLine(renderer, x, y, x, y2);
}

void drawrect(char *dest,int pallete_idx,int x,int y,int xw,int yw) {
  SDL_Renderer* renderer = reinterpret_cast<SDL_Renderer*>(dest);
  PALETTE* palette = guivol.pal;
  pallete_idx &= 0xff;
  COLOR color = palette->c[pallete_idx];
  SDL_Rect r;
  r.x=(x>=0) ? x : 0;
  r.y=(y>=0) ? y : 0;
  r.w=xw;
  r.h=yw;
  SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
  SDL_RenderFillRect(renderer, &r);
}

void __cdecl drawimager2(struct IMG *s,char *d,int draw_x,int draw_y,int o) {
  SDL_Renderer* renderer = reinterpret_cast<SDL_Renderer*>(d);
  PALETTE* palette = guivol.pal;

  int* ydisp = s->ydisp();
  uint8_t* image_bytes = reinterpret_cast<uint8_t*>(s);
  for (int y = 0; y < s->yw; y++) {
    int row_offset = ydisp[y];

    int x = s->xw;
    while (x > 0) {
      // Transparent pixels (skip)
      int runlen = image_bytes[row_offset++];
      x -= runlen;

      if (x <= 0) {
        break;
      }

      // Opaque pixels
      runlen = image_bytes[row_offset++];

      while (runlen --> 0 and x --> 0) {
        int pallete_idx = image_bytes[row_offset++];
        // Horribly wasteful
        COLOR color = palette->c[pallete_idx];
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
        SDL_RenderDrawPoint(renderer, draw_x + (s->xw -1 - x), draw_y + y);
      }
    }
  }
}

extern "C" void __cdecl drawscr(char *s,char *d,int xw,int yw,int pitch) { STUB_BODY }
