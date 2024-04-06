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
unsigned char dp[44];
unsigned char bdl=0;
EFI_DEVICE_PATH_PROTOCOL* bdpp=(EFI_DEVICE_PATH_PROTOCOL*)dp;
task* ta;
void setvgaregister(unsigned short port, unsigned char index, unsigned char d){
  io_out8(port, index);
  if(port==0x3c0)io_out8(port+1, d);
  else io_out8(port+1, d);
}
unsigned char readvgaregister(unsigned short port, unsigned char index){  
  io_out8(port, index);
  return io_in8(port+1);
}
extern "C" void nKernelmain(struct arg* ai){
  cli();
  asm("cli");
  /*io_in8(0x3ca);
  setvgaregister(0x3d4, 0x11, readvgaregister(0x3d4, 0x11)&~0x7f);
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
  setvgaregister(0x3d4, 0x17, 0xa3);
  setvgaregister(0x3c4, 0x04, 0x04);*/
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
  layerd::init();
  cns=new console(60, (scrysize)/16);
  pci::init();
  pic_init();
  asm("cli");
  rtcd::init();
  drvd::init(bdpp);
  ided::init();
  satad::init();
  cns->puts("MSR=%x\n", readmsr(0xc0000080));
  ps2::init();
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
  fsd::init();
  //window* test=new window(200, 200);
  window* mw;
  int mpx,mpy;
  asm("cli");
  const char* sbl="shutdown";
  button* btn=new button(8*strlen(sbl)+6, 16+2, sbl);
  btn->l->oncrick=(event*)acpi::shutdown;
  btn->l->slide(3, scrysize-24+2);
  window* tw=new window(400, 400);
  textbox* tbx=new textbox(60);
  //tbx->l->updown(tw->cs->height);
  tw->cs->registss(tbx->l);
  tbx->l->updown(tw->cs->manye-1);
  //tbx->l->updown(tw->cs->height);
  //tw->cs->registss(tbx->l);
  tbx->l->slide(30, 30);
  xhci::init();
  unsigned char bk[256];
  unsigned char fo=0;
  while(1){
    asm("cli");
    if(kernelbuf->len==0){
      if(fo==0)
        asm("sti\nhlt");
      else{
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
            if(l&&l->oncrick)l->oncrick((unsigned long long)l);
          if(!(l->flags&ITS_WINDOW))l=0;
          layer* lcs=0;
          if(l){
            lcs=l->master? l->master : l;
          }
          if((nowb->cs!=lcs)&&!mw){
            nowb->setactive(false);
          }
          if(l&&!mw){
            if((l->flags&ITS_TB)){
              mw=l->master->wc;
              mpx=x;
              mpy=y;
            }
            if(l->master)l=l->master;
            nowb=l->wc;
            nowb->cs->updown(layerd::top-1);
            nowb->setactive(true);
          }
          if(!l&&!mw){
            if(nowb){
              nowb->setactive(false);
              nowb=0;
            }
          }
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
        xhci::posthandle();
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
            window* nw=new window(200, 200);
          }else if(k==2){
            task* nt=new task((unsigned long long)testt);
            nt->run();
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
          }else if(k==7){
            asm("cli");
            task* t=new task((unsigned long long)terminald::main);
            t->ct->rdi=(unsigned long long)t;
            t->run();
          }
        }
      }else if(q==5){
        unsigned long long p=kernelbuf->read();
        asm("sti");
        unsigned char* k=(unsigned char*)p;
        unsigned char pk[256];
        unsigned char nk[256];
        for(int i=0;i<256;i++){
          pk[i]=0;
          nk[i]=0;
        }
        for(int i=2;i<8;i++){
          pk[k[i]]=1;
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
