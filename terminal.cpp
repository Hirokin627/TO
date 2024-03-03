#include "kernel.h"
namespace terminald{
  void main(task* tsk){
    terminal* tm=new terminal;
    tm->m(tsk);
  }
};
void terminal::m(task* t){
  asm("cli");
  cns->puts("terminal called\n");
  tsk=t;
  window* w=new window(60*8, 600);
  w->owner=t;
  cns->puts("terminal started\n");
  graphic::drawbox(w->cs, 0, 0, 0, w->cs->bxsize-1, w->cs->bysize-1);
  timer* tm=new timer;
  fifo* f=t->f;
  console* cns=new console(60, 20);
  cns->l->updown(layerd::top-1);
  cns->l->flags|=ITS_WINDOW;
  w->cs->registss(cns->l);
  cns->l->slide(1, 1);
  cns->puts("test\n");
  while(1){
    if(f->len==0){
      t->sleep();
    }else{
      int q=f->read();
      //cns->puts("q=%d\n", q);
      if(q==2){
        unsigned char k=f->read();
        if(!(k&0x80)){
          cns->puts("%c", keytable0[k]);
        }
        //cns->puts("key=%02x\n", k);
      }
    }
  }
}
