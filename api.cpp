#include "kernel.h"
void initsheet(layer* l){
  allocpage(getcr3(), (unsigned long long)l->buf, (unsigned long long)l->buf, 4*l->bxsize*l->bysize, 7);
}
extern "C" unsigned long long apibody(struct tc* ct){
  //asm("cli");
  if(ct->rax==0){
    mtaskd::current->tm->cns->putc(ct->rdi);
  }else if(ct->rax==1){
    mtaskd::current->tm->cns->puts((const char*)getpaddr(getcr3(), ct->rdi));
    mtaskd::current->tm->cns->l->refresh();
  }else if(ct->rax==2){
    unsigned long long size=(ct->rdi+0xfff)&~0xfff;
    unsigned long long addr=searchmem(size);
    int alp=mtaskd::current->alp;
    mtaskd::current->al[alp].addr=addr;
    mtaskd::current->al[alp].size=size;
    mtaskd::current->alp++;
    allocpage(getcr3(), addr, addr, size, 7);
    return addr;
  }else if(ct->rax==3){
    window* w=new window(ct->rdi, ct->rsi);
    initsheet(w->cs);
    return (unsigned long long)w;
  }else if(ct->rax==4){
    for(int i=0;i<mtaskd::current->alp;i++){
      freemem((unsigned long long)mtaskd::current->al[i].addr);
    }
    return mtaskd::current->brsp;
  }else if(ct->rax==5){
    struct fat_ent* f=(struct fat_ent*)drvd::drvs[bdl]->files->findfile((const char*)getpaddr(getcr3(), ct->rdi));
    if(f==0)return 0;
    file *fd=new file;
    fd->ptr=fd->base;
    fd->size=f->filesize;
    drvd::drvs[bdl]->files->preparecluschain(f->getclus());
    cns->puts("size=%d\n", fd->size);
    fd->base=(char*)drvd::drvs[bdl]->files->getclusaddr(f->getclus());;
    allocpage(getcr3(), (unsigned long long)fd->base, (unsigned long long)fd->base, f->filesize+0xfff, 7);
    allocpage(getcr3(), (unsigned long long)fd, (unsigned long long)fd, sizeof(file), 7);
    return (unsigned long long)fd;
  }else if(ct->rax==6){
    
  }else if(ct->rax==7){
    unsigned long long addr=ct->rdi;
    struct alloclist* al=mtaskd::current->al;
    int alp=mtaskd::current->alp;
    int i=-1;
    for(i=0;i<alp;i++){
      if(al[i].addr==addr){
        break;
      }
    }
    if(i==-1)return 0;
    for(;i<alp-1;i++){
      al[i]=al[i+1];
    }
    alp--;
    freemem((unsigned long long)addr);
  }
  return 0;
}
void api_init(){ 
  set_idt(0x40, (unsigned long long)asmapihandle, 8, 0xee);
}
