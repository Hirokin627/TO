#include "kernel.h"
int* vram;
int scrxsize,scrysize;
alignas(16) unsigned char stack[1024*1024];
console* cns;
fifo* kernelbuf;
extern "C" caddr_t sbrk(size_t size){
  return (caddr_t)searchmem(size);
}
int mx,my;
window* nowb;
textbox* nowt;
alignas(16) struct tc taska,taskb;

const char keytable0[0x80]={
0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0x08, 0, 
'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '@', '[', 0x0a, 0, 'a', 's', 
'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', ':', 0, 0, ']', 'z', 'x', 'c', 'v', 
'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4', '5', '6', '+', '1', 
'2', '3', '0', '.', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, '\\', 0, 0, 0, 0, 0, 0, 0, 0, 0, '\\', 0, 0, 
};
const char usbcode[256]={
0x00,0x00,0x00,0x00,0x1e,0x30,0x2e,0x20,0x12,0x21,0x22,0x23,0x17,0x24,0x25,0x26,
0x32,0x31,0x18,0x19,0x10,0x13,0x1f,0x14,0x16,0x2f,0x11,0x2d,0x15,0x2c,0x02,0x03,
0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x1c,0x1,0x0e,0x00,0x39,0x0c,0x00,0x1b,
0x2b,0x73,0x00,0x27,0x00,0x1a,0x33,0x34,0x35,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x35,0x37,0x0c,0x4e,0x1c,0x02,0x03,0x04,0x05,0x06,0x07,0x08,
0x09,0x0a,0x0b,0x34,0x73,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};
void testt(){
  asm("cli");
  new window(600, 600);
  mtaskd::current->sleep();
  while(1){
    asm("sti\nhlt");
    //switchcont(&taska, &taskb);
  }
}
int termlock=0;
char cuser[60];
unsigned char dp[44];
unsigned char bdl=0;
EFI_DEVICE_PATH_PROTOCOL* bdpp=(EFI_DEVICE_PATH_PROTOCOL*)dp;
task* ta;
void setvgaregister(unsigned short port, unsigned char index, unsigned char d){
  if(port==0x3c0){
    io_in8(0x3da);
    io_out8(0x3c0, index);
    io_out8(0x3c0, d);
    io_in8(0x3da);
    io_out8(0x3c0, 0x20);
  }else{
    io_out8(port, (d<<8)|index);
  }
}
unsigned char readvgaregister(unsigned short port, unsigned char index){  
  if(port==0x3c0){
    io_in8(0x3da);
    io_out8(port, index);
    unsigned char d=io_in8(0x3c1);
    io_in8(0x3da);
    io_out8(0x3c0, 0x20);
    return d;
  }else{
    io_out8(port, index);
    return io_in8(port+1);
  }
}
class logform;
void login(logform* form);
class logform{
  public:
    window* w;
    textbox* tbx;
    button* btn;
    logform(){
      w=new window(400, 400);
      tbx=new textbox(30);
      w->cs->registss(tbx->l);
      tbx->l->slide(30, 30);
      btn=new button(40+6, 18, "login");
      w->cs->registss(btn->l);
      btn->l->slide(300, 50);
      btn->l->oncrick=(event*)login;
      btn->l->owner=(unsigned long long)this;
    }
};
void logout(button* btn){
  delete btn;
  new logform;
}
void login(logform* form){
  struct fat_ent* fe=drvd::drvs[bdl]->files->findfile((const char*)nowt->chrs);
  if(fe){
    struct profile* pf=new struct profile;
    drvd::drvs[bdl]->files->loadfile(fe, (unsigned char*)pf);
    if(!strncmp((const char*)pf->sig, "USER", 4)){
      asm("cli");
      strcpy(cuser, (const char*)nowt->chrs);
      unsigned int bc=pf->bc;
      graphic::drawbox(layerd::bl, bc, 0, 0, scrxsize-1, scrysize-29);
      delete form->w;
      delete form->tbx;
      delete form->btn;
      delete form;
      termlock=0;
      //delete form->w->cs;
      //delete form->tbx->l;
      //delete form->btn->l;
    }
  }
}
void changescr(){
  
  io_out16(0x1ce, 4);
  io_out16(0x1cf, 0);
  io_out16(0x1ce, 3);
  io_out16(0x1cf, 8);
  io_out16(0x1ce, 1);
  io_out16(0x1cf, 320);
  io_out16(0x1ce, 2);
  io_out16(0x1cf, 200);
  setvgaregister(0x3c4, 0, 2);
  io_in8(0x3da);
  io_out16(0x3c4, 0x100);
  io_out8(0x3c2, 0xe3);
  io_out8(0x3c3, 0x01);
  unsigned int port=io_in8(0x3cc)&1 ? 0x3d4 : 0x3b4;
  setvgaregister(0x3c0, 0x10, 0x41);
  setvgaregister(0x3c0, 0x11, 0x00);
  setvgaregister(0x3c0, 0x12, 0x0f);
  setvgaregister(0x3c0, 0x13, 0x00);
  setvgaregister(0x3c0, 0x14, 0x00);
  io_out8(0x3c2, 0x63);
  setvgaregister(0x3c4, 0x01, 0x01);
  setvgaregister(0x3c4, 0x03, 0x00);
  setvgaregister(0x3c4, 0x04, 0x0e);
  setvgaregister(0x3ce, 0x05, 0x40);
  setvgaregister(0x3ce, 0x06, 0x05);
  setvgaregister(port, 0x00, 0x5f);
  setvgaregister(port, 0x01, 0x4f);
  setvgaregister(0x3d4, 0x02, 0x50);
  setvgaregister(0x3d4, 0x03, 0x82);
  setvgaregister(0x3d4, 0x04, 0x54);
  setvgaregister(0x3d4, 0x05, 0x80);
  setvgaregister(0x3d4, 0x06, 0xbf);
  setvgaregister(0x3d4, 0x07, 0x1f);
  setvgaregister(0x3d4, 0x08, 0x00);
  setvgaregister(0x3d4, 0x09, 0x41);
  setvgaregister(0x3d4, 0x10, 0x9c);
  setvgaregister(0x3d4, 0x11, 0x8e);
  setvgaregister(0x3d4, 0x12, 0x8f);
  setvgaregister(0x3d4, 0x13, 0x28);
  setvgaregister(0x3d4, 0x14, 0x40);
  setvgaregister(0x3d4, 0x15, 0x96);
  setvgaregister(0x3d4, 0x16, 0xb9);
  setvgaregister(0x3d4, 0x17, 0xa3);
  
  //vram=(int*)0xa0000;
}
extern "C" void nKernelmain(struct arg* ai){
  cli();
  asm("cli");
  /*setvgaregister(0x3d4, 0x11, readvgaregister(0x3d4, 0x11)&~0x7f);
  setvgaregister(0x3c4, 0x04, 0x00);
  setvgaregister(0x3c0, 0x10, 0x0c);
  setvgaregister(0x3c0, 0x11, 0x00);
  setvgaregister(0x3c0, 0x12, 0x0f);
  setvgaregister(0x3c0, 0x13, 0x08);
  setvgaregister(0x3c0, 0x14, 0x00);
  io_out8(0x3c2, 0x67);
  setvgaregister(0x3c4, 0x01, 0x00);
  setvgaregister(0x3c4, 0x03, 0x00);
  setvgaregister(0x3c4, 0x04, 0x07);
  setvgaregister(0x3ce, 0x05, 0x10);
  setvgaregister(0x3ce, 0x06, 0x0e);
  setvgaregister(0x3d4, 0x00, 0x5f);
  setvgaregister(0x3d4, 0x01, 0x4f);
  setvgaregister(0x3d4, 0x02, 0x50);
  setvgaregister(0x3d4, 0x03, 0x82);
  setvgaregister(0x3d4, 0x04, 0x55);
  setvgaregister(0x3d4, 0x05, 0x81);
  setvgaregister(0x3d4, 0x06, 0xbf);
  setvgaregister(0x3d4, 0x07, 0x1f);
  setvgaregister(0x3d4, 0x08, 0x00);
  setvgaregister(0x3d4, 0x09, 0x4f);
  setvgaregister(0x3d4, 0x10, 0x9c);
  setvgaregister(0x3d4, 0x11, 0x8e);
  setvgaregister(0x3d4, 0x12, 0x8f);
  setvgaregister(0x3d4, 0x13, 0x28);
  setvgaregister(0x3d4, 0x14, 0x1f);
  setvgaregister(0x3d4, 0x15, 0x96);
  setvgaregister(0x3d4, 0x16, 0xb9);
  setvgaregister(0x3d4, 0x17, 0xa3);*/
  
  
  /*io_out16(0x3c4, 0x100);
  io_out8(0x3c2, 0xe3);
  io_out8(0x3c3, 1);
  unsigned char sd[]={ 0x01, 0x0f, 0x00, 0x06 };
  for(int i=0;i<4;i++){
    setvgaregister(0x3c4, i+1, sd[i]);
  }
  setvgaregister(0x3c4, 0, 3);
  setvgaregister(0x3d4, 0x11, 0x20);
  unsigned char cd[]={ 0x5f, 0x4f, 0x50, 0x82, 0x54, 0x80, 0x0b, 0x3e, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xea, 0x8c, 0xdf, 0x28, 0x00, 0xe7, 0x04, 0xe3, 0xff};
  for(int i=0;i<=0x18;i++){
    setvgaregister(0x3d4, i, cd[i]);
  }
  unsigned char vd[]={ 0x00, 0x0f, 0x00, 0x00, 0x00, 0x03, 0x05, 0x00, 0xff};
  for(int i=0;i<=8;i++){
    setvgaregister(0x3ce, i, vd[i]);
  }
  unsigned char ad[]={0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x01, 0x00, 0x0f, 0x00, 0x00};
  for(int i=0;i<=0x14;i++)
    setvgaregister(0x3c0, i, ad[i]);
  io_out8(0x3c6, 0xff);*/
  //io_out16(0x1ce, 4);
  //io_out16(0x1cf, 1);
  vram=ai->Frame.fb;
  scrxsize=ai->Frame.xsize;
  scrysize=ai->Frame.ysize;
  for(int i=0;i<*(unsigned short*)ai->bd.Length;i++){
    dp[i]=*(unsigned char*)((unsigned long long)&ai->bd+i);
  }
  acpi::init((struct RSDP*)ai->acpi);
  memory_init(ai->mems, ai->size ,ai->bsize);
  unsigned int ia32=readmsr(0xc0000080);
  ia32=0x500;
  writemsr(0xc0000080, ia32);
  x64_init();
  api_init();
  kernelbuf=new fifo(128);
  pci::init();
  //svgad::init();
  layerd::init();
  cns=new console(60, (scrysize)/16);
  //cns->l->updown(0);
  pic_init();
  //pcnetd::init();
  asm("cli");
  rtcd::init();
  drvd::init(bdpp);
  ided::init();
  satad::init();
  ps2::init();
  fsd::init();
  taskb.cr3=(unsigned long long)getcr3();
  taskb.rip=(unsigned long long)testt;
  taskb.cs=8;
  taskb.ss=0x10;
  taskb.rflags=0x202;
  *(unsigned int*)&taskb.fx_area[24]=0x1f80;
  taskb.rsp=searchmem(1024)+1024-8;
  //cns->l->updown(-1);
  timerd::init();
  ta=mtaskd::init();
  cns->puts("Port=%x\n", io_in8(0x3cc)&1 ? 0x3b4 : 0x3d4);
  console* dc=new console(80/8+1, 3);
  layer* yd=dc->l;
  dc->puts("0000/00/00\n00::00");
  dc->l->slide(scrxsize-yd->bxsize, scrysize-32);
  //dc->l->updown(layerd::top+1);
  layer* l=new layer(16, 16);
  l->col_inv=-1;
  static char cursor[16][17]={
		"**************..",
		"*OOOOOOOOOOO*...",
		"*OOOOOOOOOO*....",
		"*OOOOOOOOO*.....",
		"*OOOOOOOO*......",
		"*OOOOOOO*.......",
		"*OOOOOOO*.......",
		"*OOOOOOOO*......",
		"*OOOO**OOO*.....",
		"*OOO*..*OOO*....",
		"*OO*....*OOO*...",
		"*O*......*OOO*..",
		"**........*OOO*.",
		"*..........*OOO*",
		"............*OO*",
		".............***"
  };
  for(int y=0;y<16;y++){
    for(int x=0;x<16;x++){
      unsigned int c=0;
      switch(cursor[y][x]){
        case '*':
          c=0;
          break;
        case 'O':
          c=0xffffff;
          break;
        default:
          c=-1;
          break;
      }
      l->buf[y*16+x]=c;
    }
  }
  //graphic::drawbox(l, 0xffffff, 0, 0, 15, 15);
  l->updown(layerd::top+1);
  xhci::init();
  //window* test=new window(200, 200);
  window* mw;
  int mpx,mpy;
  asm("cli");
  const char* sbl="shutdown";
  termlock=1;
  button* btn=new button(8*strlen(sbl)+6, 16+2, sbl);
  btn->l->oncrick=(event*)acpi::shutdown;
  btn->l->slide(3, scrysize-24+2);
  logform* lf=new logform;
  if(drvd::drvs['A']==0){
    asm("cli");
    window* wabc=new window(200, 200);
    graphic::putfontstr(wabc->cs, 0, 16, 0x00000, "Where am I?\n(could not\nrecognize\nboot disk");
  }
  unsigned char bk[256];
  for(int i=0;i<256;i++)bk[i]=0;
  unsigned char fo=0;
  unsigned char led=0;
  while(1){
    asm("cli");
    if(kernelbuf->len==0){
      if(fo==0){
        asm("sti\nhlt");
      }else{
        fo=0;
        mtaskd::taskswitch();
      }
      //switchcont(&taskb, &taska);
      //asm("sti");
    }else{
      unsigned int q=kernelbuf->read();
      if(q==0){
        unsigned char c=kernelbuf->read();
        signed int x=kernelbuf->read();
        signed int y=kernelbuf->read();
        if(c&1){
          if(mw){
            mpx+=x;
            mpy+=y;
          }
          layer* l=layerd::checkcrick(mx, my);
          layer* lcs=0;
          if(l){
            lcs=l;
            while(lcs->master){ 
              lcs=lcs->master;
            }
          }
          if(l&&(l->flags&ITS_TEXTBOX)){
            if(l->flags&ITS_CONSOLE)l=l->master;
            textbox* t=(textbox*)l->wc;
            nowt=t;
          }
          //if(!(l->flags&ITS_WINDOW))l=0;
          if(nowb&&(nowb->cs!=lcs)&&!mw){
            nowb->setactive(false);
          }
          if(l&&!mw){
            if((l->flags&ITS_TB)){
              mw=l->master->wc;
              mpx=x;
              mpy=y;
            }
            //(l->master)l=lcs;
            nowb=l->master ? lcs->wc : l->wc;
            //cns->puts("nowb->cs->height=%d(lcs=%p)\n", nowb->cs->height, lcs);
            if(nowb){
              nowb->cs->updown(layerd::top-1);
              nowb->setactive(true);
            }
          }
          if(!l&&!mw){
            if(nowb){
              nowb->setactive(false);
              nowb=0;
              nowt=0;
            }
          }
            if(l&&l->oncrick)l->oncrick(l->owner ? l->owner : (unsigned long long)l);
        }else{
          if(mw){
            mw->cs->slide(mw->cs->x+mpx, mw->cs->y+mpy);
            mw=0;
          }
        }
        mx=l->x+x;
        my=l->y+y;
        if(mx<0)mx=0;
        if(my<0)my=0;
        if(mx>scrxsize-1)mx=scrxsize-1;
        if(my>scrysize-1)my=scrysize-1;
        l->slide(mx, my);
      }else if(q==1){
        asm("sti");
        xhci::posthandle();
        pcnetd::polling();
        //asm("sti");
      }else if(q==2){
        unsigned char k=kernelbuf->read();
        if(nowb){
          if(nowb->owner){
            if(!(nowb->cs->flags&ITS_MADE_FOR_APP)){
              if(k!=0x4f){
                nowb->owner->f->write(2);
                nowb->owner->f->write(k);
              }else{
                delete nowb->owner->tm->w->cs;
                kernelbuf->write(8);
                kernelbuf->write((unsigned long long)nowb->owner->tm);
              }
            }
          }
        }else{
          if(k==1){
            pcnetd::polling();
          }else if(k==2){
            /*task* nt=new task((unsigned long long)testt);
            nt->run();*/
          }else if(k==0x1c){
            //io_out8(0x64, 0xfe);
            typedef enum{
              EfiResetCold,
              EfiResetWarm,
              EfiResetShutdown,
              EfiResetPlatformSpecific
            } EFI_RESET_TYPE;
            unsigned long long* rtb=(unsigned long long*)ai->rtb;
            typedef unsigned long long rs(unsigned long long, unsigned long long,unsigned long long,unsigned long long);
            rs* reset=(rs*)*(unsigned long long*)((unsigned long long)rtb+24+80);
            reset(EfiResetCold, 0, 0, 0);
          }else if(k==3){
            acpi::shutdown();
          }else if(k==4){
            if(drvd::drvs[bdl]){
              asm("sti");
              fat* f=(fat*)drvd::drvs[bdl]->files;
              //f->init(drvd::drvs['A']);
            }
          }else if(k==5){
            /*fat* f=(fat*)drvd::drvs[bdl]->files;
            dirent* d=f->getd(".", f->rc);
            dirent* de=d;
            while(de->reclen){
              cns->puts("name:%s\n", de->name);
              de++;
            }
            closedir(d);*/
          }else if(k==6&&drvd::drvs[bdl]){
          }else if(k==7&&!termlock){
            asm("cli");
            task* t=new task((unsigned long long)terminald::main);
            t->ct->rdi=(unsigned long long)t;
            t->run();
          }
        }
         if(nowt&&!(k&0x80)){
          if(keytable0[k]=='\n'){
            nowt=0;
          }
          if(nowt){
            nowt->c->putc(keytable0[k]);
            char chr=keytable0[k];
            switch(chr){
              case '\b':
                if(nowt->chrp>0){
                  nowt->chrp--;
                  nowt->chrs[nowt->chrp]=0;
                }
                break;
              default:
                nowt->chrs[nowt->chrp]=chr;
                nowt->chrp++;
                break;
            }
          }
        }
      }else if(q==5){
        unsigned long long bsize=kernelbuf->read();
        //unsigned long long p=kernelbuf->read();
        asm("sti");
        //unsigned char* k=(unsigned char*)p;
        unsigned char pk[256];
        unsigned char nk[256];
        for(int i=0;i<256;i++){
          pk[i]=0;
          nk[i]=0;
        }
        for(int i=2;i<2+bsize;i++){
          pk[kernelbuf->read()]=1;
          //cns->puts("detect: %02x\n", kernelbuf->read());
        }
        for(int i=0;i<256;i++){
          nk[i]=(pk[i]==1)&&(bk[i]==0);
          if(nk[i]){
            kernelbuf->write(2);
            kernelbuf->write(usbcode[i]);
          }
          if(pk[i]==0&&bk[i]==1){
            kernelbuf->write(2);
            kernelbuf->write(usbcode[i]|0x80);
          }
        }
        for(int i=0;i<256;i++){
          bk[i]=pk[i];
        }
      }else if(q==6){
        unsigned int type=kernelbuf->read();
        unsigned int mainaddr=kernelbuf->read();
        unsigned int subaddr=kernelbuf->read();
        drive* drv=(drive*)kernelbuf->read();
        drvd::registdrv(type, mainaddr, subaddr, drv);
        asm("sti");
      }else if(q==7){
        fo=1;
        int x0=kernelbuf->read();
        int y0=kernelbuf->read();
        int x1=kernelbuf->read();
        int y1=kernelbuf->read();
        asm("sti");
        layerd::trefreshsub(x0, y0, x1, y1);
      }else if(q==8){
        terminal* tm=(terminal*)kernelbuf->read();
        tm->tsk->sleep();
        delete tm->cns;
        delete tm->tsk;
        delete tm->w;
        delete tm;
      }else if(q==9){
        using namespace rtcd;
        //cns->puts("Year: %d month %d date %d hour %d minute %d second %d\n", y, m, d, h, mt, s);
        dc->puts("%04d/%02d/%02d\n %02d:%02d.%02d\n", y, m, d, h, mt, s);
      }
    }
  }
}
