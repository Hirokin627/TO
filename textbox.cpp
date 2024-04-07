#include "kernel.h"

textbox::textbox(unsigned int csize){
  l=new layer(csize*8+4, 19);
  l->col_inv=-1;
  l->flags|=ITS_TEXTBOX;
  l->wc=(window*)this;
  using namespace graphic;
  drawbox(l, 0xffffff, 0, 0, l->bxsize-1, l->bysize-1);
  drawbox(l, 0x848484, 0, 0, l->bxsize-1, 0);
  drawbox(l, 0x848484, 0, 0, 0, l->bysize-1);
  drawbox(l, 0x000000, 1, 1, l->bxsize-2, 1);
  drawbox(l, 0x000000, 1, 1, 1, l->bysize-2);
  drawbox(l, 0xc6c6c6, 2, l->bysize-2, l->bxsize-2, l->bysize-2);
  drawbox(l, 0xc6c6c6, l->bxsize-2, 2, l->bxsize-2, l->bysize-2);
  l->updown(layerd::top);
  c=new console(csize, 1);
  c->fc=0;
  c->l->wc=(window*)this;
  c->l->flags|=ITS_TEXTBOX;
  c->l->updown(l->height+1);
  c->l->slide(2, 2);
  l->registss(c->l);
}
