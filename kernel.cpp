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
  while(1){
    asm("sti\nhlt");
    //switchcont(&taska, &taskb);
  }
}
unsigned char dp[44];
unsigned char bdl;
EFI_DEVICE_PATH_PROTOCOL* bdpp=(EFI_DEVICE_PATH_PROTOCOL*)dp;
extern "C" void nKernelmain(struct arg* ai){
  cli();
  asm("cli");
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
  layerd::init();
  cns=new console(60, (scrysize)/16);
  pci::init();
  pic_init();
  asm("sti");
  cns->puts("MSR=%x\n", readmsr(0xc0000080));
  kernelbuf=new fifo(128);
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
  task* ta=mtaskd::init();
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
  drvd::init(bdpp);
  //window* test=new window(200, 200);
  window* mw;
  int mpx,mpy;
  xhci::init();
  /*task* tb=new task((unsigned long long)testt);
  tb->run();*/
  unsigned char bk[256];
  while(1){
    asm("cli");
    if(kernelbuf->len==0){
      asm("sti\nhlt");
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
          if(!(l->flags&ITS_WINDOW))l=0;
          if(nowb->cs!=l&&!mw){
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
        asm("sti");
      }else if(q==2){
        unsigned char k=kernelbuf->read();
        asm("sti");
        if(k==1){
          window* nw=new window(200, 200);
        }else if(k==2){
          /*task* nt=new task((unsigned long long)testt);
          nt->run();*/
        }else if(k==0x1c){
          io_out8(0x64, 0xfe);
        }else if(k==3){
          acpi::shutdown();
        }else if(k==4){
          if(drvd::drvs[bdl]){
            asm("sti");
            fat* f=(fat*)drvd::drvs[bdl]->files;
            //f->init(drvd::drvs['A']);
            file* fl=f->getf("test.txt", f->rc);
            cns->puts("first b:%02x\n", fl->base[0]);
            closef(fl);
          }
        }else if(k==5){
          fat* f=(fat*)drvd::drvs[bdl]->files;
          dirent* d=f->getd(".", f->rc);
          dirent* de=d;
          while(de->reclen){
            cns->puts("name:%s\n", de->name);
            de++;
          }
          closedir(d);
        }else if(k==6&&drvd::drvs[bdl]){
          file* f=fopen("efi/boot/bootx64.efi");
          cns->puts("first b:%02x\n", f->base[0]);
          closef(f);
        }else if(k==7&&drvd::drvs['B']&&drvd::drvs[bdl]){
          struct BPB* bpb=(struct BPB*)searchmem(512);
          drvd::drvs[bdl]->read((unsigned char*)bpb, 1, 0);
          drvd::drvs['B']->write((unsigned char*)bpb, 1, 0);
          freemem((unsigned long long)bpb);
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
            cns->puts("pushed:%02x\n", usbcode[i]);
          }
          if(pk[i]==0&&bk[i]==1){
            kernelbuf->write(2);
            kernelbuf->write(usbcode[i]|0x80);
            cns->puts("unpushed:%02x\n", usbcode[i]);
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
      }
    }
  }
}
