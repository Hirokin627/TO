#include "xhci.h"
namespace hidd{
unsigned char gete(unsigned char* a){
  return *a&0xfc;
}
unsigned long long getd(unsigned char* a){
  unsigned long long bytsize=a[0]&3;
  unsigned long long v=0;
  for(int i=0;i<bytsize;i++){
    unsigned char d=a[1+i];
    v|=d<<(i*8);
  }
  return v;
}
unsigned int getdfornt(unsigned char* p, unsigned char bsize){
  unsigned int v=0;
  for(int i=0;i<bsize;i++){
    v|=p[i]<<(i*8);
  }
  return v;
}
void plus(unsigned int* bo, unsigned int* bp, unsigned int v){
  *bp+=v;
  if(*bp>=8){
    *bo+=*bp/8;
    *bp%=8;
  }
}
void decoderd(hid* d, unsigned char* p, unsigned long long size){
  using namespace xhci;
  unsigned char slot=d->slot;
  unsigned char* lp=(unsigned char*)((unsigned long long)p+size);
  int cc=0;
  unsigned int bo=0,bp=0;
  unsigned int rs=0;
  while(lp>p){
    if(cc>0){
      if(gete(p)==0xc0){
        cc--;
      }else if(gete(p)==4){
        unsigned char pg=getd(p);
        p+=p[0]&3;
        p++;
        unsigned int lmin=0,lmax=0;
        fifo* u=new fifo(128);
        unsigned int cnt=0;
        while(gete(p)!=0x80){
          if(gete(p)==0x14){
            lmin=getd(p);
          }else if(gete(p)==0x24){
            lmax=getd(p);
          }else if(gete(p)==8){
            u->write(getd(p));
          }else if(gete(p)==0x74){
            rs=getd(p);
          }else if(gete(p)==0x94){
            cnt=getd(p);
          }
          p+=p[0]&3;
          p++;
        }
        int mu=u->len;
        if(pg==9&&!mu){
          d->boff=bo;
          d->bsize=rs;
          plus(&bo, &bp, rs*cnt);
        }
        while(mu){
          unsigned char ui=u->read();
          mu--;
          if(ui==0x30){
            d->xoff=bo;
            d->xsize=rs/8;
            d->xmax=lmax;
            d->xmin=lmin;
          }else if(ui==0x31){
            d->yoff=bo;
            d->ysize=rs/8;
            d->ymax=lmax;
            d->xmin=lmin;
          }
          plus(&bo, &bp, rs);
        }
      }else if(gete(p)==0x94||gete(p)==0x74){
        int cnt=0;
        int btsize=rs;
        while(gete(p)!=0x80){
          if(gete(p)==0x94){
            cnt=getd(p);
          }else if(gete(p)==0x74){
            btsize=getd(p);
          }
          p+=p[0]&3;
          p++;
        }
        plus(&bo, &bp, cnt*btsize);
      }
    }else{
      if(gete(p)==8){
        uint8_t t=getd(p);
        if(t==2){
          slots[slot].type=USBRMouse;
        }
      }else if(gete(p)==0xa0){
        //cns->puts("Collec start\n");
        cc++;
      }
    }
    p+=p[0]&3;
    p++;
  }
}
};
using namespace hidd;
void hid::init(unsigned char s){
  using namespace xhci;
  slot=s;
  initphase=0;
  isr=slots[slot].id.binterfacesubclass^1;
  buf=(unsigned char*)searchmem(8);
  nt=new normalTRB;
  nt->pointer=(unsigned long long)buf;
  nt->trbtransferlength=8;
  nt->ioc=1;
  cns->puts("HID\n");
  if(!isr)controltrans(slot, 0b00100001, 11, 0, slots[slot].intn, 0, 0, 0);
  else controltrans(slot, 0b10000001, 6, 0x2200, slots[slot].intn, 0x100, searchmem(0x100), 1);
}       
void hid::comp(struct transfertrb* t){
  using namespace xhci;
  if(!isr){
      if(slots[slot].type==USBMouse){
        asm("cli");
        kernelbuf->write(0);
        kernelbuf->write(buf[0]);
        kernelbuf->write((signed char)buf[1]);
        kernelbuf->write((signed char)buf[2]);
        asm("sti");
      }
      tr[slot][slots[slot].intin]->push((struct TRB*)nt);
      db[slot]=slots[slot].intin;
      initphase=1;
  }else{
    if(initphase==0){
      decoderd(this, (unsigned char*)*(unsigned long long*)*(unsigned long long*)t, 0x100-t->trbtransferlength);
      initphase=1;
      controltrans(slot, 0b00100001, 11, 1, slots[slot].intn, 0, 0, 0);
      cns->puts("HID port=%d\n", slots[slot].port);
    }else{
      int nmx=getdfornt(&buf[xoff], xsize)*scrxsize/xmax;
      int nmy=getdfornt(&buf[yoff], ysize)*scrysize/ymax;
      int px=nmx-mx;
      int py=nmy-my;
      asm("cli");
      kernelbuf->write(0);
      kernelbuf->write(getdfornt(&buf[boff], bsize));
      kernelbuf->write((signed int)px);
      kernelbuf->write((signed int)py);
      asm("sti");
      tr[slot][slots[slot].intin]->push((struct TRB*)nt);
      db[slot]=slots[slot].intin;
    }
  }
}
