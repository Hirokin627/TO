#include "kernel.h"
namespace layerd{
  layer** layers;
  int top=-1;
  int* sb;
  void init(){
    layers=(layer**)searchmem(sizeof(layer)*256);
    sb=(int*)searchmem(4*scrxsize*scrysize);
    layer* bl=new layer(scrxsize,scrysize);
    bl->col_inv=-1;
    graphic::drawbox(bl, 0x8484, 0, 0, scrxsize-1, scrysize-1);
    bl->updown(0);
  }
  void refreshsub(int x0, int y0, int x1, int y1){
    for(int i=0;i<=top;i++){
      layer* l=layers[i];
      int bx0=x0-l->x;
      int by0=y0-l->y;
      int bx1=x1-l->x;
      int by1=y1-l->y;
      if(bx0<0)bx0=0;
      if(by0<0)by0=0;
      if(bx1>l->bxsize)bx1=l->bxsize;
      if(by1>l->bysize)by1=l->bysize;
      for(int by=by0;by<by1;by++){
        int vy=l->y+by;
        for(int bx=bx0;bx<bx1;bx++){
          int vx=l->x+bx;
          if(l->buf[by*l->bxsize+bx]!=l->col_inv)sb[vy*scrxsize+vx]=l->buf[by*l->bxsize+bx];
        }
      }
    }
    for(int y=y0;y<y1;y++){
      for(int x=x0;x<x1;x++){
        vram[y*scrxsize+x]=sb[y*scrxsize+x];
      }
    }
  }
};
using namespace layerd;
layer::layer(int xsize,int ysize){
  buf=(unsigned int*)searchmem(4*xsize*ysize);
  bxsize=xsize;
  bysize=ysize;
  height=-1;
}
void layer::updown(int nheight){
  if(nheight<-1)nheight=-1;
  if(nheight>top+1)nheight=top+1;
  int old=height;
  if(old<nheight){
    if(old>=0){
    }else{
      for(int i=top+1;i>nheight;i--){
        layers[i]=layers[i-1];
        layers[i]->height=i;
      }
      top++;
    }
    layers[nheight]=this;
  }else if(old>nheight){
    if(nheight>=0){
    }else{
      for(int i=old;i<top;i++){
        layers[i]=layers[i+1];
        layers[i]->height=i;
      }
      top--;
    }
  }
  height=nheight;
  refresh();
}
void layer::refresh(){
  refreshsub(x, y, x+bxsize, y+bysize);
}
void layer::refreshconfro(int x0, int y0, int x1, int y1){
  refreshsub(x+x0, y+y0, x+x1, y+y1);
}
void layer::slide(int nx, int ny){
  int oldx=x;
  int oldy=y;
  x=nx;
  y=ny;
  refreshsub(oldx, oldy, oldx+bxsize, oldy+bysize);
  refresh();
}
