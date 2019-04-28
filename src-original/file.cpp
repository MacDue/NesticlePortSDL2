#include <sys\types.h>
#include <sys\stat.h>
#include <fcntl.h>
#include <io.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "file.h"

#ifndef _MSC_VER

#include <dir.h>

#ifdef __WATCOMC__
#include <dos.h>
#endif

//dir shit
void enumdir(char *path, DIRFUNCPTR func,void *context)
{
 #ifdef __BORLANDC__
 ffblk ff;
 int done=findfirst(path,&ff,0);
 while (!done)
  {
   if (!func(ff.ff_name,context)) break;
   done=findnext(&ff);
  }
 #endif
 #ifdef __WATCOMC__
 find_t ff;
 int done=_dos_findfirst(path,0,&ff);
 while (!done)
  {
   if (!func(ff.name,context)) break;
   done=_dos_findnext(&ff);
  }
 _dos_findclose(&ff);
 #endif
}

#endif




//---------------------------------------------
//---------------------------------------------
#include "message.h"

int FILEIO::open(char *filename)
{
 if (h) close(); //already open
 h=::open(filename,O_RDONLY|O_BINARY);
 if (h==-1) {h=0; return -1;} //error
// if (h==0) msg.error("SHITTTTTTT!!!!!");
 return 0;
}

int FILEIO::create(char *filename)
{
 if (h) close();
 #ifdef __WATCOMC__
 h=::creat(filename,S_IWRITE);
 #else creat
 h=_rtl_creat(filename,0);
 #endif
 if (h==-1) {h=0; return -1;} //error
 return 0;
}

void FILEIO::close()
{
 if (!h) return;
 ::close(h);
 h=0;
}

int FILEIO::read(void *t,unsigned size)
{
 if (!h) return -1;
 return ::read(h,t,size)<size ? -1 : 0 ;
}

int FILEIO::write(void *t,unsigned size)
{
 if (!h) return -1;
 return ::write(h,t,size)<size ? -1 : 0;
}

unsigned FILEIO::size()
{
 return h ? filelength(h) : 0;
}

unsigned FILEIO::getpos()
{
 return h ? lseek(h,0,SEEK_CUR) : 0;
}

void FILEIO::setpos(unsigned p)
{
 if (h) lseek(h,p,SEEK_SET);
}



