#include "kernel.h"
namespace timerd{
  int count;
  timer* front;
  __attribute__((interrupt)) void timerhandle(unsigned long long* rsp){
    count++;
    timer* t=front;
    bool mtc=false;
    while(t){
      if(count>=t->timeout){
        if(t->prev){
          t->prev->next=t->next;
        }else{
          front=t->next;
        }
        if(t->next){
          t->next->prev=t->prev;
        }
        t->flags|=1;
        if(t==mtaskd::mt)mtc=true;
      }
      t=t->next;
    }
    io_out8(0x20, 0x60);
    //if(mtc)mtaskd::taskswitch();
  }
  void init(){
    io_out8(0x43, 0x34);
    io_out8(0x40, 0x9c);
    io_out8(0x40, 0x2e);
    set_idt(0x20, (unsigned long long)timerhandle, 8, 0x8e);
    open_irq(0);
    front=0;
  }
  void sleep(unsigned int ms10){
    unsigned int r=rflags();
    timer* t=new timer;
    t->set(ms10);
    while(!(t->flags&1))asm("sti");
    delete t;
    srflags(r);
  }
};
using namespace timerd;
void timer::set(unsigned int cnt, unsigned char cy){
  unsigned int r=rflags();
  asm("cli");
  timeout=count+cnt;
  cyc=cy;
  prev=0;
  next=0;
  if(front){
    timer* tm=front;
    while(tm->next)tm=tm->next;
    tm->next=this;
    prev=tm;
    next=0;
  }else{
    front=this;
    next=0;
    prev=0;
  }
  srflags(r);
}
