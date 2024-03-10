#include "../api/api.h"
class system* sys;
int main(int argc, char** argv){
  sys=new system;
  sys->puts("Window creating\n");
  sys->getwin(200, 200);
  sys->exit();
  return 0;
}
