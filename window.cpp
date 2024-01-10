#include "kernel.h"
namespace graphic{
  void drawrect(layer* l, int c, int x0, int y0, int x1, int y1){
    drawbox(l, c, x0, y0, x1, y0);
    drawbox(l, c, x0, y0, x1, y1);
    drawbox(l, c, x0, y1, x1, y1);
    drawbox(l, c, x1, y0, x1, y1);
  }
};
window::window(int cxsize, int cysize){
  cs=new layer(cxsize, cysize);
  cs->col_inv=-1;
  graphic::drawbox(cs, 0xc6c6c6, 0, 0, cxsize-1, cysize-1);
  cs->updown(layerd::top);
  cs->slide(2, 21);
  edge=new layer(cxsize+4, cysize+3+20);
  edge->col_inv=-1;
  int x=edge->bxsize;
  int y=edge->bysize;
  graphic::drawbox(edge, -1, 0, 0, x-1, y-1);
  graphic::drawbox(edge, 0xc6c6c6, 0, 0, x-1, 0);
  graphic::drawbox(edge, 0xffffff, 1, 1, x-2, 1);
  graphic::drawbox(edge, 0xc6c6c6, 0, 0, 0, y-1);
  graphic::drawbox(edge, 0xffffff, 1, 1, 1, y-2);
  graphic::drawbox(edge, 0x848484, x-2, 1, x-2, y-2);
  graphic::drawbox(edge, 0x000000, x-1, 0, x-1, y-1);
  graphic::drawbox(edge, 0x000084, 3, 3, x-4, 20);
  graphic::drawbox(edge, 0x848484, 1, y-2, x-2, y-2);
  graphic::drawbox(edge, 0x000000, 0, y-1, x-1, y-1);
  edge->updown(cs->height);
  cs->registss(edge);
  tb=new layer(cxsize-3, 20);
  tb->col_inv=-1;
  graphic::drawbox(tb, 0x84, 0, 0, tb->bxsize-1, tb->bysize-1);
  tb->updown(cs->height);
  tb->slide(3, 3);
  cs->registss(tb);
  cs->slide(200, 200);
  cs->updown(layerd::top-1);
}