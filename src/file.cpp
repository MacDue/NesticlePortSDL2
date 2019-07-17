// #include <sys\types.h>
// #include <sys\stat.h>
#include <fcntl.h>
// #include <io.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <cstdint>
#include <filesystem>

#include "file.h"

#ifndef _MSC_VER

// #include <dir.h>

#ifdef __WATCOMC__
// #include <dos.h>
#endif

//dir shit
void enumdir(char *path, DIRFUNCPTR func,void *context)
{
 // #ifdef __BORLANDC__
 // ffblk ff;
 // int done=findfirst(path,&ff,0);
 // while (!done)
 //  {
 //   if (!func(ff.ff_name,context)) break;
 //   done=findnext(&ff);
 //  }
 // #endif
 // #ifdef __WATCOMC__
 // find_t ff;
 // int done=_dos_findfirst(path,0,&ff);
 // while (!done)
 //  {
 //   if (!func(ff.name,context)) break;
 //   done=_dos_findnext(&ff);
 //  }
 // _dos_findclose(&ff);
 // #endif
 //TODO: Make work with glob path
 for (auto const &entry : std::filesystem::directory_iterator("./")) {
   if (!func(entry.path().filename().c_str(), context)) {
     break;
   }
 }
}

#endif


//---------------------------------------------
//---------------------------------------------
#include "message.h"

int FILEIO::open(char *filename)
{
 if (h.is_open()) close(); //already open
 m_current_openmode = std::ios::in;
 h.open(filename,std::ios::binary | m_current_openmode);
 if (h.fail()) {h.close(); return -1;} //error
// if (h==0) msg.error("SHITTTTTTT!!!!!");
 return 0;
}

int FILEIO::create(char *filename)
{
 if (h.is_open()) close();
 m_current_openmode = std::ios::out;
 h.open(filename, std::ios::binary | m_current_openmode);
 if (h.fail()) {h.close(); return -1;} //error
 return 0;
}

void FILEIO::close()
{
 if (!h.is_open()) return;
 h.close();
}

int FILEIO::read(void *t,unsigned size)
{
 if (!h.is_open()) return -1;
 return !h.read(static_cast<char*>(t),size) ? -1 : 0;
}

int FILEIO::write(void *t,unsigned size)
{
 if (!h.is_open()) return -1;
 return !h.write(static_cast<const char*>(t),size) ? -1 : 0;
}

unsigned FILEIO::size()
{
 if (!h.is_open()) {
  return 0;
 }
 unsigned current_pos = getpos();
 h.seekg(0, std::ios::end);
 unsigned size = h.tellg();
 setpos(current_pos);
 return size;
}

unsigned FILEIO::getpos()
{
 return h.is_open() ? static_cast<unsigned>(h.tellg()) : 0;
}

void FILEIO::setpos(unsigned p)
{
 if (h.is_open()) {
  h.clear();
  h.seekg(p, std::ios::beg);
 }
}
