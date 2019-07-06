/*
  GUI.CPP -- Manually decompiled/reimplemented from GUI.LIB by MacDue.
  Variable names & code structure are best guesses.
  Mistakes are possible.

  Very literal implementation (clean up later).
*/

#include <stdlib.h>
#include <string.h>
#include <cassert>
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
  int midX = this->width()/2;
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
      this->parent->sendmessage(this, GUIMSG_HELDDOWN);
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
        this->parent->sendmessage(this, GUIMSG_PUSHED);
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
          this->parent->sendmessage(this, GUIMSG_HELDDOWN);
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
          this->parent->sendmessage(this, GUIMSG_CHECKED);
        } else {
          this->parent->sendmessage(this, GUIMSG_UNCHECKED);
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
      x + contents->width(),
      y + contents->height() + 14), contents(contents)
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
    this->width(),
    12);
  font[!this->focus ? 3 : 1]->draw(this->title, dest, this->x1 +5, this->y1 +1);
  drawrect(
    dest,
    143,
    this->x1,
    this->y2 -2,
    this->width(),
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
    this->height() -12,
    this->width(),
    12);
  //  *(_BYTE *)(bar_ + 45) = -113; ?
  // TODO: Possibly incorrect.
  this->b1 = new GUItextbutton(
    this,
    b1name,
    this->height() - 12,
    this->width() / 2);
  this->contents->bringtofront();
}

