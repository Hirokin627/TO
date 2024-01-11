#include "kernel.h"
namespace ps2{
  __attribute__((interrupt)) void keyhandle(unsigned long long* esp){
    kernelbuf->write(2);
    kernelbuf->write(io_in8(0x60));
    io_out8(0x20, 0x61);
  }
  unsigned char md[3];
  unsigned int mp=0;
  int initphase=0;
  void waitforkbc(){
    while(io_in8(0x64)&2);
  }
  __attribute__((interrupt)) void mousehandle(unsigned long long* esp){
    unsigned char d=io_in8(0x60);
    if(mp==0){
      if(d==0xfa){
        if(initphase==-1){
          mp=1;
        }else if(initphase==0){
          waitforkbc();
          io_out8(0x64, 0xd4);
          waitforkbc();
          io_out8(0x60, 0xf3);
          waitforkbc();
          io_out8(0x64, 0xd4);
          waitforkbc();
          io_out8(0x60, 60);
          initphase=-1;
        }
      }
    }else if(mp==1){
      if((d&0xc8)==8){
        md[0]=d;
        mp=2;
      }
    }else if(mp==2){
      md[1]=d;
      mp=3;
    }else if(mp==3){
      md[2]=d;
      kernelbuf->write(0);
      unsigned int px=md[1];
      unsigned int py=md[2];
      if(md[0]&0x10)px|=~0xff;
      if(md[0]&0x20)py|=~0xff;
      kernelbuf->write(md[0]);
      kernelbuf->write(px);
      kernelbuf->write(-py);
      mp=1;
    }
    io_out8(0xa0, 0x64);
    io_out8(0x20, 0x62);
  }
  void init(){
    set_idt(0x21, (unsigned long long)keyhandle, 8, 0x8e);
    set_idt(0x2c, (unsigned long long)mousehandle, 8, 0x8e);
    open_irq(1);
    open_irq(12);
    waitforkbc();
    io_out8(0x64, 0x60);
    waitforkbc();
    io_out8(0x60, 0x47);
    waitforkbc();
    io_out8(0x64, 0xd4);
    waitforkbc();
    io_out8(0x60, 0xf4);
    waitforkbc();
  }
};
