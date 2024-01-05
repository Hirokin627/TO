#include "kernel.h"
int* vram;
int scrxsize,scrysize;
alignas(16) unsigned char stack[1024*1024];
console* cns;
fifo* kernelbuf;
extern "C" caddr_t sbrk(size_t size){
  return (caddr_t)searchmem(size);
}
extern "C" void nKernelmain(struct arg* ai){
  cli();
  asm("cli");
  vram=ai->Frame.fb;
  scrxsize=ai->Frame.xsize;
  scrysize=ai->Frame.ysize;
  memory_init(ai->mems, ai->size ,ai->bsize);
  layerd::init();
  kernelbuf=new fifo(128);
  cns=new console(60, scrysize/16);
  x64_init();
  pci::init();
  pic_init();
  for(int i=0;i<3;i++)cns->puts("test %d\n", i);
  xhci::init();
  while(1){
      sti();
      asm("sti\nhlt");
  }
}
