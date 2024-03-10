#include "kernel.h"
namespace graphic{
  void drawrect(layer* l, int c, int x0, int y0, int x1, int y1){
    drawbox(l, c, x0, y0, x1, y0);
    drawbox(l, c, x0, y0, x1, y1);
    drawbox(l, c, x0, y1, x1, y1);
    drawbox(l, c, x1, y0, x1, y1);
  }
};
void window::setactive(bool ac){
  unsigned int c=0x84;
  if(!ac)c=0x848484;
  graphic::drawbox(tb, c, 0, 0, tb->bxsize-1, tb->bysize-1);
}
window::window(int cxsize, int cysize){
  if(nowb){
    nowb->setactive(false);
    nowb->cs->refresh();
  }
  cs=new layer(cxsize, cysize);
  cs->col_inv=-1;
  cs->flags|=ITS_WINDOW|ITS_CS;
  cs->wc=this;
  graphic::drawbox(cs, 0xc6c6c6, 0, 0, cxsize-2, cysize-1);
  cs->updown(layerd::top);
  cs->slide(1, 21);
  edge=new layer(cxsize+2, cysize+3+20);
  edge->col_inv=-1;
  edge->flags|=ITS_WINDOW;
  int x=edge->bxsize;
  int y=edge->bysize;
  graphic::drawbox(edge, -1, 0, 0, x-1, y-1);
  graphic::drawbox(edge, 0xc6c6c6, 2, 2, x-2, 23);
  graphic::drawbox(edge, 0xc6c6c6, 0, 0, x-1, 0);
  graphic::drawbox(edge, 0xffffff, 1, 1, x-2, 1);
  graphic::drawbox(edge, 0xc6c6c6, 0, 0, 0, y-1);
  graphic::drawbox(edge, 0xffffff, 1, 1, 1, y-2);
  graphic::drawbox(edge, 0x848484, x-2, 1, x-2, y-2);
  graphic::drawbox(edge, 0x000000, x-1, 0, x-1, y-1);
  //graphic::drawbox(edge, 0x000084, 3, 3, x-3, 20);
  graphic::drawbox(edge, 0x848484, 1, y-2, x-2, y-2);
  graphic::drawbox(edge, 0x000000, 0, y-1, x-1, y-1);
  edge->updown(cs->height+1);
  cs->registss(edge);
  tb=new layer(cxsize-4, 20);
  tb->flags|=ITS_WINDOW|ITS_TB;
  tb->col_inv=-1;
  graphic::drawbox(tb, 0x84, 0, 0, tb->bxsize-1, tb->bysize-1);
  tb->updown(cs->height);
  tb->slide(3, 3);
  cs->registss(tb);
  cs->slide(200, 200);
  nowb=this;
  cs->updown(layerd::top-1);
}
