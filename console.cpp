#include "kernel.h"
console::console(int line, int row){
  lines=line;
  rows=row;
  l=new layer(line*8, row*16);
  l->updown(layerd::top+1);
}
void console::putc(char chr){
  switch(chr){
    default:
      graphic::putfont(l, 0xffffff, cx, cy, chr, true);
      cx+=8;
      break;
    case '\n':
      nline();
      break;
  }
}
void console::putsns(const char* str){
  for(int i=0;str[i]!=0;i++)putc(str[i]);
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
    graphic::drawbox(l, 0, 0, l->bysize-16, l->bxsize-1, l->bysize-1);
    l->refresh();
    cy-=16;
  }
}
void console::puts(const char* format,...){
  va_list ap;
  char s[1024];
  
  va_start(ap, format);
  vsprintf(s, format, ap);
  va_end(ap);
  
  putsns((const char*)s);
}
