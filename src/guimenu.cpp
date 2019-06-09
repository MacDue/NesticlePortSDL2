#include <stdlib.h>
#include "dd.h"
#include "r2img.h"
#include "font.h"
#include "guirect.h"
#include "guimenu.h"
#include "mouse.h"

extern FONT *font[10];

/* menuitem */
void menuitem::draw(int x, int y, int xw, int sel) {
  // maybe?
  if (this->text[0] == '-') {
    drawrect(screen, 151, x + 3, y + 4, xw - 6, 2);
  } else {
    font[sel]->draw(this->text, screen, x + 3, y);
  }
}

int menuitem::width() {
  int result;
  if (this->text) {
    result = font[0]->getwidth(this->text) + 10;
  } else {
    result = 5;
  }
  return result;
}

int menuitem::height() {
  return 10;
}

int menuitem::menufunc() {
  int result;
  if (this) {
    if (this->func) {
      this->func();
      result = 1;
    } else {
      result = 0;
    }
  } else {
    result = 0;
  }
  return result;
}

/* menu */
int menu::getnumitems() {
  int numitems = 0;
  class menuitem* item = this->m;
  while (item->text) {
    ++numitems;
    item++;
  }
  return numitems;
}

int menu::getmaxwidth() {
  int max_width = 0;
  for (class menuitem* item = this->m; item->text; item++) {
    int width = item->width();
    if (max_width < width) {
      max_width = width;
    }
  }
  return max_width;
}

int menu::gettotalwidth() {
  int total_width = 0;
  for (class menuitem* item = this->m; item->text; item++) {
    total_width += item->width();
  }
  return total_width;
}

int menu::getmaxheight() {
  return this->m[0].height();
}

int menu::gettotalheight() {
  return this->m[0].height() * this->getnumitems();
}

int menu::keyhit(char kbscan, char key) {
  for (class menuitem* item = this->m; item->text; item++) {
    char shortcut = item->key;
    if (shortcut & 0x80 && (shortcut & 0x7F) == kbscan
        || (!(shortcut & 0x80) && shortcut == key)
    ) {
      item->menufunc();
      return 1;
    }
  }
  return 0;
}

/* GUImenu */

GUImenu::GUImenu(GUIrect* parent, class menu* m, int x, int y)
  : GUIrect(parent, x, y, x, y)
{
  this->tmenu = m;
  this->selmi = 0;
}

GUIrect* GUImenu::click(class mouse &m) {
  GUIrect* result;
  if (this == this->hittest(m.x, m.y)) {
    if (this->child) {
      this->losefocus();
      result = 0;
    } else {
      result = this;
    }
  } else {
    GUIrect* child = this->child;
    if (child) {
      if (child->click(m)) {
        result = this;
      } else {
        this->losefocus();
        result = 0;
      }
    } else {
      result = 0;
    }
  }
  return result;
}

int GUImenu::release(class mouse &m) {
  int result;
  GUIrect* child = this->child;
  if (child) {
    if (child->release(m)) {
      this->losefocus();
      result = 1;
    } else {
      result = 0;
    }
  } else {
    if (this->selmi
      && this == this->hittest(m.x, m.y)
    ) {
      this->domenuitem(this->selmi);
      if (!this->selmi->submenu) {
        result = 1;
        this->selmi = NULL;
        return result;
      }
      this->losefocus();
    }
    result = 0;
  }
  return result;
}

int GUImenu::drag(class mouse &m) {
  int sx, sy;
  int result;
  if (this->hittest(m.x, m.y)) {
    class menuitem* item = menuhittest(m.x - this->x1, m.y - this->y1, sx, sy);
    if (item != this->selmi) {
      this->selmi = item;
      GUIrect* child;
      while(true) {
        child = this->child;
        if (!child) {
          break;
        }
        if (child) {
          delete child;
        }
      }
      if (this->selmi && this->selmi->submenu) {
        /* this->child = ? */ new GUIvmenu(this, this->selmi->submenu, sx, sy);
      }
    }
    result = 1;
  } else {
    if (this->child) {
      this->child->drag(m);
      result = 1;
    }
  }
  return result;
}

// next
// void GUImenu::losefocus() {
//
// }
