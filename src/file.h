#ifndef _FILEIO_
#define _FILEIO_

#include <fstream>

//file input/output wrapper functions
class FILEIO
{
  std::ios_base::openmode m_current_openmode;
  std::fstream h;
 public:
  FILEIO() {}
  int open(char *filename);
  FILEIO(char *file) {open(file);}
  ~FILEIO() {close();}
  int create(char *filename);
  void close();
  int read(void *t,unsigned size);
  int write(void *t,unsigned size);

 //allocates memory and reads from a file
  void *readalloc(unsigned num)
  {
   void *t=malloc(num);
   if (!t) return 0; //malloc failed
   if (read(t,num)) {free(t); return 0;} //read failed
   return t;
  }

  int readint() {int t=0; read(&t,sizeof(int)); return t;}
  void writeint(int x) {write(&x,sizeof(int));}

/*  void writeint(int x);
  int readint();
  void *readalloc(unsigned num);*/

  unsigned size(); //returns file size
  unsigned getpos(); //returns file position
  void     setpos(unsigned int p); //sets file position

};


//function used to enumerate all files in a pathspec
typedef int (*DIRFUNCPTR)(char *filename,void *context);
void enumdir(char *path,DIRFUNCPTR func,void *context);


#endif
