#include "kernel.h"

void pic_init(){
  cns->puts("initialing pic...\n");
  io_out8(0x21, 0xff);
  io_out8(0xa1, 0xff);
  
  io_out8(0x20, 0x11);
  io_out8(0x21, 0x20);
  io_out8(0x21, 0x04);
  io_out8(0x21, 0x01);
  
  io_out8(0xa0, 0x11);
  io_out8(0xa1, 0x28);
  io_out8(0xa1, 0x2);
  io_out8(0xa1, 0x01);
  
  io_out8(0x21, 0xfb);
  io_out8(0xa1, 0xff);
}

void open_irq(char irq){
  if(irq<8){ 
    extern unsigned char mpb;
    mpb&=~(1<<irq);
    io_out8(0x21, mpb);
  }else{
    extern unsigned char spb;
    open_irq(2);
    spb&=~(1<<(irq-8));
    io_out8(0xa1, spb);
  }
}
