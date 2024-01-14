#include "kernel.h"
namespace timerd{
  int count;
  timer* front;
  __attribute__((interrupt)) void timerhandle(unsigned long long* rsp){
    count++;
    timer* t=front;
    while(t){
      if(count>=t->timeout){
        if(t->prev)
          t->prev->next=t->next;
        else
          front=t->next;
        if(t->next)
          t->next->prev=t->prev;
        t->flags|=1;
      }
      t=t->next;
    }
    io_out8(0x20, 0x60);
  }
  void init(){
    io_out8(0x43, 0x34);
    io_out8(0x40, 0x9c);
    io_out8(0x40, 0x2e);
    set_idt(0x20, (unsigned long long)timerhandle, 8, 0x8e);
    open_irq(0);
  }
};
using namespace timerd;
void timer::set(unsigned int cnt, unsigned char cy){
  unsigned int r=rflags();
  asm("cli");
  timeout=count+cnt;
  cyc=cy;
  if(front){
    timer* tm=front;
    while(tm->next)tm=tm->next;
    tm->next=this;
    prev=tm;
  }else{
    front=this;
    next=0;
  }
  srflags(r);
}
