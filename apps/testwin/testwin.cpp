#include "../api/api.h"
class system* sys;
int main(int argc, char** argv){
  sys=new system;
  sys->getwin(200, 200);
  return 0;
}
