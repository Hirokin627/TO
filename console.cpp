#include "kernel.h"
console::console(int line, int row){
  lines=line;
  rows=row;
  l=new layer(line*8, row*16);
  l->flags|=ITS_CONSOLE;
  fc=0xffffff;
  bc=-1;
  graphic::drawbox(l, bc, 0, 0, l->bxsize-1, l->bysize-1);
  l->col_inv=-1;
  l->updown(layerd::top+1);
}
void console::putc(char chr, bool nf){
  switch(chr){
    default:
      graphic::putfont(l, fc, cx, cy, chr, nf);
      cx+=8;
      if(cx>=l->bxsize){
        nline();
        cx=0;
      }
      break;
    case '\b':
      if(cx>0){
        cx-=8;
        graphic::drawbox(l, bc, cx, cy, cx+7, cy+15, true);
      }
      break;
    case '\n':
      nline();
      break;
  }
}
void console::putsns(const char* str){
  int bx=cx;
  int by=cy;
  int ss=strlen(str);
  int ex=(cx+ss*8);
  int ey=(cy+16);
  if(ex>l->bxsize){
    ex=l->bxsize;
    ey+=16;
  }
  /*if(ey>l->bysize){
    ey=l->bysize;
  }*/
  for(int i=0;str[i]!=0;i++){
    putc(str[i], true);
    if(str[i]=='\n'){
      ey+=16;
    }
  }
  //l->refreshconfro(bx, by, ex, ey);
}
void console::nline(){
  cy+=16;
  cx=0;
  if(cy>=l->bysize){
    for(int y=0;y<l->bysize-16;y++){
      for(int x=0;x<l->bxsize;x++){
        l->buf[y*l->bxsize+x]=l->buf[(y+16)*l->bxsize+x];
      }
    }
    graphic::drawbox(l, bc, 0, l->bysize-16, l->bxsize-1, l->bysize-1);
    l->refresh();
    cy-=16;
  }
}
void console::puts(const char* format,...){
  //unsigned int r=rflags();
  //asm("cli");
  va_list ap;
  char s[1024];
  
  va_start(ap, format);
  vsprintf(s, format, ap);
  va_end(ap);
  
  putsns((const char*)s);
  //srflags(r);
}
