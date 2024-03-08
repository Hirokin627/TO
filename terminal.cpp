#include "kernel.h"
void setmustdir(unsigned char* dl, char** path, int* dn, const char* n,char ddl=bdl);
namespace terminald{
  void main(task* tsk){
    terminal* tm=new terminal;
    tm->m(tsk);
  }
};
void terminal::m(task* t){
  asm("cli");
  //cns->puts("terminal called\n");
  tsk=t;
  int line=60;
  int rows=15;
  window* w=new window(line*8, rows*16);
  w->owner=t;
  //cns->puts("terminal started\n");
  graphic::drawbox(w->cs, 0, 0, 0, w->cs->bxsize-1, w->cs->bysize-1);
  timer* tm=new timer;
  fifo* f=t->f;
  cns=new console(line, rows);
  cns->l->updown(layerd::top-1);
  cns->l->flags|=ITS_WINDOW;
  w->cs->registss(cns->l);
  cns->l->slide(1, 1);
  char cmdl[60];
  int lp=0;
  t->tm=this;
  cns->puts("test\n>");
  while(1){
    asm("cli");
    if(f->len==0){
      t->sleep();
    }else{
      int q=f->read();
      //cns->puts("q=%d\n", q);
      if(q==2){
        unsigned char k=f->read();
        if(!(k&0x80)){
          if(keytable0[k]=='\n'){
            cns->nline();
            cmdl[lp]=0;
            char* f_arg=(char*)cmdl;
            int ma=0;
            for(int i=0;i<lp;i++){
              if(cmdl[i]==' '){
                cmdl[i]=0;
                ma++;
                if(ma==1)f_arg=&cmdl[i+1];
              }
            }
            if(!strcmp((const char*)cmdl, "ls")){
              const char* dp=".";
              if(f_arg!=cmdl)dp=(const char*)f_arg;
              dirent* de=opendir(dp);
              if(de){
                dirent* d=de;
                while(d->reclen){
                  cns->puts("%s", d->name);
                  if(d->type==4){
                    cns->puts("    <DIR>");
                  }
                  cns->nline();
                  d++;
                }
                closedir(de);
              }
            }else if(!strcmp((const char*)cmdl, "cd")){
              if(f_arg==cmdl){
                cns->puts("Usage cd <directory name>\n");
              }else{
                int dn;
                unsigned char dl;
                char* path;
                setmustdir(&dl, &path, &dn,  (char*)f_arg);
                dn=drvd::drvs[bdl]->files->getdn((const char*)path, dn);
                if(dn>=0){
                  t->cd=dn;
                }
              }
            }else if(!strcmp((const char*)cmdl, "cat")){
              file* f=fopen((const char*)f_arg);
              if(f){
                for(int i=0;i<f->size;i++){
                  cns->putc(f->base[i]);
                }
                cns->nline();
                closef(f);
              }else{
                cns->puts("File not found\n");
              }
            }else if(cmdl[0]!=0){
              file* f=fopen((const char*)cmdl);
              if(f){
                unsigned long long* backup=getcr3();
                unsigned long long* ap4=(unsigned long long*)makep4();
                allocpage(ap4, 0xffff800000000000, (unsigned long long)f->base, f->size, 7);
                typedef void ent();
                setcr3(ap4);
                ent* entry=(ent*)*(unsigned long long*)((unsigned long long)f->base+24);
                cns->puts("entry point:%p first b=%02x\n", entry, *(unsigned char*)entry);
                asm("sti");
                entry();
                setcr3(backup);
                closef(f);
              }else{
                cns->puts("File not present\n");
              }
            }
            cns->putc('>');
            lp=0;
          }else if(keytable0[k]!='\b'){
            cns->putc(keytable0[k]);
            cmdl[lp]=keytable0[k];
            lp++;
          }else{
            if(lp>0){
              cns->putc(keytable0[k]);
              lp--;
              cmdl[lp]=0;
            }
          }
        }
        //cns->puts("key=%02x\n", k);
      }
    }
  }
}
