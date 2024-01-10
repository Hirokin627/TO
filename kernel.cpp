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
  asm("sti");
  ps2::init();
  for(int i=0;i<3;i++)cns->puts("test %d\n", i);
  cns->l->updown(-1);
  layer* l=new layer(16, 16);
  l->col_inv=-1;
  static char cursor[16][17]={
		"**************..",
		"*OOOOOOOOOOO*...",
		"*OOOOOOOOOO*....",
		"*OOOOOOOOO*.....",
		"*OOOOOOOO*......",
		"*OOOOOOO*.......",
		"*OOOOOOO*.......",
		"*OOOOOOOO*......",
		"*OOOO**OOO*.....",
		"*OOO*..*OOO*....",
		"*OO*....*OOO*...",
		"*O*......*OOO*..",
		"**........*OOO*.",
		"*..........*OOO*",
		"............*OO*",
		".............***"
  };
  for(int y=0;y<16;y++){
    for(int x=0;x<16;x++){
      unsigned int c=0;
      switch(cursor[y][x]){
        case '*':
          c=0;
          break;
        case 'O':
          c=0xffffff;
          break;
        default:
          c=-1;
          break;
      }
      l->buf[y*16+x]=c;
    }
  }
  //graphic::drawbox(l, 0xffffff, 0, 0, 15, 15);
  l->updown(layerd::top+1);
  window* test=new window(200, 200);
  xhci::init();
  while(1){
    if(kernelbuf->len==0){
      asm("sti\nhlt");
    }else{
      asm("cli");
      unsigned int q=kernelbuf->read();
      if(q==0){
        unsigned char c=kernelbuf->read();
        signed int x=kernelbuf->read();
        signed int y=kernelbuf->read();
        asm("sti");
        mx=l->x+x;
        my=l->y+y;
        if(mx<0)mx=0;
        if(my<0)my=0;
        if(mx>scrxsize-1)mx=scrxsize-1;
        if(my>scrysize-1)my=scrysize-1;
        l->slide(mx, my);
      }else if(q==1){
        asm("sti");
        xhci::posthandle();
      }
    }
  }
}
