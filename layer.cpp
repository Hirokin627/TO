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
    graphic::drawbox(bl, 0xcc2528, 0, 0, scrxsize-1, scrysize-1);
    graphic::drawbox(bl, 0xc6c6c6, 0, scrysize-28, scrxsize-1, scrysize-28);
    graphic::drawbox(bl, 0xffffff, 0, scrysize-27, scrxsize-1, scrysize-27);
    graphic::drawbox(bl, 0xc6c6c6, 0, scrysize-26, scrxsize-1, scrysize-1);
    
    graphic::drawbox(bl, 0xffffff, 3, scrysize-24, 59, scrysize-24);
    graphic::drawbox(bl, 0xffffff, 2, scrysize-24, 2, scrysize-4);
    graphic::drawbox(bl, 0x848484, 3, scrysize-4, 59, scrysize-4);
    graphic::drawbox(bl, 0x848484, 59, scrysize-23, 59, scrysize-5);
    graphic::drawbox(bl, 0x000000, 2, scrysize-3, 59, scrysize-3);
    graphic::drawbox(bl, 0x000000, 60, scrysize-24, 60, scrysize-3);
    
    graphic::drawbox(bl, 0x848484, scrxsize-47, scrysize-24, scrxsize-4, scrysize-24);
    graphic::drawbox(bl, 0x848484, scrxsize-47,scrysize-23, scrxsize-47, scrysize-4);
    graphic::drawbox(bl, 0xffffff, scrxsize-47, scrysize-3, scrxsize-4, scrysize-3);
    graphic::drawbox(bl, 0xffffff, scrxsize-3, scrysize-24, scrxsize-3, scrysize-3);
    bl->updown(0);
  }
  unsigned int mixc(unsigned int bc, unsigned int fc, unsigned char i){
  }
  void trefreshsub(int x0, int y0, int x1, int y1){
    if(x0<0)x0=0;
    if(y0<0)y0=0;
    if(x1>scrxsize)x1=scrxsize;
    if(y1>scrysize)y1=scrysize;
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
  void refreshsub(int x0, int y0, int x1, int y1){
    //trefreshsub(x0, y0, x1, y1);
    if(mtaskd::mt&&(mtaskd::current!=ta)){
      kernelbuf->write(7);
      kernelbuf->write(x0);
      kernelbuf->write(y0);
      kernelbuf->write(x1);
      kernelbuf->write(y1);
      mtaskd::taskswitch();
      //asm("cli\nhlt");
    }else{
      trefreshsub(x0, y0, x1, y1);
    }
  }
  layer* checkcrick(int mx, int my){
    for(int i=top-1;i>0;i--){
      layer* l=layers[i];
      int vx=mx-l->x;
      int vy=my-l->y;
      if(vx>=0&&vx<l->bxsize){
        if(vy>=0&&vy<l->bysize){
          return l;
        }
      }
    }
    return 0;
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
  if(master){
    int noff=nheight;
    nheight+=master->height;
    if(old>=master->height)nheight++;
  }
  if(old<nheight){
    if(old>=0){
      for(int i=old;i<nheight;i++){
        layers[i]=layers[i+1];
        layers[i]->height=i;
      }
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
      for(int i=old;i>nheight;i--){
        layers[i]=layers[i-1];
        layers[i]->height=i;
      }
      layers[nheight]=this;
    }else{
      for(int i=old;i<top;i++){
        layers[i]=layers[i+1];
        layers[i]->height=i;
      }
      top--;
    }
  }
  height=nheight;
  for(int i=0;i<manye;i++){
    slaves[i]->updown(i);
  }
  refresh();
}
void layer::registss(layer* s){
  slaves[manye]=s;
  manye++;
  s->master=this;
}
void layer::refresh(){
  refreshsub(x, y, x+bxsize, y+bysize);
}
void layer::refreshconfro(int x0, int y0, int x1, int y1){
  refreshsub(x+x0, y+y0, x+x1, y+y1);
}
void layer::slide(int nx, int ny){
  if(master){
    nx+=master->x;
    ny+=master->y;
  }
  int oldx=x;
  int oldy=y;
  x=nx;
  y=ny;
  for(int i=0;i<manye;i++){
    slaves[i]->slide(slaves[i]->x-oldx, slaves[i]->y-oldy);
  }
  refreshsub(oldx, oldy, oldx+bxsize, oldy+bysize);
  refresh();
}
