/*
  GUI.CPP -- Manually decompiled/reimplemented from GUI.LIB by MacDue.
  Variable names & code structure are best guesses.
  Mistakes are possible.

  Very literal implementation (clean up later).
*/

#include <stdlib.h>
#include <string.h>
#include "gui.h"
#include "r2img.h"
#include "font.h"
#include "mouse.h"
#include "guivol.h"

extern FONT *font[10];

void GUIbar::draw(char* dest) {
  drawrect(
    dest,
    this->color,
    this->x1,
    this->y1,
    this->x2 - this->x1,
    this->y2 - this->y1
  );
}


GUIstatictext::GUIstatictext(GUIrect* parent, int f, char* str, int x, int y)
  : GUIrect(parent, x, y, x, y), fontnum(f)
{
  this->settext(str);
}

void GUIstatictext::settext(char* str) {
  if (str) {
    /* Buffer overflow waiting to happen :) */
    strcpy(this->text, str);
  } else {
    this->text[0] = '\0';
  }
}

void GUIstatictext::draw(char* dest) {
  font[this->fontnum]->draw(
    this->text,
    dest,
    this->x1,
    this->y1);
}

void GUIstaticcenteredtext::draw(char* dest) {
  /* Best guess */
  FONT* font = &font[this->fontnum];
  int midX = (this->x2 - this->x1)/2;
  int fontWidth = font->getwidth(this->text);
  font->draw(
    this->text,
    dest,
    midX - fontWidth/2,
    this->y1
  );
}


GUIstaticimage::GUIstaticimage(GUIrect* parent, struct IMG* img, int x, int y)
  : GUIrect(parent, x, y, x, y), img(img)
{
}

void GUIstaticimage::draw(char* dest) {
  drawimager2(
    this->img,
    dest,
    this->x1,
    this->y1, 0);
}

/* GUIbutton */

GUIbutton::GUIbutton(GUIrect* parent, int x, int y, int xw, int yw)
  : GUIrect(parent, x, y, x + xw, y + yw)
{
  this->btimer.set(20);
  this->depressed = false;
}

GUIrect* GUIbutton::click(class mouse &m) {
  GUIrect* click_result;
  if (m.click) {
    this->depressed = true;
    this->btimer.set(28);
    // 60 sendmessage
    if (this->parent) {
      /*TODO Find out what 2 means: guess click*/
      this->parent->sendmessage(this, 2);
    }
    click_result = this;
  } else {
    click_result = NULL;
  }
  return click_result;
}

int GUIbutton::release(class mouse &m) {
  /* TODO: Check */
  int result;
  if (m.rel) {
    this->depressed = false;
    if (this->parent) {
      if (this->hittest(m.x, m.y)) {
        /* TODO find out what 1 means: guess release*/
        this->parent->sendmessage(this, 1);
      }
    }
    result = true;
  } else {
    result = false;
  }
  return result;
}

void GUIbutton::draw(char* dest) {
  if (this->depressed) {
    this->fill(/*color=*/27);
    drawhline(dest,/*color=*/29, /*x*/this->x1, /*y*/this->y1, /*x2*/this->x2 - 1);
    drawvline(dest,/*color=*/29, /*x*/this->x2 - 1, /*y*/this->y1, /*y2*/this->y2 - 1);
    drawvline(dest,/*color=*/23, /*x*/this->x1, /*y*/this->y1, /*y2*/ this->y2 - 1);
    drawhline(dest,/*color=*/23, /*x*/this->x1, /*y*/this->y2 - 1, this->x2 - 1);
  } else {
    this->fill(/*color=*/25);
    drawhline(dest,/*color=*/23, /*x*/this->x1, /*y*/this->y1, /*x2*/this->x2 - 1);
    drawvline(dest,/*color=*/23, /*x*/this->x2 - 1, /*y*/this->y1, /*y2*/this->y2 - 1);
    drawvline(dest,/*color=*/27, /*x*/this->x1, /*y*/this->y1, /*y2*/ this->y2 - 1);
    drawhline(dest,/*color=*/27, /*x*/this->x1, /*y*/this->y2 - 1, this->x2 - 1);
  }
  if (this->depressed) {
    if (this->parent) {
      if (this->btimer.check()) {
        GUIrect* hit = this->hittest(m.x, m.y);
        if (hit) {
          this->parent->sendmessage(this, 2);
          this->btimer.set(7);
        }
      }
    }
  }
}

/* GUItextbutton */

GUItextbutton::GUItextbutton(GUIrect* parent, char* text, int x, int y)
  : GUIbutton(parent, x, y, 0, 0)
{
  int text_width = font[0]->getwidth(text);
  this->bwidth = text_width + 12;
  this->moverel(-(text_width + 12)/2, 0);
  this->x2 = this->x1 + this->bwidth;
  this->y1 = this->y1 + 12;
  strcpy(this->text, text);
}

void GUItextbutton::settext(char* str) {
  if (str) {
    strcpy(this->text, str);
  } else {
    this->text[0] = '\0';
  }
}

