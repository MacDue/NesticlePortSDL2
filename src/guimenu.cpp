#include <stdlib.h>
#include "dd.h"
#include "r2img.h"
#include "font.h"
#include "guirect.h"
#include "guimenu.h"
#include "mouse.h"
#include "keyb.h"
#include "guiroot.h"

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
        /* Might be an infinite loop */
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
void GUImenu::losefocus() {
  GUIrect* child;
  while (true) {
    /* Might be an infinite loop */
    child = this->child;
    if (!child) {
      break;
    }
    if (child) {
      delete child;
    }
  }
  this->selmi = NULL;
}


void GUImenu::losechildfocus() {
  ;
}


int GUImenu::domenuitem(menuitem *i) {
  if (i) {
    i->menufunc();
  }
  return 1;
}

/* GUIvmenu */

GUIvmenu::GUIvmenu(GUImenu* parent, class menu *m, int x, int y)
  : GUImenu(parent, m, x, y)
{
  this->x2 = this->x1 + this->getmenuwidth();
  this->y2 = this->y1 + this->getmenuheight();
}

void GUIvmenu::draw(char* dest) {
  drawrect(
    dest,
    /* color ?*/ 154,
    this->x1,
    this->y1,
    this->width(),
    this->height());
  int item_y = this->y1;
  // Best guess V
  for (menuitem* item = this->tmenu->m; item->text; item++) {
    if (item == this->selmi) {
      drawrect(dest, 2, this->x1, item_y, this->width(), item->height());
      item->draw(this->x1, item_y, this->width(), 2);
    } else {
      item->draw(this->x1, item_y, this->width(), 3);
    }
    if (item->key & 0x80) {
      char* keyname = getkeyname(item->key & 0x7F);
      font[2]->draw(keyname, screen, this->x2 - 12, item_y);
    } else {
      font[2]->draw(item->key, screen, this->x2 - 8, item_y);
    }
    if (item->submenu) {
      font[3]->draw('>', screen, this->x2 - 8, item_y);
    }
    item_y += item->height();
  }
  GUIrect::draw(dest);
}

class menuitem* GUIvmenu::menuhittest(int x, int y, int &sx, int &sy) {
  int current_height = 0;
  class menuitem* item;
  for (item = this->tmenu->m ; ; item++) {
    if (!item) {
      return NULL;
    }
    current_height += item->height();
    if (current_height > y) {
      break;
    }
  }
  // Unsure if correct:
  if (item->text[0] == '-') {
    return NULL;
  } else {
    sx = this->width();
    sy = current_height - item->height();
    return item;
  }
}

/* GUIhmenu */

GUIhmenu::GUIhmenu(GUIrect* parent, class menu *m, int x, int y)
  : GUImenu(parent, m, x, y)
{
  this->x2 = this->x1 + this->getmenuwidth();
  this->y2 = this->y1 + this->getmenuheight();
  GUIrect::setfocus(this);
}

void GUIhmenu::draw(char* dest) {
  int item_x = this->x1;
  drawrect(dest,
    154,
    this->x1,
    this->y1,
    this->width(),
    this->height());
  for (class menuitem* item = this->tmenu->m; item->text; item++) {
    if (item == this->selmi) {
      drawrect(dest, 2, item_x, this->y1, item->width(), item->height());
      item->draw(item_x, this->y1, 20, 2);
    } else {
      item->draw(item_x, this->y1, 20, 3);
    }
    item_x += item->width();
  }
  GUIrect::draw(dest);
}

class menuitem* GUIhmenu::menuhittest(int x, int y, int &sx, int &sy) {
  int current_width = 0;
  for (class menuitem* item = this->tmenu->m; item->text; item++) {
    current_width += item->width();
    if (current_width > x) {
      sx = current_width - item->width();
      sy = this->y2;
      return item;
    }
  }
  return NULL;
}

int GUIhmenu::keyhit(char kbscan, char key) {
  int result;
  if (key) {
    for (class menuitem* item = this->tmenu->m; item->text; item++) {
      /* kbscan == *item in dis (but that seems wrong) */
      if (item->submenu && item->submenu->keyhit(kbscan, key)) {
        return 1;
      }
    }
    result = 0;
  } else {
    result = 0;
  }
  return result;
}

/* GUIpopupmenu */

GUIpopupmenu::GUIpopupmenu(GUIrect* treport, class menu* m, int x, int y)
  : GUIvmenu(reinterpret_cast<GUImenu*>(guiroot), m, x, y), report(treport)
{
  if (this->x1 < 0) {
    this->moverel(-this->x1, 0);
  }
  if (this->y1 < 0) {
    this->moverel(0, -this->y1);
  }
  if (SCREENX < this->x2) {
    this->moverel(SCREENX - this->x2, 0);
  }
  if (SCREENY < this->y2) {
    this->moverel(0, SCREENY - this->y2);
  }
  GUIrect::setmodal(this);
}


int GUIpopupmenu::domenuitem(menuitem* t) {
  this->report->sendmessage(this, (t - this->tmenu->m)/sizeof(menuitem));
  return 1;
}

int GUIpopupmenu::release(mouse &m) {
  GUImenu::release(m);
  if (this) {
    delete this;
  }
  return 1;
}

void GUIpopupmenu::draw(char* dest) {
  GUIvmenu::draw(dest);
  GUIrect::outline(0);
}


GUIpopupmenu::~GUIpopupmenu() {
  GUIrect::setmodal(NULL);
}
