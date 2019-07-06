#include <cstdio>
#include <SDL.h>
#include "config.h"
#include "dd.h"

extern char configfile[];
extern char appname[];

int  initgame();
void updatescreen();
void terminategame();
void gametimer();

void enablegui();

void cleanup()
{
  terminategame();
 // terminatedsound();
 // terminateddraw();
 // if (timerid) {timeKillEvent(timerid); timerid=0;}
 // ShowCursor(TRUE);
}

static struct SDL_State {
  SDL_Window* window;
  SDL_Renderer* renderer;
} sdl_state;


static bool setup_SDL2(struct SDL_State* sdl_state) {
  sdl_state->window = NULL;
  sdl_state->renderer = NULL;

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
  if (sdl_state->window != NULL) {
    SDL_DestroyWindow(sdl_state->window);
  }
  if (sdl_state->renderer != NULL) {
    SDL_DestroyRenderer(sdl_state->renderer);
  }
  SDL_Quit();
  return false;
}

char *screen = NULL;

int initialize()
{
  bool sdl_setup = setup_SDL2(&sdl_state);
  screen = reinterpret_cast<char*>(sdl_state.renderer);
  return !sdl_setup;
}

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

  bool loop_alive = true;
  SDL_Event event;
  while(loop_alive) {
    SDL_SetRenderDrawColor(sdl_state.renderer, 0, 0, 255, 255);
    SDL_RenderClear(sdl_state.renderer);

    while(SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        loop_alive = false;
      }
    }
    updatescreen();
    SDL_RenderPresent(sdl_state.renderer);
  }

  cleanup();
}