void GUItextbutton::draw(char* dest) {
  GUIbutton::draw(dest);
  int font_idx = 3;
  if (!this->depressed) {
    font_idx = 1;
  }
  font[font_idx]->draw(this->text, dest, this->x1 + 6, this->y1 + 2);
}

/* GUIimagebutton */

GUIimagebutton::GUIimagebutton(GUIrect* parent, struct IMG* img, int x, int y)
  : GUIbutton(parent, x, y, 0, 0), img(img)
{
  this->moverel(-img->xw/2, 0);
  this->x2 = this->x1 + img->xw + 2;
  this->y2 = this->y1 + img->yw + 2;
}

void GUIimagebutton::draw(char* dest) {
  GUIbutton::draw(dest);
  drawimager2(this->img, dest, this->x1 +1, this->y1 +1, 0);
}

/* GUIcheckbox */

GUIcheckbox::GUIcheckbox(GUIrect* parent, char* str, int x, int y, int state)
  : GUIrect(parent, x, y, x, y), checked(state), depressed(false)
{
  this->x2 = this->x1 + font[0]->getwidth(str) + 12;
  this->y2 = this->y1 + 10;
  strcpy(this->text, str);
}

int GUIcheckbox::keyhit(char kbscan, char key) {
  int result;
  if (key == ' ') {
    this->checked ^= 1;
    result = 1;
  } else {
    result = 0;
  }
  return result;
}

GUIrect* GUIcheckbox::click(class mouse &m) {
  if (m.click) {
    this->depressed = true;
    return this;
  } else {
    return NULL;
  }
}

int GUIcheckbox::release(class mouse &m) {
  int result;
  if (m.rel) {
    this->depressed = false;
    if (this->hittest(m.x, m.y)) {
      this->checked ^= 1;
      if (this->parent) {
        if (this->checked) {
          this->parent->sendmessage(this, 3 /*TODO find meaning: probably checked*/);
        } else {
          this->parent->sendmessage(this, 4 /*probably unchecked */);
        }
      }
    }
    result = 1;
  } else {
    result = 0;
  }
  return result;
}

void GUIcheckbox::draw(char* dest) {
  if (this->focus) {
    drawbox(
      dest,
      2,
      this->x1 - 1,
      this->y1 - 1,
      this->x2,
      this->y2);
  }
  drawrect(dest, 0, this->x1, this->y1, 8, 8);
  drawrect(dest, 15, this->x1 + 1, this->y1 + 1, 6, 6);
  if (this->checked) {
    drawimager2(guivol.checkmark, dest, this->x1 + 1, this->y1 + 1, 0);
  }
  int font_idx = 1;
  if (!this->depressed) {
    font_idx = 3;
  }
  font[font_idx]->draw(this->text, dest, this->x1 + 10, this->y1);
}

/* GUIbox */

GUIbox::GUIbox(GUIrect* parent, char* title, GUIcontents* contents, int x, int y)
  : GUIrect(
      parent,
      x,
      y,
      x + contents->x2 - contents->x1,
      y + contents->y2 - contents->y1 + 14), contents(contents)
{
  this->settitle(title);
  this->close = new GUIbutton(this, 0, 0, 8, 8); /* leaked! */
  this->reposclosebutton();
  this->contents->reparent(this);
  this->contents->moverel(0, 12);
  GUIrect::setfocus(this->contents);
  this->contents->cyclefocus(0);
}

void GUIbox::settitle(char* title) {
  if (title) {
    strcpy(this->title, title);
  } else {
    /* Uncertain predicted code: */
    this->title[0] = '\0';
  }
}

void GUIbox::reposclosebutton() {
  this->close->moveto(
    this->x2 - 10,
    this->y1 + 1);
}

GUIrect* GUIbox::click(class mouse &m) {
  if (!m.click || this->y1 + 12 <= m.y) {
    return NULL;
  }
  return this;
}

int GUIbox::release(class mouse &m) {
  return 1;
}

int GUIbox::drag(class mouse &m) {
  this->moverel(m.x - m.oldx, m.y - m.oldy);
  return 1;
}

int GUIbox::keyhit(char kbscan, char key) {
  return GUIrect::keyhit(kbscan, key) != 0;
}

int GUIbox::sendmessage(GUIrect* c, int msg) {
  if (c == this->close && msg == 1 && this) {
    delete this;
  }
  if (c == this->contents) {
    if (this->contents->parent) {
      this->contents->parent->sendmessage(this, msg);
    }
  }
  return 1;
}

void GUIbox::draw(char* dest) {
  drawbox(
    dest,
    !this->focus ? 0 : 139,
    this->x1 - 1,
    this->y1 - 1,
    this->x2 + 1,
    this->y2 + 1);
  drawrect(
    dest,
    143,
    this->x1,
    this->y1,
    this->x2 - this->x1,
    12);
  font[!this->focus ? 3 : 1]->draw(this->title, dest, this->x1 +5, this->y1 +1);
  drawrect(
    dest,
    143,
    this->x1,
    this->y2 -2,
    this->x2 - this->x1,
    2);
  GUIrect::draw(dest);
}

/* GUImaximizebox */

