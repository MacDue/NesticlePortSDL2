#include <SDL.h>
#include <cstdint>
#include "r2img.h"
#include "guivol.h"
#include "wip/stubs.h"
#include "nesvideo.h"
#include "sdl2/ddmimic.h"

#define CBASE 224

extern GUIVOL guivol;
extern COLOR nespal[];
extern int SCREENX, SCREENY;


#define CLIP_TILE(x,y)                        \
  int clipEndY = (y) + 8 - SCREENY;           \
  if (clipEndY < 0) clipEndY = 0;             \
  int clipEndX = (x) + 8 - SCREENX;           \
  if (clipEndX < 0) clipEndX = 0;             \
  int clipStartY = (y) < 0 ? abs((y)%8) : 0;  \
  int clipStartX = (x) < 0 ? abs((x)%8) : 0;


static inline SDL_Renderer* prepare_renderer(char* dest, int& x, int& y) {
  DDDestPtr dest_ptr = static_cast<DDDestPtr>(reinterpret_cast<size_t>(dest));
  SDL_Renderer* renderer = dd_get_renderer();
  if (dest_ptr == DDDestPtr::DEST_OFFSET) {
    DDDestOffset offset = dd_get_offset();
    x += offset.x;
    y += offset.y;
  }
  return renderer;
}


static inline void set_render_color(SDL_Renderer* renderer, uint8_t pallete_idx) {
  PALETTE* palette = guivol.pal;
  // COLOR* nes_palette = nespal;
  // COLOR color = (pallete_idx < CBASE ? palette->c : nespal)[pallete_idx];
  COLOR color = palette->c[pallete_idx];
  SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
}


void setpalettenum(int index,COLOR *c)
{
  guivol.pal->c[index] = *c;
// // msg.printf(1,"palentry[%d]=%d,%d,%d",index,c->r,c->g,c->b);
}


void __cdecl drawhline(char *d,int pallete_idx,int x,int y,int x2) {
  SDL_Renderer* renderer = prepare_renderer(d, x, y);
  set_render_color(renderer, pallete_idx);
  SDL_RenderDrawLine(renderer, x, y, x2 - 1, y);
}


void __cdecl drawvline(char *d,int pallete_idx,int x,int y,int y2) {
  SDL_Renderer* renderer = prepare_renderer(d, x, y);
  set_render_color(renderer, pallete_idx);
  SDL_RenderDrawLine(renderer, x, y, x, y2 - 1);
}


void drawrect(char *dest,int pallete_idx,int x,int y,int xw,int yw) {
  SDL_Renderer* renderer = prepare_renderer(dest, x, y);
  set_render_color(renderer, pallete_idx);
  SDL_Rect r{x, y, xw, yw};
  SDL_RenderFillRect(renderer, &r);
}


void __cdecl drawimager2(struct IMG *s,char *d,int draw_x,int draw_y,int o) {
  SDL_Renderer* renderer = prepare_renderer(d, draw_x, draw_y);

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
        set_render_color(renderer, pallete_idx);
        SDL_RenderDrawPoint(renderer, draw_x + (s->xw -1 - x), draw_y + y);
      }
    }
  }
}


void bitmap8x8::draw_tile(char *dest,int x,int y)
{
  CLIP_TILE(x,y);
  SDL_Renderer* renderer = prepare_renderer(dest, x, y);
  for (int i=clipStartY; i < 8 - clipEndY; i++) {
    for (int j=clipStartX; j< 8 - clipEndX; j++) {
      set_render_color(renderer, s[i][j]);
      SDL_RenderDrawPoint(renderer, x+j, y+i);
    }
  }
}


#define SET_PIXEL(i,j,p_idx) { \
      printf("sprite col %d\n", p_idx);\
    set_render_color(renderer, p_idx);                                  \
    SDL_RenderDrawPoint(renderer, x+(i), y+(j));                        \
  }

void bitmap8x8::draw_sprite(char *dest,int x,int y,int o)
{
 CLIP_TILE(x,y);
 SDL_Renderer* renderer = prepare_renderer(dest, x, y);

 switch (o & 0b11)
 {
  case 0: //normal
   {
   for (int i=clipStartY; i < 8 - clipEndY; i++)
    for (int j=clipStartX; j< 8 - clipEndX; j++)
      if (s[i][j]) SET_PIXEL(j, i, s[i][j]);
   }
   break;
  case 1: //flipx
   {
   for (int i=clipStartY; i < 8 - clipEndY; i++)
    for (int j=clipStartX; j< 8 - clipEndX; j++)
      if (s[i][j]) SET_PIXEL(j^7, i, s[i][j]);
   }
   break;
  case 2: //flipy
   {
   for (int i = 7 - clipEndY; i>=clipStartY; i--)
    for (int j=clipStartX; j< 8 - clipEndX; j++)
      if (s[i][j]) SET_PIXEL(j, i, s[i][j]);
   }
   break;
  case 3: //flipx|flipy
   {
   for (int i = 7 - clipEndY; i>=clipStartY; i--)
    for (int j=clipStartX; j< 8 - clipEndX; j++)
      if (s[i][j]) SET_PIXEL(j^7, i, s[i][j]);
   }
   break;
 }
}

extern "C" void __cdecl drawscr(char *s,char *d,int xw,int yw,int pitch) {
  STUB_BODY
}
