#include "kernel.h"
void initsheet(layer* l){
  allocpage(getcr3(), (unsigned long long)l->buf, (unsigned long long)l->buf, 4*l->bxsize*l->bysize, 7);
}
extern "C" unsigned long long apibody(struct tc* ct){
  asm("cli");
  if(ct->rax==0){
    mtaskd::current->tm->cns->putc(ct->rdi);
  }else if(ct->rax==1){
    mtaskd::current->tm->cns->puts((const char*)getpaddr(getcr3(), ct->rdi));
    mtaskd::current->tm->cns->l->refresh();
  }else if(ct->rax==2){
    unsigned long long size=(ct->rdi+0xfff)&~0xfff;
    unsigned long long addr=searchmem(size);
    allocpage(getcr3(), addr, addr, size, 7);
    return addr;
  }else if(ct->rax==3){
    window* w=new window(ct->rdi, ct->rsi);
    return (unsigned long long)w;
  }
  return 0;
}
void api_init(){ 
  set_idt(0x40, (unsigned long long)asmapihandle, 8, 0xee);
}
