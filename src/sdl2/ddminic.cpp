#include "ddmimic.h"

static SDL_Renderer *dd_renderer = nullptr;
static DDDestOffset current_dest_offset{0, 0};

SDL_Renderer* dd_get_renderer() {
  return dd_renderer;
}

void dd_set_renderer(SDL_Renderer* renderer) {
  dd_renderer = renderer;
}

void dd_set_draw_offset(char *&dest, int x_off, int y_off) {
  dest = reinterpret_cast<char*>(DDDestPtr::DEST_OFFSET);
  current_dest_offset.x = x_off;
  current_dest_offset.y = y_off;
}

DDDestOffset dd_get_offset() {
  return current_dest_offset;
}