GUImaximizebox::GUImaximizebox(GUIrect* parent, char* title, GUIcontents* c, int x, int y)
 : GUIbox(parent, title, c, x, y), maximized(0)
{
  this->max = new GUIbutton(this, 0, 0, 8, 8); /* leaked? */
  this->reposmaxbutton();
  this->close->bringtofront();
}

void GUImaximizebox::maximize() {
  if (!this->maximized) {
    this->contents->maximize();
    this->reparent(root);
    this->unlink();
    this->parent = root;
    this->link(root->lastchild);
    this->maximized = true;
    this->reposmaxbutton();
  }
}

void GUImaximizebox::restore() {
  if (this->maximized) {
    this->contents->moveto(this->x1, this->y1 + 12);
    this->contents->restore();
    this->reparent(guiroot);
    this->maximized = false;
    this->bringtofront();
    this->reposmaxbutton();
  }
}

void GUImaximizebox::reposmaxbutton() {
  if (this->maximized) {
    this->max->moveto(
      this->contents->x2 - 10,
      this->contents->y1 + 2);
  } else {
    this->max->moveto(
      this->close->x1 - 10,
      this->close->y1);
  }
}

GUIrect* GUImaximizebox::hittest(int x, int y) {
  GUIrect* hit;
  if (this->maximized) {
    if (this->max->hittest(x, y)) {
      hit = this->max;
    } else {
      hit = this->contents->hittest(x, y);
    }
  } else {
    hit = GUIrect::hittest(x, y);
  }
  return hit;
}

int GUImaximizebox::sendmessage(GUIrect *c, int msg) {
  int result;
  if (c == this->max) {
    if (msg == 1) {
      if (this->maximized) {
        this->restore();
      } else {
        this->maximize();
      }
    }
    result = 1;
  } else {
    result = GUIbox::sendmessage(c, msg);
  }
  return result;
}

void GUImaximizebox::draw(char* dest) {
  if (this->maximized) {
    this->contents->draw(dest);
    this->max->draw(dest);
  } else {
    GUIbox::draw(dest);
  }
}

/* GUIonebuttonbox */

GUIonebuttonbox::GUIonebuttonbox
  (GUIrect* parent, char* str, GUIcontents* c, char* b1name, int x, int y)
    : GUIbox(parent, str, c, x, y)
{
  this->y2 += 10;
  /* Guess on inlined constructor params, wrong? */
  this->bar = new GUIbar(
    this,
    -113, /* color*/
    0,
    this->y2 - this->y1 -12,
    this->x2 - this->x1,
    12);
  //  *(_BYTE *)(bar_ + 45) = -113; ?
  // TODO: Possibly incorrect.
  this->b1 = new GUItextbutton(
    this,
    b1name,
    this->y2 - this->y1 - 12,
    (this->x2 - this->x1) / 2);
  this->contents->bringtofront();
}

int GUIonebuttonbox::sendmessage(GUIrect* c, int msg) {
  int result;
  if (c != this->b1 || msg != 1) {
    result = GUIbox::sendmessage(c, msg);
  } else {
    if (this->contents->sendmessage(this, 20)) {
      delete this;
    }
    result = 1;
  }
  return result;
}

int GUIonebuttonbox::keyhit(char kbscan, char key) {
  int result;
  /* maybe key == 28? */
  if (kbscan == 28) {
    if (this->contents->sendmessage(this, 20)) {
      delete this;
    }
    result = 1;
  } else {
    result = GUIbox::keyhit(kbscan, key);
  }
  return result;
}

/* GUItwobuttonbox */

GUItwobuttonbox::GUItwobuttonbox
  (GUIrect* parent, char *str, GUIcontents *contents, char* b1name, char* b2name, int x, int y)
    : GUIbox(parent, str, contents, x, y)
{
  this->bar = new GUIbar(
    this,
    -113,
    0,
    this->y2 - this->y1 -12,
    this->x2 - this->x1,
    12);
  this->b1 = new GUItextbutton(
    this,
    b1name,
    0, 0);
  this->b2 = new GUItextbutton(
    this,
    b2name,
    0, 0);
  this->b1->moverel(
    (this->x2 - this->x1) / 4,
    this->y2 - this->y1 - 12);
  this->b2->moverel(
    3 * (this->x2 - this->x1) / 4,
    this->y2 - this->y1 - 12);
  this->contents->bringtofront();
}

int GUItwobuttonbox::sendmessage(GUIrect* c, int msg) {
  int result;
  if (c != this->b1 || msg != 1) {
    if (c != this->b2 || msg != 1) {
      result = GUIbox::sendmessage(c, msg);
    } else {
      if (this->contents->sendmessage(this, 21)) {
        delete this;
      }
      result = 1;
    }
  } else {
    if (this->contents->sendmessage(this, 20)) {
      delete this;
    }
    result = 1;
  }
  return result;
}

int GUItwobuttonbox::keyhit(char kbscan, char key) {
  int result;
  if (kbscan == 28) {
    if (this->contents->sendmessage(this, 20)) {
      delete this;
    }
    result = 1;
  } else {
    result = GUIbox::keyhit(kbscan, key);
  }
  return result;
}

/* GUItrackbutton */
