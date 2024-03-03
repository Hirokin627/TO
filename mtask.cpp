#include "kernel.h"
extern struct tc taska,taskb;
namespace mtaskd{
  #define switchcyc 2
  timer* mt=0;
  task* current;
  fifo* tasks;
  struct tc* ctp=&taska;
  void onlyhlt(){
    while(1)asm("sti\nhlt");
  }
  task* init(){
    mt=new timer;
    tasks=new fifo(128);
    ctp=&taska;
    task* t=new task(0);
    switchcont(t->ct, t->ct);
    current=t;
    t->run();
    task* oh=new task((unsigned long long)onlyhlt);
    //oh->run();
    switchcont(&taska, &taska);
    mt->set(switchcyc);
    return t;
  }
  void taskswitch(bool cc){
    mt->set(switchcyc);
    task* t=(task*)tasks->read();
    if(!cc)tasks->write((unsigned long long)t);
    task* n=(task*)tasks->front();
    current=n;
    switchcont(n->ct, t->ct);
  }
};
using namespace mtaskd;
void task::run(){
  unsigned int r=rflags();
  asm("cli");
  tasks->write((unsigned long long)this);
  srflags(r);
}
void task::sleep(){
  unsigned int r=rflags();
  asm("cli");
  if(current==this){
    taskswitch(true);
  }else{
    int size=tasks->len;
    for(int i=0;i<size;i++){
      unsigned long long t=tasks->read();
      if(t!=(unsigned long long)this)tasks->write(t);
    }
  }
  srflags(r);
}
task::task(unsigned long long rip){
  ct=new struct tc;
  ct->rip=rip;
  ct->cs=8;
  ct->ss=0x10;
  ct->rflags=0x202;
  ct->rsp=searchmem(1024)+1024-8;
  ct->cr3=(unsigned long long)getcr3();
  *(unsigned int*)&ct->fx_area[24]=0x1f80;
  f=new fifo(128, this);
}
