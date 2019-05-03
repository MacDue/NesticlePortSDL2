

#define MBLEFT   1
#define MBRIGHT  2
#define MBMIDDLE 4

class mouse
{
public:
 int hidden; /*0*/
 int oldb; // 4 last mouse button state
 int oldx,oldy; // 8, 12 last coords

 int x,y; // 16, 20, current mouse position
 int b;   // 24, current holding down of mouse buttons
 char click; // 28 click state of buttons
 char rel;  // 29 release state of buttons

 class GUIrect *capture; //is the mouse captured? (in dragging)

 mouse(); //constructor

 void draw(char *dest);
 void updatexy(int newx,int newy);
 void updatebut(int newbut);
 void reset() {oldx=x; oldy=y;}
 void showcursor() {hidden=0;}
 void hidecursor() {hidden=1;}
};

extern class mouse m;
