#include "../api/api.h"
class system* sys;
int main(int argc, char** argv){
  sys=new system;
  sys->putc('A');
  sys->puts("\nHIRO IS GOOD\n");
  return 0;
}
