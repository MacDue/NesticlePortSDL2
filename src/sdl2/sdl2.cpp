#include <cstdio>
#include <SDL.h>
#include "config.h"
#include "dd.h"
#include "mouse.h"
#include "message.h"

extern char configfile[];
extern char appname[];

int  initgame();
void updatescreen();
void terminategame();
void gametimer();

void enablegui();

static struct SDL_State {
  SDL_Window* window;
  SDL_Renderer* renderer;
  bool running;
} sdl_state;

static void destroy_SDL2(struct SDL_State* sdl_state) {
  if (sdl_state->window != NULL) {
    SDL_DestroyWindow(sdl_state->window);
  }
  if (sdl_state->renderer != NULL) {
    SDL_DestroyRenderer(sdl_state->renderer);
  }
  SDL_Quit();
}

static bool setup_SDL2(struct SDL_State* sdl_state) {
  sdl_state->window = NULL;
  sdl_state->renderer = NULL;
  sdl_state->running = true;

  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    fprintf(stderr, "Failed to start SDL (%s)\n", SDL_GetError());
    goto sdl_error;
  }

  sdl_state->window = SDL_CreateWindow(appname,
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    SCREENX, SCREENY,
    SDL_WINDOW_SHOWN);

  if (sdl_state->window == NULL) {
    fprintf(stderr, "Failed to create window (%s)\n", SDL_GetError());
    goto sdl_error;
  }

  sdl_state->renderer = SDL_CreateRenderer(sdl_state->window, -1, SDL_RENDERER_ACCELERATED);

  if (sdl_state->renderer == NULL) {
    fprintf(stderr, "Failed to create renderer (%s)\n", SDL_GetError());
    goto sdl_error;
  }

  return true;

sdl_error:
  destroy_SDL2(sdl_state);
  return false;
}

void cleanup()
{
  terminategame();
 // terminatedsound();
 // terminateddraw();
 // if (timerid) {timeKillEvent(timerid); timerid=0;}
 // ShowCursor(TRUE);
  destroy_SDL2(&sdl_state);
}

void quitgame() {
  sdl_state.running = false;
}

char *screen = NULL;

int initialize()
{
  bool sdl_setup = setup_SDL2(&sdl_state);
  screen = reinterpret_cast<char*>(sdl_state.renderer);
  return !sdl_setup;
}

int setddrawmode(int xw,int yw)
{
  if (!sdl_state.window) {
    return -1;
  }
  SDL_SetWindowSize(sdl_state.window, xw, yw);
 // if (!DDO) return -1;
 //
 // //set video mode
 // HRESULT err;
 // if ((err=DDO->SetDisplayMode(xw,yw,8,0,0))!=DD_OK) return err;
 //
 SCREENX=xw; SCREENY=yw;
 //
 // //clip cursor
 // RECT r;
 // r.left=0; r.right=SCREENX;
 // r.top=0; r.bottom=SCREENY;
 // ClipCursor(&r);
 return 0;
}

void changeresolution(int xw,int yw)
{
 // if (!DDO) return;
 //
 // releasesurfaces();
 if (setddrawmode(xw,yw))
  {
  msg.error("%dx%d mode not supported",xw,yw);
   setddrawmode(SCREENX,SCREENY);
  }
 // createsurfaces();
 //
  msg.printf(2,"DirectDraw mode set: %dx%dx%d",SCREENX,SCREENY,8);
}

class mouse m;

int main(int argc, char* argv[]) {
  cfg=new config();
  cfg->load(configfile);

  enablegui();
  if (initialize()!=0) return false;

  if (initgame()!=0)
  {
    cleanup();
    puts("Unable to initialize nes");
    return -1;
  }

  SDL_ShowCursor(SDL_DISABLE);
  SDL_Event event;
  while(sdl_state.running) {
    m.reset();
    SDL_SetRenderDrawColor(sdl_state.renderer, 0, 0, 0, 255);
    SDL_RenderClear(sdl_state.renderer);

    while(SDL_PollEvent(&event)) {
      int mb = 0;
      switch (event.type) {
        case SDL_QUIT:
          sdl_state.running = false;
          break;
        case SDL_MOUSEBUTTONDOWN:
          if (event.button.button == SDL_BUTTON_LEFT)  mb |= 1;
          if (event.button.button == SDL_BUTTON_RIGHT) mb |= 2;
        case SDL_MOUSEBUTTONUP:
          m.updatebut(mb);
          break;
      }
    }
    int mouse_x, mouse_y;
    SDL_GetMouseState(&mouse_x, &mouse_y);
    m.updatexy(mouse_x, mouse_y);
    updatescreen();
    SDL_RenderPresent(sdl_state.renderer);
  }

  cleanup();
}
