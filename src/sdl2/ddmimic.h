#ifndef DDMINIC_H
#define DDMINIC_H

#include <cstddef>
class SDL_Renderer;

typedef size_t dest_ptr_t;

struct DDDestOffset {
  int x, y;
};

enum class DDDestPtr {
  DEST_NORMAL = 0,
  DEST_OFFSET,
};

SDL_Renderer* dd_get_renderer();
void dd_set_renderer(SDL_Renderer* renderer);

void dd_set_draw_offset(char *&dest, int x_off, int y_off);
DDDestOffset dd_get_offset();


#endif
