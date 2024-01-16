#include "kernel.h"
extern struct tc taska,taskb;
namespace mtask{
  #define switchcyc 2
  timer* mt;
  fifo* tasks;
  task* init(){
    mt=new timer;
    tasks=new fifo(128);
    task* ta=new task(0);
    switchcont(ta->ct, ta->ct);
    ta->run();
    mt->set(switchcyc);
    return ta;
  }
  void taskswitch(bool cc){
    mt->set(switchcyc);
    task* old=(task*)tasks->read();
    if(!cc)tasks->write((unsigned long long)old);
    task* n=(task*)tasks->front();
    switchcont(n->ct, old->ct);
  }
};
using namespace mtask;
task::task(unsigned long long rip){
  ct=new struct tc;
  ct->cr3=(unsigned long long)getcr3();
  ct->rsp=searchmem(1024)+1024-8;
  ct->cs=8;
  ct->ss=0x10;
  ct->rflags=0x202;
  ct->rip=rip;
  *(unsigned int*)&ct->fx_area[24]=0x1f80;
}
void task::run(){
  unsigned int r=rflags();
  asm("cli");
  tasks->write((unsigned long long)this);
  srflags(r);
}
