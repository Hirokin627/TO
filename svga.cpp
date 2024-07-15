#include "kernel.h"
namespace svgad{
  struct pcid svga;
  unsigned int iobase;
  #define NULL 0
  unsigned int* fifo=NULL;
  int wp=0;
  int size=0;
  bool sp=false;
  void writereg(unsigned int ind, unsigned int data){
    if(!sp)return;
    io_out32(iobase+0, ind);
    io_out32(iobase+1, data);
  }
  unsigned int readreg(unsigned int ind){
    if(!sp)return -1;
    io_out32(iobase+0, ind);
    return io_in32(iobase+1);
  }
  void putfifo(unsigned int d){
    if(!sp)return;
    asm("cli");
    fifo[wp]=d;
    wp++;
    if(wp*4>=fifo[1])wp=293;
    fifo[2]=wp*4;;
  }
  void init(){
    for(int i=0;i<pci::many;i++){
      if(pci::readpcidata(pci::pcis[i], 0)==0x040515ad){
        svga=pci::pcis[i];
        sp=true;
        pci::writepcidata(svga, 4, pci::readpcidata(svga, 4)|7);
        //pause();
        //pause();
          break;
      }
    }
    if(!sp)return;
    for(int i=0;i<6;i++){
        if(pci::readpcidata(svga, 4*i+0x10)&1){
          iobase=pci::readpcidata(svga, 4*i+0x10)&~3;
          break;
        }
    }
    int nxsize=1920;
    int nysize=1080;
    pci::writepcidata(svga, 4, pci::readpcidata(svga, 4)|7);
    writereg(0, 0x90000002);
    fifo=(unsigned int*)((unsigned long long)readreg(18));
    vram=(int*)readreg(13);
    size=readreg(19);
    scrysize=nysize;
    scrxsize=nxsize;
    fifo[0]=fifo[3]=fifo[2]=293*4;
    fifo[1]=size;
    fifo[5]=0;
    wp=293;
    //setcr3(0);
    writereg(1, 1);
    writereg(20, 1);
    writereg(2, nxsize);
    writereg(3, nysize);
    writereg(7, 32);
    writereg(1, 1);
    readreg(12);
    putfifo(1);
    putfifo(0);
    putfifo(0);
    putfifo(nxsize);
    putfifo(nysize);
    writereg(21, 1);
    //layerd::resetsb();
    layerd::refreshsub(0, 0, nxsize, nysize);
    
    //writereg(13, searchmemforio(4*nxsize*nysize));
    //vram=(int*)readreg(13);
  }
};
