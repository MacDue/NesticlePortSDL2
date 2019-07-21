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

namespace fs = std::filesystem;


static bool wildcard_match(char const * pattern, char const * string) {
  for (; *pattern; pattern++) {
    switch (*pattern) {
      case '?': {
        if (*string == '\0') {
          return false;
        }
        string++;
        break;
      }
      case '*': {
        int max = strlen(string);
        for (int i = 0; i < max; i++) {
          if (wildcard_match(pattern+1, string+i)) {
            return true;
          }
        }
        return false;
      }
      default: {
        if (*string == '\0' or tolower(*string) != *pattern) {
          return false;
        }
        string++;
      }
    }
  }
  return *string == '\0';
}

#include <iostream>

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
 for (auto const &entry : fs::directory_iterator("./")) {
   auto fn = entry.path().filename();
   const char* c_fn = fn.c_str();
   if (!fs::is_regular_file(entry.status())
        || !wildcard_match(path, c_fn)
   ) {
     continue;
   }
   if (!func(const_cast<char*>(c_fn), context)) {
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
