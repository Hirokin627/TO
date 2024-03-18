#include "kernel.h"
void setmustdir(unsigned char* dl, char** path, int* dn, const char* n,char ddl=bdl);
namespace terminald{
  void main(task* tsk){
    terminal* tm=new terminal;
    tm->m(tsk);
  }
  void appcaller(unsigned long long rip, terminal* tm){
    
  }
};
char terminal::getc(){
  task* t=tsk;
  while(1){
    asm("cli");
    if(t->f->len==0){
      t->sleep();
    }else{
      unsigned int q=t->f->read();
      if(q==2){
        char k=t->f->read();
        if(!(k&0x80))return k;
      }
    }
  }
}
void terminal::m(task* t){
  asm("cli");
  //cns->puts("terminal called\n");
  tsk=t;
  t->tm=this;
  int line=60;
  int rows=15;
  w=new window(line*8+2, rows*16+2);
  w->owner=t;
  //cns->puts("terminal started\n");
  graphic::drawbox(w->cs, 0, 2, 2, w->cs->bxsize-3, w->cs->bysize-2);
  timer* tm=new timer;
  fifo* f=t->f;
  cns=new console(line, rows);
  cns->l->updown(layerd::top-1);
  cns->l->flags|=ITS_WINDOW;
  w->cs->registss(cns->l);
  cns->l->slide(1, 2);
  char cmdl[60];
  int lp=0;
  char runningapp=0;
  t->tm=this;
  cns->puts(">");
  cns->l->refresh();
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
            if(cmdl[lp-1]==0)lp--;
            if(!strcmp((const char*)cmdl, "ls")){
              drvd::drvs[bdl]->files->preparecluschain(0);
              struct fat_ent* de=(struct fat_ent*)drvd::drvs[bdl]->files->getclusaddr(0);
              for(int i=0;de[i].name[0]!=0;i++){
                if(de[i].attr==0x0f){
                  struct fat_lent* l=(struct fat_lent*)&de[i];
                  char* n=(char*)searchmem(((l->ord&0x1f)+12));
                  fatd::makename(l, n);
                  cns->puts("%s\n", n);
                  freemem((unsigned long long)n);
                  i+=de[i].name[0]&0x1f;
                  //i--;
                }else{
                  for(int j=0;j<11;j++){
                    cns->puts("%c", de[i].name[j]);
                  }
                  cns->nline();
                }
              }
            }else if(!strcmp((const char*)cmdl, "cd")){
              if(f_arg==cmdl){
                cns->puts("Usage cd <directory name>\n");
              }else{
                /*int dn;
                unsigned char dl;
                char* path;
                setmustdir(&dl, &path, &dn,  (char*)f_arg);
                dn=drvd::drvs[bdl]->files->getdn((const char*)path, dn);
                if(dn>=0){
                  t->cd=dn;
                }*/
              }
            }else if(!strcmp((const char*)cmdl, "lsd")){
              for(unsigned int i=0;i<0xff;i++){
                if(drvd::drvs[i]){
                  cns->puts("Drive %c: type:%d\n", i, drvd::drvs[i]->type);
                }
              }
            }/*else if(!strcmp((const char*)cmdl, "cat")){
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
            }*//*else if(!strcmp((const char*)cmdl, "cat")){
              struct fat_ent* f=drvd::drvs[bdl]->files->findfile((const char*)f_arg);
              if(f==0)cns->puts("file not found\n");
              drvd::drvs[bdl]->files->preparecluschain((f->clus_h<<16)|f->clus_l);
              int size=f->filesize;
              unsigned char* buf=drvd::drvs[bdl]->files->getclusaddr((f->clus_h<<16)|f->clus_l);
              do{
                cns->putc(*buf);
                buf++;
              }while(size--);
            }*/else if(!strcmp((const char*)cmdl, "exit")){
              asm("cli");
              delete w->cs; 
              kernelbuf->write(8);
              kernelbuf->write((unsigned long long)tm);
            }else if(!strcmp((const char*)cmdl, "clear")){
              graphic::drawbox(cns->l, cns->l->col_inv, 0, 0, cns->l->bxsize-1, cns->l->bysize-1);
              cns->cx=cns->cy=0;
            }else if(!strcmp((const char*)cmdl, "install")){
              cns->puts("select drive:\n");
              for(int i=0;i<256;i++){
                if(drvd::drvs[i]&&bdl!=i){
                  cns->puts("%c: ", i);
                }
              }
              cns->nline();
              char d=keytable0[getc()]-0x20;
              cns->puts("Selected: %c\n", d);
              //if(drvd::drvs[d]->files)delete drvd::drvs[d]->files;
              drive* nd=drvd::drvs[d];
              drive* bd=drvd::drvs[bdl];
              unsigned char* b=(unsigned char*)searchmem(512);
              for(int i=0;i<0x20;i++){
                bd->read(b, 1, i);
                nd->write(b, 1, i);
              }
              freemem((unsigned long long)b);
              struct BPB* bpb=(struct BPB*)searchmem(512);
              bd->read((unsigned char*)bpb, 1, 0);
              unsigned int* ft=(unsigned int*)searchmem(bpb->fat_size_32*512);
              ft[0]=0xffffff8;
              ft[1]=0xfffffff;
              ft[2]=0xffffff8;
              nd->write((unsigned char*)ft, bpb->fat_size_32, bpb->reserved_sector_count);
              freemem((unsigned long long)ft);
              unsigned char* buf=(unsigned char*)searchmem(1024);
              fat* files=(fat*)nd->files;
              //nd->write(buf, bpb->sectors_per_cluster, files->calcclus(bpb->root_cluster)); 
              fsd::recognizefs(d);
              freemem((unsigned long long)bpb);
              //while(1)drvd::drvs[bdl]->read((unsigned char*)searchmem(512), 1, 0);
              //createf((const char*)fn);
            }else if(cmdl[0]!=0){
              struct fat_ent* fe=drvd::drvs[bdl]->files->findfile((const char*)cmdl);;
              if(fe){
                drvd::drvs[bdl]->files->preparecluschain(fe->getclus());
                unsigned char* buf=drvd::drvs[bdl]->files->getclusaddr(fe->getclus());
                
                if(buf[0]==0x7f){
                  file* f=new file;
                  f->base=(char*)searchmem(fe->filesize);
                  for(int j=0;j<fe->filesize;j++)
                    f->base[j]=buf[j];
                  f->size=fe->filesize;
                  asm("cli");
                  
                  unsigned long long* backup=getcr3();
                  unsigned long long* ap4=(unsigned long long*)makep4();
                  unsigned long long as=searchmem(1024*1024);
                  allocpage(ap4, 0xffffff8000000000, as, 1024*1024, 7);
                  allocpage(ap4, 0xffff800000000000, (unsigned long long)f->base, f->size, 7);
                  allocpage(ap4, (unsigned long long)cmdl, (unsigned long long)cmdl, 60, 7); 
                  int argc=1;
                  char* argv[30];
                  if(cmdl!=f_arg){
                    for(int i=0;i<lp;i++){
                      if(cmdl[i]==0){
                        argv[argc]=&cmdl[i+1];
                        argc++;
                      }
                    }
                  }
                  argv[0]=cmdl;
                  asm("cli");
                  setcr3(ap4);
                  unsigned long long isp=searchmem(1024*1024);
                  mtaskd::current->alp=0;
                  *(unsigned long long*)4=isp+0xffff8;
                  jumpasapp(argc, argv, (unsigned long long)*(unsigned long long*)((unsigned long long)f->base+24), mtaskd::current);
                  setcr3(backup);
                  breakp4(ap4);
                  freemem((unsigned long long)isp);
                }else{
                  //cns->puts("this is not app (first byte:%02x)\n", f->base[0]);
                }
                //closef(f);
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
