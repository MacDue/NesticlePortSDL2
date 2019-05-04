/*
  GUIRECT.CPP -- Manually decompiled/reimplemented from GUI.LIB by MacDue.
  Variable names & code structure are best guesses.
  Mistakes are possible.
*/

#include <stdlib.h>
#include "guirect.h"
#include "mouse.h"
#include "guiroot.h"
#include "config.h"
#include "r2img.h"
#include "dd.h"
#include "keyb.h"


GUIrect *GUIrect::modal = NULL;
void GUIrect::setmodal(GUIrect* modal) {
  GUIrect::modal = modal;
  m.capture = modal;
}


GUIrect::GUIrect(GUIrect* parent, int x1, int y1, int x2, int y2) {
  if (x2 <= x1) {
    this->x1 = x2;
    this->x2 = x1;
  } else {
    this->x1 = x1;
    this->x2 = x2;
  }
  if (y2 <= y1) {
    this->y1 = y2;
    this->y2 = y1;
  } else {
    this->y2 = y1;
    this->y1 = y2;
  }
  this->parent = NULL;
  this->child = this->lastchild = NULL;
  this->prev = this->next = NULL;
  this->focus = 0;
  this->lastfocus = NULL;
  reparent(parent);
}


void GUIrect::reparent(GUIrect* p) {
  if (this->parent) {
    unlink();
  }
  this->parent = p;
  if (p) {
    moverel(p->x1, p->y1);
    link(NULL); // Why tho???
    bringtofront();
  }
}


void GUIrect::moverel(int dx, int dy) {
  this->x1 += dx;
  this->x2 += dx;
  this->y1 += dy;
  this->y2 += dy;
  for (GUIrect* c = this->child; c; c = c->next) {
    c->moverel(dx, dy);
  }
}


void GUIrect::moveto(int x, int y) {
  moverel(x - this->x1, y - this->y1);
}


void GUIrect::draw(char* dest) {
  for (GUIrect* c = this->lastchild; c; c = this->prev) {
    /* TODO is this right so far */
    c->draw(dest);
  }
}


void GUIrect::link(GUIrect* t) {
  /* TODO check */
  if (this->parent) {
    this->prev = t;
    if (t) {
      this->next = this->prev->next;
    } else {
      this->next = this->parent->child;
    }
    if (this->prev) {
      this->prev->next = this;
    } else {
      this->parent->child = this;
    }
    if (this->next) {
      this->next->prev = this;
    } else {
      this->parent->lastchild = this;
    }
  }
}


void GUIrect::unlink() {
  /* TODO: check */
  if (this->parent) {
    if (this->prev) {
      this->prev->next = this->next;
    } else {
      this->parent->child = this->next;
    }
    if (this->next) {
      this->next->prev = this->prev;
    } else {
      this->parent->lastchild = this->prev;
    }
    this->next = NULL;
    this->prev = NULL;
  }
}


GUIrect* GUIrect::hittest(int x, int y) {
  /* Note rect structure seems to be:
            -----------x1,y1
            |           |
          x2,y2----------
  */
  GUIrect* hit;
  if (this->x1 <= x /* test seems wrong */
    && this->x2 >= x
    && this->y1 <= y
    && this->y2 >= y
  ) {
    /* Inside this */
    if (this->child) {
      GUIrect* c = this->child;
      while (true) {
        hit = c->hittest(x,y);
        if (hit) {
          break;
        }
        c = c->next;
        if (!c) {
          goto hittest__no_children_left;
        }
      }
    } else {
hittest__no_children_left:
      if (this == guiroot) {
        this->losechildfocus();
        hit = NULL;
      } else {
        hit = this;
      }
    }
  } else {
    hit = NULL;
  }
  return hit;
}


void GUIrect::bringtofront() {
  if (this->parent) {
    if (this->prev) {
      unlink();
      link(NULL);
    }
  }
}


void GUIrect::sendtoback() {
  if (this->parent) {
    if (this->next) {
      unlink();
      link(this->lastchild);
    }
  }
}


int GUIrect::keyhit(char kbscan, char key) {
  int result;
  if (this->focus) {
    GUIrect* c = this->child;
    if (c) {
      while (!c->focus || !c->keyhit(kbscan, key)) {
        c = c->next;
        if (!c) {
          goto keyhit__no_more_children;
        }
      }
      result = 1;
    } else {
keyhit__no_more_children:
      result = 0;
    }
  } else {
    result = 0;
  }
  return result;
}


void GUIrect::losefocus() {
  this->focus = 0;
}


void GUIrect::receivefocus() {
  this->focus = this->acceptfocus();
  if (this->parent) {
    this->parent->lastfocus = this;
  }
}


/*static*/ int GUIrect::setfocus(GUIrect* f) {
  /* Confusing logic */
  int result;
  GUIrect* that = f;
  if (f) {
    root->losechildfocus();
    while (that && !that->acceptfocus()) {
      that = that->parent;
    }
    while (that && that->lastfocus) {
      that = that->lastfocus;
    }
    for (; that; that = that->parent) {
      that->receivefocus();
    }
    result = 0; /*???*/
  } else {
    result = 0;
  }
  return result;
}


void GUIrect::losechildfocus() {
  if (this->focus) {
    this->losefocus();
    for (GUIrect* c = this->child; c; c = c->next) {
      c->losechildfocus();
    }
  }
}


void GUIrect::cyclefocus(int dir) {
  if (this->child) {
    for (GUIrect* c = this->child; c; c = c->next) {
      c->losechildfocus();
    }
    GUIrect* piorfocus = this->lastfocus;
    while (true) {
      if (dir) {
        piorfocus = piorfocus ? piorfocus->next : this->child;
      } else {
        piorfocus = piorfocus ? piorfocus->prev : this->lastchild;
      }
      if (piorfocus) {
        if (piorfocus->acceptfocus()) {
          break;
        }
      }
      if (piorfocus == this->lastfocus) {
        return;
      }
    }
    if (piorfocus == this->lastfocus) {
      return;
    }
  }
}


void GUIrect::fill(char color) {
  if (cfg->get(CFG_FILLEDDLG)) {
    drawrect(
      screen,
      color,
      this->x1,
      this->y1,
      this->width(),
      this->height());
  }
}


void GUIrect::outline(char color) {
  drawbox(screen, color, this->x1, this->y1 - 1, this->x2, this->y2);
}


GUIrect::~GUIrect() {
  /* This is pretty much a verbarium copy inc compiler odd bits */
  if (this == GUIrect::modal) {
    GUIrect::setmodal(NULL);
  }
  if (this->parent && this == this->parent->lastfocus) {
    this->parent->lastfocus = NULL;
  }
  while (true) {
    /* This part might be wrong */
    if (!this->child) {
      break;
    }
    delete this->child; // call destructor
  }
  this->unlink();
}


/* -------------- GUIroot --------------  */

GUIroot::GUIroot(ROOT* root)
  : GUIrect(root, 0, 0, root->x2 - root->x1, root->y2 - root->y1)
{
  guiroot = this;
}


GUIroot::~GUIroot() {
  guiroot = NULL;
}


int GUIroot::keyhit(char kbscan, char key) {
  int result;
  if (kbstat & 2 && kbscan == 15 /*TODO figure out meaning*/) {
    this->cyclefocus(0);
    result = 1;
  } else {
    result = GUIrect::keyhit(kbscan, key);
  }
  return result;
}
