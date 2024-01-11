#include "kernel.h"
namespace timer{
  int i;
  __attribute__((interrupt)) void timerhandle(unsigned long long* esp){
    vram[i]=0;
    i++;
    io_out8(0x20, 0x60);
  }
  void init(){
    set_idt(0x20, (unsigned long long)timerhandle, 8, 0x8e);
    open_irq(0);
  }
};
