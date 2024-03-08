#include "kernel.h"
extern "C" unsigned long long apibody(struct tc* ct){
  mtaskd::current->tm->cns->putc('A');
  return 0;
}
void api_init(){ 
  set_idt(0x40, (unsigned long long)asmapihandle, 8, 0xee);
}
