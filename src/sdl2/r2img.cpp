#include <SDL.h>
#include <cstdint>
#include "r2img.h"
#include "wip/stubs.h"


#define TO_RGB(color)

struct RGB {
  uint8_t r, g, b;
};


struct RGB to_rgb(char byte_color) {
  struct RGB rgb {
    (byte_color >> 5) * 32,
    ((byte_color & 0b11100) >> 2) * 32,
    (byte_color & 0b11) * 64,
  };
  return rgb;
}


void __cdecl drawhline(char *d,int color,int x,int y,int x2) { STUB_BODY }

void __cdecl drawvline(char *d,int color,int x,int y,int y2) { STUB_BODY }

void drawrect(char *dest,int color,int x,int y,int xw,int yw) {
  SDL_Renderer* renderer = reinterpret_cast<SDL_Renderer*>(dest);
  SDL_Rect r;

  r.x=(x>=0) ? x : 0;
  r.y=(y>=0) ? y : 0;
  r.w=xw;
  r.h=yw;

  printf("%d %d %d %d\n", r.x, r.y, r.w, r.h);

  uint8_t red = (color & 0xFF000000) >> 24;
  uint8_t green = (color & 0x00FF0000) >> 16;
  uint8_t blue = (color & 0x0000FF00) >> 8;



  SDL_SetRenderDrawColor(renderer, red, green, blue, 255);
  SDL_RenderFillRect(renderer, &r);
}

void __cdecl drawimager2(struct IMG *s,char *d,int x,int y,int o) {
  SDL_Renderer* renderer = reinterpret_cast<SDL_Renderer*>(d);
  char* img_data = s->data();

  for (int dx=x; dx < x+s->xw; dx++) {
    for (int dy=y; dy < y+s->yw; dy++) {
      struct RGB color = to_rgb(img_data[(dx-x)+(dy-y)*s->xw]);
      SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
      SDL_RenderDrawPoint(renderer, dx, dy);
    }
  }
}

extern "C" void __cdecl drawscr(char *s,char *d,int xw,int yw,int pitch) { STUB_BODY }
