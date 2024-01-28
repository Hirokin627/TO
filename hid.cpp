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
  bsize+=7;
  bsize/=8;
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
      if(gete(p)==0xc0){
        cns->puts("collec end\n");
        cc--;
      }else if(gete(p)==4){
        cns->puts("Usage Page(%x)\n", getd(p));
        unsigned char pg=getd(p);
        p+=p[0]&3;
        p++;
        unsigned int lmin=0,lmax=0;
        fifo* u=new fifo(128);
        unsigned int cnt=0;
        while(gete(p)!=0x80&&gete(p)!=0x90){
          if(gete(p)==0x14){
            cns->puts("log min(%x)\n", getd(p));
            lmin=getd(p);
          }else if(gete(p)==0x24){
            cns->puts("log max(%x)\n", getd(p));
            lmax=getd(p);
          }else if(gete(p)==8){
            cns->puts("Usage (%x)\n", getd(p));
            if(getd(p)==2){
              cns->puts("This is MOUSE(TRUE!!!!!!!!)\n");
              slots[slot].type=USBRMouse;
              break;
            }else if(getd(p)==1)break;
            else if(getd(p)==6){
              slots[slot].type=USBRKeyboard;
              break;
            }else u->write(getd(p));
          }else if(gete(p)==0x74){
            cns->puts("Report size(%x)\n", getd(p));
            rs=getd(p);
          }else if(gete(p)==0x94){
            cns->puts("Report count(%x)\n", getd(p));
            cnt=getd(p);
          }
          p+=p[0]&3;
          p++;
        }
        cns->puts("Input or Output\n");
        int mu=u->len;
        if(pg==9){
          d->boff=bo;
          d->bsize=rs;
          plus(&bo, &bp, rs*cnt);
        }else if(pg==1){
          while(mu){
            unsigned char ui=u->read();
            mu--;
            if(ui==0x30){
              d->xoff=bo;
              d->xsize=rs;
              d->xmax=lmax;
              d->xmin=lmin;
              cns->puts("x addr bit=%d byte=%d\n", bp, bo);
              cns->puts("Value attr:%02x\n", getd(p)); 
              d->off=getd(p)==6;
            }else if(ui==0x31){
              d->yoff=bo;
              d->ysize=rs;
              d->ymax=lmax;
              d->xmin=lmin;
              cns->puts("y addr bit=%d byte=%d\n", bp, bo);
            }
            plus(&bo, &bp, rs);
          }
        }else if(pg==12){
          plus(&bo, &bp, rs*cnt);
        }else if(pg==7){
          if(getd(p)==0){
            d->kaoff=bo;
            d->kasize=cnt;
          }
          cns->puts("rs=%d cnt=%d\n", rs, cnt);
          plus(&bo, &bp, rs*cnt);
        }else{
          plus(&bo, &bp, rs*cnt);
        }
        delete u;
      }else if(gete(p)==0x94||gete(p)==0x74){
        int cnt=0;
        int btsize=rs;
        cns->puts("Pedding\n");
        while(gete(p)!=0x80){
          if(gete(p)==0x94){
            cnt=getd(p);
          }else if(gete(p)==0x74){
            btsize=getd(p);
          }
          p+=p[0]&3;
          p++;
        }
        cns->puts("Size: cnt=%d btsize=%d\n", cnt, btsize);
        plus(&bo, &bp, cnt*btsize);
      }
      if(gete(p)==8){
        uint8_t t=getd(p);
        cns->puts("type(guess): %02x\n", t);
        if(t==2){
          slots[slot].type=USBRMouse;
        }else{
        }
      }else if(gete(p)==0xa0){
        cc++;
      }else{
      }
    p+=p[0]&3;
    p++;
  }
  bo+=(bp%8);
  d->nt->trbtransferlength=bo;
  if(bo>9){
    delete d->buf;
    d->buf=(unsigned char*)searchmem(bo);
  }
  if(slots[slot].type==USBRKeyboard){
    if(d->kasize==0){
      ports[slots[slot].port].haveerr=1;
      unsigned char port=slots[slot].port;
      ports[port].phase=waitreset;
      slots[slot].phase=waitreset;
      resetport(port);
    }
  }
  cns->puts("Total report length:%d (bo=%d bp=%d)\n", bo, bo-1, bp);
}
};
using namespace hidd;
#define reportlength 800
void hid::init(unsigned char s){
  using namespace xhci;
  slot=s;
  initphase=0;
  buf=(unsigned char*)searchmem(9);
  nt=new normalTRB;
  nt->pointer=(unsigned long long)buf;
  nt->trbtransferlength=8;
  nt->ioc=1;
  nt->isp=1;
  cns->puts("HID\n");
  cns->puts("sub=%d\n", id.binterfacesubclass);
  isr=id.binterfacesubclass^1;
  unsigned char* p=fulld;
  do {
    if(p[0]==0)break;   
    if(p[1]==4){
      slots[slot].intn=p[2];
      cns->puts("protocol=%d\n", p[7]);
    }
    if(p[1]==5){
      if(p[2]&0x80){
        unsigned char t=p[3]&3;
        t+=(p[2]>>7)*4;;
        cns->puts("ep type=%d addr=%d\n", t, calcepaddr(p[2]));
        if(t==7){
          intin=calcepaddr(p[2]);
        }
      }
    }
    p+=p[0];
  }while(p[1]!=4);
  cns->puts("intn=%d indci=%d isr=%d\n", slots[slot].intn, intin, isr);
  if(isr)controltrans(slot, 0b10000001, 6, 0x2200, slots[slot].intn, reportlength, searchmem(reportlength), 1);
  else
    controltrans(slot, 0b00100001, 11, 0, slots[slot].intn, 0, 0, 0);
  initialized=1;
}       
void hid::comp(struct transfertrb* t){
  using namespace xhci;
  if(isr&&t->code!=4){
    if(initphase==0){
    off=1; //マウスじゃなかったときのため（off=0だと/xmaxでエラー泊）
      decoderd(this, (unsigned char*)*(unsigned long long*)*(unsigned long long*)t, reportlength-t->trbtransferlength);
      if(slots[slot].type==USBRKeyboard&&kasize==0)return;
      initphase=1;
      controltrans(slot, 0b00100001, 11, 1, slots[slot].intn, 0, 0, 0);
    }else{
      if(!off){
        asm("cli");
        int nmx=getdfornt(&buf[xoff], xsize)*scrxsize/xmax;
        int nmy=getdfornt(&buf[yoff], ysize)*scrysize/ymax;
        int px=nmx-mx;
        int py=nmy-my;
        kernelbuf->write(0);
        kernelbuf->write(getdfornt(&buf[boff], bsize));
        kernelbuf->write((signed int)px);
        kernelbuf->write((signed int)py);
        asm("sti");
      }else{
        asm("cli");
        kernelbuf->write(0);
        kernelbuf->write(getdfornt(&buf[boff], 8));
        kernelbuf->write((signed char)getdfornt(&buf[xoff], xsize));
        kernelbuf->write((signed char)getdfornt(&buf[yoff], ysize));
        asm("sti");
      }
    }
    tr[slot][intin]->push((struct TRB*)nt);
    db[slot]=intin;
  }
  if(!isr){
    if(id.binterfaceprotocol==2){
      asm("cli");
      kernelbuf->write(0);
      kernelbuf->write((signed char)buf[0]);
      kernelbuf->write((signed char)buf[1]);
      kernelbuf->write((signed char)buf[2]);
      asm("sti");
      tr[slot][intin]->push((struct TRB*)nt);
      db[slot]=intin;
    }else if(id.binterfaceprotocol==1){
      asm("cli");
      kernelbuf->write(5);
      kernelbuf->write((unsigned long long)buf);
      kernelbuf->write(0);
      asm("sti");
      tr[slot][intin]->push((struct TRB*)nt);
      db[slot]=intin;
    }
  }
}
