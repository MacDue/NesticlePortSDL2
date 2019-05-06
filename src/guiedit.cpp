#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <cstdio>
#include "font.h"
#include "gui.h"
#include "mouse.h"
#include "r2img.h"
#include "keyb.h"
#include "dd.h"
#include "guivol.h"

extern FONT *font[10];
extern char* getclipboardtext();


GUIedit::GUIedit(GUIrect* parent, char* prompt, int x, int y, int xw)
  : GUIrect(parent, x, y, x, y)
{
  this->disabled = false;
  this->prompt[0] = '\0';
  if (prompt) {
    strcpy(this->prompt, prompt);
    this->promptw = font[0]->getwidth(this->prompt) + 1;
  } else {
    this->promptw = 0;
  }
  this->inputmaxw = xw;
  this->x2 = this->x1 + xw + this->promptw;
  this->y2 = this->y1 + 10;
}

GUIrect* GUIedit::hittest(int x, int y) {
  GUIrect* hit;
  if (this->disabled) {
    hit = NULL;
  } else {
    hit = GUIrect::hittest(x, y);
  }
  return hit;
}

GUIrect* GUIedit::click(class mouse &m) {
  return NULL;
}

void GUIedit::draw(char* dest) {
  font[3]->draw(this->prompt, dest, this->x1, this->y1);
  drawrect(
    dest,
    0,
    this->promptw + this->x1 - 1,
    this->y1 - 1,
    this->inputmaxw,
    10);
  if (!this->disabled) {
    this->drawdata(
      dest,
      this->promptw + this->x1,
      this->y1,
      this->inputmaxw - 2);
    if (this->focus) {
      drawbox(
        dest,
        2,
        this->promptw + this->x1 - 2,
        this->y2 - 2,
        this->inputmaxw + this->promptw + this->x1,
        this->y1 + 10);
    }
  }
  GUIrect::draw(dest);
}

/* GUItextedit */

GUItextedit::GUItextedit(GUIrect* parent, char* prompt, char* input, int x, int y, int xw, int maxinputlen)
  : GUIedit(parent, prompt, x, y, xw), maxinputlen(maxinputlen)
{
  this->setinput(input);
}

void GUItextedit::backspace() {
  if (this->inputlen > 0) {
    --this->inputlen;
  }
  this->input[this->inputlen] = '\0';
  this->getinputw();
}

void GUItextedit::addchar(char c) {
  if (this->inputlen < this->maxinputlen) {
    this->input[this->inputlen++] = c;
    this->input[this->inputlen] = '\0';
    this->getinputw();
  }
}

void GUItextedit::paste() {
  char* text = getclipboardtext();
  if (text) {
    if (strlen(text) < 0x50) {
      this->setinput(text);
    }
    free(text); // TODO: probably replace?
  }
}


void GUItextedit::drawdata(char* dest, int x, int y, int xw) {
  int x_end = x + xw;
  int y_end = y + 12;
  int old_screen_x = SCREENX;
  int old_screen_y = SCREENY;
  if (x < 0) {
    x = 0;
  }
  if (x_end > SCREENX) {
    x_end = SCREENX;
  }
  if (y < 0) {
    y = 0;
  }
  if (y_end > SCREENY) {
    y_end = SCREENY;
  }
  /* TODO: Port following code (too low level): */
  char* start_px = x + /* assume row length in bytes? */ PITCH * y + dest;
  /*
    Some kind of hack to translate the text position
    SCREENX/Y are using in the draw*() code.
    This code needs fixing for higher level graphics.
  */
  SCREENX = x_end - x;
  SCREENY = y_end - y;
  int padding = this->inputw - (xw - 5);
  if (padding < 0) {
    padding = 0;
  }
  font[1]->draw(this->input, start_px, -padding, 0);
  if (this->focus && uu & 0x20) {
    font[1]->draw('_', start_px, this->inputw - padding, 0);
  }
  SCREENX = old_screen_x;
  SCREENY = old_screen_y;
}

int GUItextedit::isvalidkey(char key) {
  return isgraph(key); /* Just a guess */
}

int GUItextedit::keyhit(char kbscan, char key) {
  int result;
  if (kbscan == 14) {
    this->backspace();
    result = 1;
  } else if (toupper(key) == 'V' && kbstat & 2
    || kbscan == 82 && kbstat & 1
  ) {
    this->paste();
    result = 1;
  } else {
    if (this->isvalidkey(key)) {
      this->addchar(key);
      result = 1;
    } else {
      result = 0;
    }
  }
  return result;
}

void GUItextedit::getinputw() {
  this->inputw = font[0]->getwidth(this->input);
  if (this->parent) {
    this->parent->sendmessage(this, GUIMSG_EDITCHANGED);
  }
}

void GUItextedit::setinput(char* inp) {
  if (inp) {
    strcpy(this->input, inp);
  } else {
    this->input[0] = '\0';
  }
  this->inputlen = strlen(this->input);
  this->inputw = font[0]->getwidth(this->input);
}

/* GUInumbertextedit */

char* itoa(int num, char* buf, int base) {
  sprintf(buf, "%d", num);
  return buf;
}