int GUIonebuttonbox::sendmessage(GUIrect* c, int msg) {
  int result;
  if (c != this->b1 || msg != 1) {
    result = GUIbox::sendmessage(c, msg);
  } else {
    if (this->contents->sendmessage(this, GUIMSG_B1)) {
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
    if (this->contents->sendmessage(this, GUIMSG_B1)) {
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
    this->height() - 12,
    this->width(),
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
    this->width() / 4,
    this->height() - 12);
  this->b2->moverel(
    3 * this->width() / 4,
    this->height() - 12);
  this->contents->bringtofront();
}

int GUItwobuttonbox::sendmessage(GUIrect* c, int msg) {
  int result;
  if (c != this->b1 || msg != 1) {
    if (c != this->b2 || msg != 1) {
      result = GUIbox::sendmessage(c, msg);
    } else {
      if (this->contents->sendmessage(this, GUIMSG_B2)) {
        delete this;
      }
      result = 1;
    }
  } else {
    if (this->contents->sendmessage(this, GUIMSG_OK)) {
      delete this;
    }
    result = 1;
  }
  return result;
}

int GUItwobuttonbox::keyhit(char kbscan, char key) {
  int result;
  if (kbscan == 28) {
    if (this->contents->sendmessage(this, GUIMSG_OK)) {
      delete this;
    }
    result = 1;
  } else {
    result = GUIbox::keyhit(kbscan, key);
  }
  return result;
}

/* GUItrackbutton */

GUItrackbutton::GUItrackbutton(class GUItrack *parent, int size)
  : GUIbutton(parent, 0, 0, size, size), ptrack(parent)
{}


/* GUIvtrackbutton */

GUIvtrackbutton::GUIvtrackbutton(class GUIvtrack *parent, int size)
  : GUItrackbutton(parent, size)
{}

GUIrect* GUIvtrackbutton::click(class mouse &m) {
  this->tpos = m.y - this->y1;
  return GUIbutton::click(m);
}

int GUIvtrackbutton::drag(class mouse &m) {
  this->ptrack->movethumb(m.y - this->tpos - this->ptrack->y1);
  return 1;
}

/* GUIhtrackbutton */

GUIhtrackbutton::GUIhtrackbutton(class GUIhtrack* parent, int size)
  : GUItrackbutton(parent, size)
{}

GUIrect* GUIhtrackbutton::click(class mouse &m) {
  this->tpos = m.x - this->x1;
  return GUIbutton::click(m);
}

int GUIhtrackbutton::drag(class mouse &m) {
  this->ptrack->movethumb(m.x - this->tpos - this->ptrack->x1);
  return 1;
}

/* GUItrack */

GUItrack::GUItrack(GUIscrollbar* parent, int x, int y, int xw, int yw)
  : GUIrect(parent, x, y, x + xw, y + yw), tracklen(yw), trackpos(0), pscroll(parent)
{ }

void GUItrack::movethumb(int tpos) {
  if (tpos < 0) {
    tpos = 0;
  }
  if (tpos > this->tracklen) {
    tpos = this->tracklen;
  }
  this->trackpos = tpos;
  this->positionthumb();
  this->pscroll->setposfromtrack(this->trackpos, this->tracklen);
}

void GUItrack::setthumb() {
  /* UNKNOWN IMPL */
  assert(false && "not implemented!");
}

void GUItrack::settrackfrompos(int pos, int min, int max) {
  this->trackpos = this->tracklen * (pos - min) / (max - min);
  this->positionthumb();
}

void GUItrack::draw(char* dest) {
  drawrect(
    dest,
    30,
    this->x1,
    this->y1,
    this->width(),
    this->height());
  GUIrect::draw(dest);
}

/* GUIvtrack */

GUIvtrack::GUIvtrack(GUIvscrollbar* parent)
  : GUItrack(
    parent,
    0,
    parent->up->height(),
    parent->width(),
    parent->down->y1 - parent->up->y2)
{
  this->thumb = new GUIvtrackbutton(this, this->width());
}

void GUIvtrack::positionthumb() {
  this->thumb->moveto(this->x1, this->trackpos + this->y1);
}

void GUIvtrack::setthumbsize(int posrange) {
  int y_scale = this->height() / posrange;
  if (y_scale < 8) {
    y_scale = 8;
  }
  this->thumb->x2 = this->thumb->x1 + this->width();
  this->thumb->y2 = this->thumb->y1 + y_scale;
  this->tracklen = this->height() - y_scale;
  movethumb(this->trackpos);
}

/* GUIhtrack */

GUIhtrack::GUIhtrack(GUIhscrollbar* parent)
  : GUItrack(
    parent,
    parent->up->width(),
    0,
    parent->down->x1 - parent->up->x2,
    parent->height())
{
  this->thumb = new GUIhtrackbutton(this, this->y2 - this->y1);
}

void GUIhtrack::positionthumb() {
  this->thumb->moveto(this->trackpos + this->x1, this->y1);
}

void GUIhtrack::setthumbsize(int posrange) {
  int x_scale = this->width() / posrange;
  if (x_scale < 8) {
    x_scale = 8;
  }
  this->thumb->x2 = this->thumb->x1 + x_scale;
  this->thumb->y2 = this->thumb->y1 + this->height();
  this->tracklen = this->width() - x_scale;
  movethumb(this->trackpos);
}

/* GUIscrollbar */

GUIscrollbar::GUIscrollbar(GUIrect* parent, int x, int y, int xw, int yw)
  : GUIrect(parent, x, y, x + xw, y + yw), pos(0), min(0), max(0)
{ }

void GUIscrollbar::clip() {
  if (this->min > this->pos) {
    this->pos = this->min;
  }
  if (this->max < this->pos) {
    this->pos = this->max;
  }
}

void GUIscrollbar::setrange(int tmin, int tmax) {
  this->min = tmin;
  this->max = tmax;
  if (tmax < this->min) {
    this->max = this->min;
  }
  if (this->max != this->min) {
    this->track->settrackfrompos(
      this->pos,
      this->min,
      this->max);
    this->track->setthumbsize(this->max - this->min + 1);
  }
}

void GUIscrollbar::setpos(int p) {
  this->pos = p;
  this->clip();
  if (this->max != this->min) {
    this->track->settrackfrompos(this->pos, this->min, this->max);
  }
}

void GUIscrollbar::setposfromtrack(int tpos, int tlen) {
  int new_pos_x16 = 16 * this->min + (this->max - this->min) * 16 * tpos / tlen;
  int new_pos_rounding_error = new_pos_x16 & 0xF;
  this->pos = new_pos_x16 >> 4;
  if (new_pos_rounding_error >= 8) {
    ++this->pos;
  }
}

void GUIscrollbar::draw(char* dest) {
  if (this->min != this->max) {
    GUIrect::draw(dest);
    if (this->focus) {
      drawbox(
        dest,
        2,
        this->x1 - 1,
        this->y1 - 1,
        this->x2 + 1,
        this->y2 + 1);
    }
  }
}

int GUIscrollbar::sendmessage(GUIrect* c, int msg) {
  if (msg == 2) {
    if (c == this->up) {
      this->setpos(this->pos - 1);
    }
    if (c == this->down) {
      this->setpos(this->pos + 1);
    }
  }
  return 0;
}

/* GUIvscrollbar */

GUIvscrollbar::GUIvscrollbar(GUIrect* parent, int x, int y, int height)
  : GUIscrollbar(parent, x, y, 11, height)
{
  this->up = new GUIimagebutton(this, guivol.umark, 0, 0);
  this->down = new GUIimagebutton(this, guivol.dmark, 0, 0);
  this->up->moveto(this->x1, this->y1);
  this->down->moveto(this->x1, this->y2 - this->down->height());
  this->track = new GUIvtrack(this);
  this->setrange(0, 0);
  this->setpos(0);
}

int GUIvscrollbar::keyhit(char kbscan, char key) {
  int result;
  /* probably kbscan ? */
  if (kbscan == 72) {
    this->sendmessage(this->up, GUIMSG_HELDDOWN);
    result = 1;
  } else if (kbscan == 80) {
    this->sendmessage(this->down, GUIMSG_HELDDOWN);
    result = 1;
  } else {
    result = 0;
  }
  return result;
}

/* GUIhscrollbar */

GUIhscrollbar::GUIhscrollbar(GUIrect* parent, int x, int y, int width)
  : GUIscrollbar(parent, x, y, width, 11)
{
  this->up = new GUIimagebutton(this, guivol.lmark, 0, 0);
  this->down = new GUIimagebutton(this, guivol.rmark, 0, 0);
  this->up->moveto(this->x1, this->y1);
  this->down->moveto(this->x2 - this->down->width(), this->y1);
  this->track = new GUIhtrack(this);
  this->setrange(0, 0);
  this->setpos(0);
}

int GUIhscrollbar::keyhit(char kbscan, char key) {
  int result;
  if (kbscan == 75) {
    this->sendmessage(this->up, GUIMSG_HELDDOWN);
    result = 1;
  } else if (kbscan == 77) {
    this->sendmessage(this->down, GUIMSG_HELDDOWN);
    result = 1;
  } else {
    result = 0;
  }
  return result;
}

/* GUIlistbox */

GUIlistbox::GUIlistbox(GUIrect* parent, int x, int y, int xw, int iy, int iheight)
  : GUIrect(parent, x, y, x + xw, iheight * iy + y),
    depressed(false),
    numitems(0),
    items(NULL),
    itemheight(iheight),
    itemv(iy)
{
  this->timer.clear();
  this->dblclick.clear();
  this->scroll = new GUIvscrollbar(this, 0, 0, this->height());
  this->moveto(this->x2 - this->scroll->width(), this->y1);
  this->clearsel();
  this->dblclick.set(0);
}

void GUIlistbox::freeitems() {
  if (this->items) {
    free(this->items);
  }
  this->items = NULL;
  this->numitems = 0;
  this->clearsel();
}

ITEMPTR* GUIlistbox::resizeitems(int n) {
  this->items = realloc(this->items, sizeof(ITEMPTR) * n);
  while (n > this->numitems) {
    this->numitems++;
    this->items[this->numitems] = NULL;
  }
  this->numitems = n;
  if (n <= this->sel) {
    this->setsel(n - 1);
  }
  this->scroll->setrange(0, this->numitems - this->itemv);
  return this->items;
}

void GUIlistbox::clearsel() {
  this->sel = -1;
}

void GUIlistbox::setsel(int s) {
  if (s < 0) {
    s = 0;
  }
  if (s > this->numitems - 1) {
    s = this->numitems - 1;
  }
  if (s != this->sel) {
    this->sel = s;
    if (s < this->scroll->getpos()) {
      this->scroll->setpos(s);
    }
    if (this->itemv + this->scroll->getpos() <= this->sel) {
      this->scroll->setpos(this->sel - this->itemv + 1);
    }
    if (this->parent) {
      this->parent->sendmessage(this, GUIMSG_LISTBOXSELCHANGED);
    }
  }
}

GUIrect* GUIlistbox::click(class mouse &m) {
  GUIrect* result;
  if (m.click) {
    if (!this->dblclick.check()) {
      if (this->parent) {
        this->parent->sendmessage(this, GUIMSG_LISTBOXDBLCLICKED);
      }
    }
    this->dblclick.set(30);
    this->timer.set(10);
    this->depressed = true;
    result = this;
  } else {
    result = NULL;
  }
  return result;
}

int GUIlistbox::release(class mouse &m) {
  this->depressed = false;
  return 1;
}

int GUIlistbox::drag(class mouse &m) {
  return 1;
}

void GUIlistbox::draw(char* dest) {
  drawbox(
    dest,
    7,
    this->x1 - 1,
    this->y1 - 1,
    this->x2 + 1,
    this->y2 + 1);
  drawrect(
    dest,
    0,
    this->x1,
    this->y1,
    this->width(),
    this->height());
  this->drawitems(dest, this->x1 + 1, this->y1 + 1);
  GUIrect::draw(dest);
  if (this->depressed) {
    int selected_item = (m.y - this->y1) / this->itemheight;
    if (selected_item >= 0) {
      if (selected_item < this->itemv) {
        this->setsel(selected_item + this->scroll->getpos());
      } else {
        if (this->timer.check()) {
          if (this->numitems - 1 > this->sel) {
            this->setsel(this->sel + 1);
          }
          this->timer.reset();
        }
      }
    } else {
      if (this->timer.check()) {
        if (this->sel > 0) {
          this->setsel(this->sel - 1);
        }
        this->timer.reset();
      }
    }
  }
}

int GUIlistbox::keyhit(char kbscan, char key) {
  int result;
  switch (kbscan) {
    case 71:
      this->setsel(0);
      result = 1;
      break;
    case 79:
      this->setsel(this->numitems - 1);
      result = 1;
      break;
    case 73:
      this->setsel(this->sel - this->itemv);
      result = 1;
      break;
    case 81:
      this->setsel(this->itemv + this->sel);
      result = 1;
      break;
    case 72:
      if (this->sel > 0) {
        this->setsel(this->sel - 1);
      }
      result = 1;
      break;
    case 80:
      if (this->numitems - 1 > this->sel) {
        this->setsel(this->sel + 1);
      }
      result = 1;
      break;
    default:
      result = 0;
      break;
  }
  return result;
}

/* GUIstringlistbox */

void GUIstringlistbox::drawitems(char* dest, int x, int y) {
  int draw_item = this->scroll->getpos();
  int items_left = this->itemv;
  while(true) {
    if (items_left <= 0 || draw_item >= this->numitems) {
      break;
    }
    if (draw_item == this->sel) {
      drawrect(dest, 2, x, y, this->width()-2, this->itemheight);
      char* str = this->items[draw_item];
      if (str) {
        font[1]->draw(str, dest, x, y);
      }
    } else {
      char* str = this->items[draw_item];
      if (str) {
        font[1]->draw(str, dest, x, y);
      }
    }
    --items_left;
    ++draw_item;
    y += this->itemheight;
  }
}
