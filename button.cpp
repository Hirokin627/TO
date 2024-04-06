#include "kernel.h"
button::button(int xsize, int ysize, const char* name){
  l=new layer(xsize, ysize);
  l->col_inv=-1;
  graphic::drawbox(l, 0xc6c6c6, 0, 0, xsize-1, ysize-1);
  using namespace graphic;
  drawbox(l, 0xffffff, 0, 0, 0, ysize-1);
  drawbox(l, 0xffffff, 0, 0, xsize-1, 0);
  drawbox(l, 0, xsize-1, 0, xsize-1, ysize-1);
  drawbox(l, 0, 0, ysize-1, xsize-1, ysize-1);
  drawbox(l, 0x848484, 1, ysize-2, xsize-2, ysize-2);
  drawbox(l, 0x848484, xsize-2, 1, xsize-2, ysize-2);
  if(name)putfontstr(l, 0, 3, 0, name);
  l->updown(layerd::top);
}