GUInumbertextedit::GUInumbertextedit
  (GUIrect* parent, char* prompt, int input, int x, int y, int xw, int maxinputlen)
    : GUItextedit(parent, prompt, itoa(input, this->input, 10), x, y, xw, maxinputlen)
{
}

int GUInumbertextedit::isvalidkey(char key) {
  return isdigit(key) || (key == '-' && !this->inputlen);
}

int GUInumbertextedit::getstate() {
  return atol(this->input);
}

/* GUInumberedit */

GUInumberedit::GUInumberedit(GUIrect* parent, char* prompt, int x, int y, int xw)
  : GUIedit(parent, prompt, x, y - 2, xw)
{
  this->x2 += 10;
  this->y2 += 10;
  this->up = new GUIimagebutton(this, guivol.umark, 0, 0);
  this->up->moveto(this->x2 - this->up->width() + 1, this->y1);
  this->down = new GUIimagebutton(this, guivol.dmark, 0, 0);
  this->down->moveto(this->x2 - this->down->width() + 1, this->y1 + 8);
}

int GUInumberedit::keyhit(char kbscan, char key) {
  int result;
  if (kbscan == 14) {
    this->clear();
    this->clip();
    result = 1;
  } else if (key == '?') {
    this->setmax();
    this->clip();
    result = 1;
  } else if (key != '+' && key != '=' && key != '.' && key != '>') {
    if (key != '-' && key != '_' && key != ',' && key != '<') {
      result = 0;
    } else {
      this->sendmessage(this->down, GUIMSG_HELDDOWN);
      result = 1;
    }
  } else {
    this->sendmessage(this->up, GUIMSG_HELDDOWN);
    result = 1;
  }
  return result;
}

void GUInumberedit::draw(char* dest) {
  font[3]->draw(this->prompt, dest, this->x1, this->y1 + 2);
  drawrect(
    dest,
    0,
    this->promptw + this->x1 + 1,
    this->y1 + 2,
    this->inputmaxw,
    10);
  if (!this->disabled) {
    this->drawdata(dest, this->promptw + this->x1, this->y1 + 3, this->inputmaxw - 2);
    if (this->focus) {
      drawbox(
        dest,
        2,
        this->promptw + this->x1 - 2,
        this->y1 + 1,
        this->inputmaxw + this->promptw + this->x1,
        this->y1 + 13);
    }
  }
}

/* GUIintedit */

GUIintedit::GUIintedit(GUIrect* parent, char* prompt, int x, int y, int xw, int num, int min, int max)
  : GUInumberedit(parent, prompt, x, y, xw), min(min), max(max), n(num)
{
  this->clip();
}

void GUIintedit::add(int num) {
  int result = num + this->n;
  if (this->n != num + this->n) {
    this->set(result);
  }
}

void GUIintedit::minus(int num) {
  int result = this->n - num;
  if (this->n != result) {
    this->set(result);
  }
}

void GUIintedit::clip() {
  if (this->max > this->n) {
    this->n = this->max;
  }
  if (this->min < this->n) {
    this->n = this->min;
  }
}

void GUIintedit::clear() {
  if (this->n) {
    this->set(0);
  }
}

void GUIintedit::setmax() {
  if (this->max != this->n) {
    this->set(this->max);
  }
}

void GUIintedit::drawdata(char* dest, int x, int y, int xw) {
  font[1]->printf(x, y, "%d", this->n);
}

int GUIintedit::sendmessage(GUIrect* c, int msg) {
  if (msg == GUIMSG_HELDDOWN) {
    if (kbstat & 1) {
      if (c == this->up) {
        this->add(10);
      }
      if (c == this->down) {
        this->minus(10);
      }
    } else {
      if (c == this->up) {
        this->add(1);
      }
      if (c == this->down) {
        this->minus(1);
      }
    }
  }
  return 0;
}

/* GUIfloatedit */

GUIfloatedit::GUIfloatedit
  (GUIrect* parent, char* prompt, int x, int y, int xw, float num, float min, float max)
    : GUInumberedit(parent, prompt, x, y, xw), min(min), max(max), n(num)
{
  this->clip();
}

void GUIfloatedit::add(float num) {
  float result = this->n + num;
  if (this->n != result) {
    this->set(result);
  }
}

void GUIfloatedit::minus(float num) {
  float result = this->n = num;
  if (this->n != result) {
    this->set(result);
  }
}

void GUIfloatedit::clip() {
  if (this->n < this->min) {
    this->n = this->min;
  }
  if (this->n > this->max) {
    this->n = this->max;
  }
}

void GUIfloatedit::clear() {
  if (this->n != 0.0) {
    this->set(0);
  }
}

void GUIfloatedit::setmax() {
  if (this->n != this->max) {
    this->set(this->max);
  }
}

void GUIfloatedit::drawdata(char* data, int x, int y, int xw) {
  font[1]->printf(x, y, "%.1f", this->n);
}

int GUIfloatedit::sendmessage(GUIrect* c, int msg) {
  if (msg == GUIMSG_HELDDOWN) {
    if (kbstat & 1) {
      if (c == this->up) {
        this->add(1.0);
      }
      if (c == this->down) {
        this->minus(1.0);
      }
    } else {
      if (c == this->up) {
        this->add(0.1);
      }
      if (c == this->down) {
        this->minus(0.1);
      }
    }
  }
  return 0;
}
