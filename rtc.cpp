#include "kernel.h"

namespace rtcd{
  int i=0;
  unsigned short y,m,d,h,mt,s;
  unsigned char getrtcreg(unsigned char reg){
    io_out8(0x70, reg);
    return io_in8(0x71);
  }
  void writertcreg(unsigned char reg, unsigned char d){
    io_out8(0x70, reg);
    io_out8(0x71, d);
  }
  __attribute__((interrupt)) void rtchandle(int* esp){  
    getrtcreg(0x0c);
    y=getrtcreg(9) <90 ? getrtcreg(9)+2000 : getrtcreg(9)+1900;
    m=getrtcreg(8);
    d=getrtcreg(7);
    h=getrtcreg(4);
    mt=getrtcreg(2);
    s=getrtcreg(0);
    kernelbuf->write(9);
    io_out8(0x20, 0x62);
    io_out8(0xa0, 0x60);
  }
  void init(){
    set_idt(0x28, (unsigned long long)rtchandle, 8, 0x8e);
    open_irq(8);
    unsigned char b=getrtcreg(0x0b);
    //if(!(b&0x4)){
      b|=4;
    //}
    if(!(b&0x10)){
      b|=0x10;
    }
    writertcreg(0x0b, b);
  }
};
