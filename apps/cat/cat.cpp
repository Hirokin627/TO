#include "../api/api.h"
class system* sys;
int main(int argc, char** argv){
  sys=new system;
  if(argc<2){
    sys->puts("Usage: (file name)\n");
    sys->exit();
  }
  file* f=sys->openf((const char*)argv[argc-1]);
  if(!f){
    sys->puts("File not found\n");
    sys->exit();
  }
  f->cnt=0;
  for(int i=0;i<f->size;i++)sys->putc(f->base[i]);
  closef(f);
  sys->exit();
  return 0;
}
