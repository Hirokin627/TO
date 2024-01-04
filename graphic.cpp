#include "kernel.h"
#include "font.h"
namespace graphic{
  void drawbox(layer* l, int c, int x0, int y0, int x1, int y1, bool nf){
    for(int y=y0;y<=y1;y++){
      for(int x=x0;x<=x1;x++){ 
        l->buf[l->bxsize*y+x]=c;
      }
    }
    if(nf)l->refreshconfro(x0, y0, x1+1, y1+1);
  }
  void putfont(layer* l, int c, int bx, int by, char chr, bool nf){
    for(int y=0;y<16;y++){
      for(int x=0;x<8;x++){
        if(font_map[chr][y][x])l->buf[(y+by)*l->bxsize+x+bx]=c;
      }
    }
    if(nf)l->refreshconfro(bx, by, bx+8, by+16);
  }
  void putfontstr(layer* l, int c, int bx, int by, const char* str, bool nf){
    for(int i=0;str[i]!=0;i++,bx+=8){
      putfont(l, c, bx, by, str[i]);
    }
  }
};
