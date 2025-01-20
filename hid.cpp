#include "xhci.h"
namespace hidd{
unsigned char gete(unsigned char* a){
  return *a&0xfc;
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
unsigned long long getd(unsigned char* a, unsigned int bita, unsigned int bsize){
  unsigned long long v=0;
  unsigned char* p=&a[bita/8];
  //cns->puts("index: %02x\n", bita/8);
  unsigned int hasuu=bita%8;
  for(unsigned int i=0,j=0;i<(bsize+7+hasuu-1)/8;i++){
    //cns->puts("decode: %02x from %02x\n", (p[i]>>hasuu)<<j, p[i]);
    v|=(p[i]>>hasuu)<<j;
    j+=8-hasuu;
    hasuu=0;
  }
  //cns->puts("test: %0llx to %0llx\n", (1<<bsize)-1, v);
  v=v&((1<<bsize)-1);
  if((v>>(bsize-1))&1){
    unsigned long long a=~0;
    a&=~((1<<bsize)-1);
    v|=a;
  }
  return v;
}
unsigned long long getd(unsigned char* a){
  return getdfornt(&a[1], (a[0]&3)*8);
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
  //cns->puts("The first byte of the report descriptor: %02x\n", p[0]);
  /*for(unsigned int i=0;i<size;i++){
    for(unsigned int j=0;j<(p[i]&3)+1;j++){
      cns->puts("%02x ", p[i+j]);
    }
    cns->nline();
    i+=p[i]&3;
  }*/
  d->atkoff=0;
  unsigned char cp=0;
  unsigned int obo=0;
  unsigned int lmin,lmax;
  unsigned int uis[30];
  unsigned int uip=0;
  unsigned char rp=0;
  unsigned char rc=0;
  while(lp>p){
    unsigned char type=gete(p);
    //p++;
    switch(type){
      case 0x04:
          cp=getd(p);
          if(getd(p)==9){
            uis[uip]=0;
            uip++;
            //cns->puts("Button Usage\n");
          }else if(getd(p)==7){
            //uis[uip]=0x100;
            //uip++;
            uip=0;
          }
          //cns->puts("Usage Page: %02x\n", getd(p));
        break;
      case 0x84:
        //cns->puts("report setting to %d\n", getd(p));
        //if(rc)bo+=8;
        rc++;
          bo+=8;
          /*if(d->xsize)d->xoff+=8;
          if(d->ysize)d->yoff+=8;
          if(d->bsize)d->boff+=8;*/
        rp=getd(p);
        break;
      case 0xc0:
        cp=0;
        break;
      case 0x08:
        uis[uip]=getd(p);
        uip++;
        //cns->puts("Usage %x\n", getd(p));
        break;
      case 0x14:
        lmin=getd(p);
        break;
      case 0x24:
        lmax=getd(p);
        //cns->puts("setting lmax: %x\n", lmax);
        break;
      case 0x74:
        rs=getd(p);
        //cns->puts("setting rs=%d\n", rs);
        break;
      case 0x94:
        cc=getd(p);
        //cns->puts("setting c=%d\n", cc);
        break;
      case 0x90:
        if(cp==8){
          //cns->puts("led page:%dbit %dbits size\n", obo, rs*cc);
          uip=0;
          cp=7;
        }
        obo+=rs*cc;
        //cns->puts("OUTPUT REPORT\n");
        break;
      case 0x80:
        unsigned char attr=getd(p);
        if(uip==0){
          if((cp==7)&&(rs==8)&&(!(attr&1))){
            //cns->puts("Key array is %dbits on %d\n", rs*cc, bo);
            d->kasize=rs*cc;
            d->kaoff=bo;
          }else if((rs==1)&&(!d->atksize)){
            d->atkoff=bo;
            d->atksize=rs*cc;
            //cns->puts("off size %d %d\n", d->atkoff, d->atksize);
          }
          bo+=rs*cc;
          //cns->puts("plus rs*cc(%d)\n", rs*cc);
        }
        for(unsigned int i=0;i<uip;i++){
          unsigned int ui=uis[i];
          if((ui==0x02)||(ui==0x01)||(ui==238))continue;
          bool valid=false;
          //cns->puts("configuring Usage: %x\n", ui);
          if(ui==0x30){
            d->xoff=bo;
            d->xsize=rs;
            d->xmin=lmin;
            d->xmax=lmax;
            if(!(attr&4)){
              d->off=0;
              //cns->puts("Offing OFFSET\n");
            }
          }else if(ui==0x31){
            d->yoff=bo;
            d->ysize=rs;
            d->ymin=lmin;
            d->ymax=lmax;
          }else if(ui==0){
            d->boff=bo;
            d->bsize=rs*cc;
          //cns->puts("plus %d(cc=%d)\n", rs*(cc-1), cc);
            bo+=rs*(cc-1);
          }else if(cp==7){
            bo+=rs*(cc-1);
          }else if(ui==6){
            bo+=rs*(cc-1);
          //cns->puts("plus %d(cc=%d)\n", rs*(cc-1), cc);
          }
          bo+=rs;
          //cns->puts("plus %d\n", rs);
        }
        uip=0;
        //cns->puts("xmax=%d(off=%d) ymax=%d(off=%d)\n", d->xmax, d->ymax, d->xoff, d->yoff);
        break;
    }
    p+=p[0]&3;
    p++;
  }
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
  //cns->puts("HID\n");
  //cns->puts("sub=%d\n", id.binterfacesubclass);
  isr=id.binterfacesubclass^1;
  unsigned char* p=fulld;
  do {
    if(p[0]==0)break;   
    if(p[1]==4){
      //slots[slot].intn=p[2];
      //cns->puts("protocol=%d\n", p[7]);
    }
    if(p[1]==5){
      if(p[2]&0x80){
        unsigned char t=p[3]&3;
        t+=(p[2]>>7)*4;;
        //cns->puts("ep type=%d addr=%d\n", t, calcepaddr(p[2]));
        if(t==7){
          intin=calcepaddr(p[2]);
        }
      }
    }
    p+=p[0];
  }while(p[1]!=4);
  //cns->puts("intn=%d indci=%d isr=%d\n", slots[slot].intn, intin, isr);
  //led=searchmem(1);
  if(isr){
    off=1; //マウスじゃなかったときのため（off=0だと/xmaxでエラー泊）
    initphase=0;
    initialized=1;
    //cns->puts("getting report descirptor...\n");
      controltrans(slot, 0b10000001, 6, 0x2200, in, reportlength,   searchmem(reportlength), 1);
  }else{
    initphase=1234;
    initialized=1;
    off=1;
    isr=1;
    controltrans(slot, 0b00100001, 11, 1, in, 0, 0, 0);
  }
  firstt=true;
  initialized=1;
  test=(unsigned char*)searchmem(1);
}       
void hid::comp(struct transfertrb* t){
  using namespace xhci;
  a:
  if(isr&&t->code!=4){
    if(initphase==1234){
        struct TRB* ttt=(struct TRB*)*(unsigned long long*)t;
        //cns->puts("protocol at %0llx\n", ttt->type);
      //cns->puts("ep state: %d at %d\n", dcbaa[slot]->epcont[0].epstate, slots[slot].intn);
      initphase=0;
      for(int i=0;i<50000;i++);
      controltrans(slot, 0b10000001, 6, 0x2200, in, reportlength,   searchmem(reportlength), 1);
      //controltrans(slot, 0b10100001, 3, 0, slots[slot].intn, 1, searchmem(1), 1);
    }else if(initphase==1233){
      if(t->code!=6){
        struct dataTRB* ttt=(struct dataTRB*)*(unsigned long long*)t;
        //cns->puts("protocol at %0llx\n", *(unsigned char*)ttt->pointer);
        controltrans(slot, 0b00100001, 11, 1, in, 0, 0, 0);
      }else{
      initphase=1;
      //controltrans(slot, 0b00100001, 11, 1, slots[slot].intn, 0, 0, 0);
      tr[slot][intin]->push((struct TRB*)nt);
      db[slot]=intin;
      }
      //controltrans(slot, 0b10000001, 6, 0x2200, slots[slot].intn, reportlength,   searchmem(reportlength), 1);
      initphase=0;
    }else if(initphase==0){
      //cns->puts("length: %0d\n", reportlength-t->trbtransferlength);
      decoderd(this, (unsigned char*)*(unsigned long long*)*(unsigned long long*)t, reportlength-t->trbtransferlength);
      //if(slots[slot].type==USBRKeyboard&&kasize==0)return;
      initphase=1;
      //controltrans(slot, 0b00100001, 11, 1, slots[slot].intn, 0, 0, 0);
      sendingnt=true;
      tr[slot][intin]->push((struct TRB*)nt);
      db[slot]=intin;
    }else{
      //if(((struct TRB*)t->pointer)->type==1){
        //sendingnt=true;
     // }
      if(xmax&&ymax){
        int nmx=getd(buf, xoff, xsize);
        int nmy=getd(buf, yoff, ysize);
        if(!off){
          //if((!xmax)||(!ymax))asm("cli\nhlt");
          //asm("cli\nhlt");
          nmx=nmx*scrxsize/xmax;
          nmy=nmy*scrysize/ymax;
          int px=nmx-mx;
          int py=nmy-my;
          //asm("cli");
          kernelbuf->write(0);
          kernelbuf->write(getd(buf, boff, bsize));
          kernelbuf->write((signed int)px);
          kernelbuf->write((signed int)py);
          //asm("sti");
        }else{
          //cns->puts("strange data recieved code=%d length=%d(NT: %d)\n", t->code,   t->trbtransferlength, nt->trbtransferlength);
          kernelbuf->write(0);
          kernelbuf->write(getd(buf, boff, bsize));
          kernelbuf->write((signed long long)nmx);
          kernelbuf->write((signed long long)nmy);
        }
      }
      if(kasize){
        *(unsigned long long*)kernel=getd(buf, atkoff, atksize);
        unsigned long long nkey=getd(buf, kaoff, kasize);
        *(unsigned long long*)kernel|=nkey<<16;
          //cns->puts("data: %0llx %016llx\n", *(unsigned long long*)kernel, *(unsigned long long*)buf);
        if((!firstt)||(*(unsigned long long*)kernel)){
          kernelbuf->write(5);
          kernelbuf->write(kasize/8);
          for(unsigned int i=0;i<kasize/8;i++)
            kernelbuf->write(getd(buf, kaoff+i*8, 8));
          firstt=false;
        }
        //*(unsigned char*)led^=1;
        //controltrans(slot, 0b00100001, 9, 0x0202, in, 1, led, 0);
      }
      if(((struct TRB*)t->pointer)->type==1){
        tr[slot][intin]->push((struct TRB*)nt);
        db[slot]=intin;
      }
    }
  }else if(t->code==4){
    cns->puts("Transaction hid error! on slot=%d\n", slot);
  }
  if(!isr){
    if(id.binterfaceprotocol==2){
      //asm("cli");
      kernelbuf->write(0);
      kernelbuf->write((signed char)buf[0]);
      kernelbuf->write((signed char)buf[1]);
      kernelbuf->write((signed char)buf[2]);
      //asm("sti");
      tr[slot][intin]->push((struct TRB*)nt);
      db[slot]=intin;
    }else if(id.binterfaceprotocol==1){
      if(initphase==2){
        if(((struct TRB*)*(unsigned long long*)t)->type!=1){
          cns->puts("result: %d\n", t->code);
        }else{
          sendingnt=false;
          //asm("cli");
          kernelbuf->write(5);
          kernelbuf->write((unsigned long long)buf);
          //asm("sti");
        //cns->puts("setting LED epstate=%d\n", dcbaa[slot]->epcont[0].epstate);
          test[0]+=1;
        /*if(dcbaa[slot]->epcont[0].epstate!=1)cns->puts("EP Error detected!(state=%d)\n", dcbaa[slot]->epcont[0].epstate);
        controltrans(slot, 0b00100001, 9, 0x0200, slots[slot].intn, 1, (unsigned long long)test, 0);
        }*/
          if(!sendingnt){
            sendingnt=true;
            tr[slot][intin]->push((struct TRB*)nt);
            db[slot]=intin;
          }
        }
      }else if(initphase==0){
        initphase=2;
        tr[slot][intin]->push((struct TRB*)nt);
        db[slot]=intin;
      }else{
        initphase=2;
      }
    }
  }
}
