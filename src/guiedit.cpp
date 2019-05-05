#include <string.h>
#include <ctype.h>
#include <cstdlib>
#include "font.h"
#include "gui.h"
#include "mouse.h"
#include "r2img.h"
#include "keyb.h"
#include "dd.h"

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
