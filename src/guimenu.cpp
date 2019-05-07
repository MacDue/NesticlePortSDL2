#include "dd.h"
#include "r2img.h"
#include "font.h"
#include "guirect.h"
#include "guimenu.h"

extern FONT *font[10];

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
