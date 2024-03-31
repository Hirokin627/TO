#include "kernel.h"
fifo::fifo(int sz, task* t){
  size=sz;
  len=0;
  rp=wp=0;
  datas=(unsigned long long*)searchmem(sz*8);
  tsk=t;
}
void fifo::write(unsigned long long d){
  //asm("cli");
  if(!lock){
    lock=1;
    datas[wp]=d;
    wp++;
    if(wp==size)wp=0;
    len++;
    lock=0;
    //if(len==size)asm("cli\nhlt");
    if(tsk)tsk->run();
  }else{
    cns->puts("FIFO LOCK ERROR\n");
    asm("cli\nhlt");
  }
}
unsigned long long fifo::read(){
  unsigned long long d=datas[rp];
  rp++;
  if(rp==size)rp=0;
  len--;
  return d;
}
