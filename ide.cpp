#include "kernel.h"
namespace ided{ 
  unsigned char sendcmd(unsigned char p2, unsigned char p3, unsigned char p4, unsigned char p5, unsigned char p6, unsigned char p7){
    while(io_in8(0x1f7)&0x80);
    io_out8(0x1f2, p2);
    io_out8(0x1f3, p3);
    io_out8(0x1f4, p4);
    io_out8(0x1f5, p5);
    io_out8(0x1f6, p6);
    io_out8(0x1f7, p7);
    while(io_in8(0x1f7)&0x80);
    return io_in8(0x1f7);
  }
  void init(){
    io_out8(0x3f6, 4);
    io_out8(0x3f6, 2);
    unsigned char stt=sendcmd(0, 0, 0, 0, 0xa0, 0xec);
    if(stt&1){
      cns->puts("HD not found\n");
    }else{
      for(int i=0;i<256;i++)io_in16(0x1f0);
      cns->puts("HD found\n");
      drive* ide=new idedrv(0);
      drvd::registdrv(1, 0, 0, ide);
    }
  }
};
using namespace ided;
idedrv::idedrv(unsigned char a){
  addr=a;
  type=1;
}
void idedrv::read(unsigned char* buf, unsigned int cnt, unsigned int lba512){
  for(int i=0;i<cnt;i++){
    if(sendcmd(1, lba512, lba512>>8, lba512>>16, 0xe0|(addr<<4)|((lba512>>24)&0xf), 0x20)&1)return;
    for(int j=0;j<256;j++){
      *(unsigned short*)&buf[512*i+j*2]=io_in16(0x1f0);
    }
  }
}
void idedrv::write(unsigned char* buf, unsigned int cnt, unsigned int lba512){
  for(int i=0;i<cnt;i++){
    if(sendcmd(1, lba512, lba512>>8, lba512>>16, 0xe0|(addr<<4)|((lba512>>24)&0xf), 0x30)&1)return;;
    for(int j=0;j<256;j++){
      io_out16(0x1f0, *(unsigned short*)&buf[i*512+j*2]);
    }
  }
}
