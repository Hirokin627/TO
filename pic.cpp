#include "kernel.h"

    extern unsigned char mpb;
    extern unsigned char spb;
void pic_init(){
  cns->puts("initialing pic...\n");
  mpb=0xff;
  spb=0xff;
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
    mpb&=~(1<<irq);
    io_out8(0x21, mpb);
  }else{
    open_irq(2);
    spb&=~(1<<(irq-8));
    io_out8(0xa1, spb);
  }
}
