#include "kernel.h"
int* vram;
int scrxsize,scrysize;
alignas(16) unsigned char stack[1024*1024];
console* cns;
fifo* kernelbuf;
extern "C" caddr_t sbrk(size_t size){
  return (caddr_t)searchmem(size);
}
int mx,my;
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
  //cns->l->updown(-1);
  layer* l=new layer(16, 16);
  graphic::drawbox(l, 0xffffff, 0, 0, 15, 15);
  l->updown(layerd::top+1);
  xhci::init();
  while(1){
    if(kernelbuf->len==0){
      sti();
      asm("sti\nhlt");
    }else{
      cli();
      asm("cli");
      unsigned int q=kernelbuf->read();
      if(q==0){
        unsigned char c=kernelbuf->read();
        signed int x=kernelbuf->read();
        signed int y=kernelbuf->read();
        mx=l->x+x;
        my=l->y+y;
        if(mx<0)mx=0;
        if(my<0)my=0;
        if(mx>scrxsize-1)mx=scrxsize-1;
        if(my>scrysize-1)my=scrysize-1;
        l->slide(mx, my);
      }else if(q==1){
        xhci::posthandle();
      }
    }
  }
}
