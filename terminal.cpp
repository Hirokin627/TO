#include "kernel.h"
namespace terminald{
  void main(task* tsk){
    terminal* tm=new terminal;
    tm->m(tsk);
  }
};
void terminal::m(task* t){
  cns->puts("terminal stated\n");
  tsk=t;
  window* w=new window(600, 600);
  graphic::drawbox(w->cs, 0, 0, 0, w->cs->bxsize-1, w->cs->bysize-1);
  timer* tm=new timer;
  t->sleep();
}
