#include "xhci.h"
namespace massd{
};
using namespace xhci;
using namespace massd;
void mass::init(unsigned char s){
  slot=s;
  initialized=1;
  initphase=0;
  unsigned char* p=fulld;
  p+=p[0];
  do {
    if(p[0]==0)break;
    if(p[1]==5){
      unsigned char ep=calcepaddr(p[2]);
      unsigned char ept=p[3]&3;
      ept+=(p[2]>>7)*4;
      if(ept==6)bulkin=ep;
      else if(ept==2)bulkout=ep;
    }
    p+=p[0];
  }while(p[1]!=4);
  outtrb=new struct normalTRB;
  outtrb->trbtransferlength=31;
  outtrb->ioc=outtrb->isp=1;
  intrb=new struct normalTRB;
  intrb->trbtransferlength=13;
  intrb->ioc=intrb->isp=1;
    struct CBW* cbw=new struct CBW;
    mycbw=cbw;
  //controltrans(slot, 0b10000000, 8, 0, 0, 1, searchmem(1), 1);
  controltrans(slot, 0b00100001, 0xff, 0, id.binterfacenumber, 0, 0, 0);
}
void mass::comp(struct transfertrb* t){
  if(t->code!=1&&t->code!=13){
    cns->puts("Trans error detect %d\n", t->code);
    freemem((unsigned long long)maxlun);
    initphase=0;
    controltrans(slot, 0b00100001, 0xff, 0, id.binterfacenumber, 0, 0, 0);
  }
  if(initphase==0){
  maxlun=(unsigned char*)searchmem(1);
    controltrans(slot, 0b10100001, 0xfe, 0, id.binterfacenumber, 1, (unsigned long long)maxlun, 1);
    initphase=1;
  }else if(initphase==1){
    cns->puts("maxlun=%d\n",*maxlun);
    struct CBW* cbw=mycbw;
    cbw->tag=1;
    cbw->transferlength=8;
    cbw->lun=0;
    cbw->flags=0x80;
    cbw->cblength=12;
    cbw->cb[0]=0x25;
    outtrb->pointer=(unsigned long long)cbw;
    tr[slot][bulkout]->push((struct TRB*)outtrb);
    db[slot]=bulkout;
    initphase=2;
  }else if(initphase==2){
    intrb->pointer=searchmem(8);
    intrb->trbtransferlength=8;
    tr[slot][bulkin]->push((struct TRB*)intrb);
    db[slot]=bulkin;
    initphase=3;
  }else if(initphase==3){
    for(int i=0;i<8;i++)cns->puts("%02x ", *(unsigned char*)(intrb->pointer+i));
    cns->nline();
    for(int i=0;i<4;i++){
      bpb|=*(unsigned char*)(intrb->pointer+i+4)<<((3-i)*8);
    }
    cns->puts("bpb=%x\n", bpb);
    intrb->pointer=searchmem(sizeof(struct CSW));
    cns->puts("CSW=%p\n", intrb->pointer);
    intrb->trbtransferlength=13;
    tr[slot][bulkin]->push((struct TRB*)intrb);
    db[slot]=bulkin;
    initphase=4;
  }else if(initphase==4){
    struct CSW* csw=(struct CSW*)intrb->pointer;
    if(csw->sig!=0x53425355){
    tr[slot][bulkin]->push((struct TRB*)intrb);
    db[slot]=bulkin;
    }else freemem((unsigned long long)csw);
    drvd::registdrv(5, slots[slot].port, id.binterfacenumber, new usbdrv(slot, id.binterfacenumber));
  }else if(initphase==5){
    struct CBW* cbw=(struct CBW*)outtrb->pointer;
    struct normalTRB* nt=cbw->flags>>7? intrb : outtrb;
    nt->trbtransferlength=bpb;
    nt->pointer=(unsigned long long)tb;
    unsigned int ep=cbw->flags>>7 ? bulkin : bulkout;
    tr[slot][ep]->push((struct TRB*)nt);
    db[slot]=cbw->flags>>7 ? bulkin : bulkout;
    initphase=6;
  }else if(initphase==6){
    struct CSW* csw=(struct CSW*)searchmem(sizeof(struct CSW));
    intrb->pointer=(unsigned long long)csw;
    intrb->trbtransferlength=13;
    tr[slot][bulkin]->push((struct TRB*)intrb);
    db[slot]=bulkin;
    initphase=7;
  }else if(initphase==7){
    struct CSW* csw=(struct CSW*)intrb->pointer;
    if(csw->sig!=0x53425355){
      tr[slot][bulkin]->push((struct TRB*)intrb);
      db[slot]=bulkin;
      cns->puts("Error\n");
    }else{
      if(csw->status!=0){
        cns->puts("Error code=%d cmd=%02x\n", csw->status, mycbw->cb[0]);
      }
      freemem((unsigned long long)csw);
      outtrb->pointer=(unsigned long long)mycbw;
      initphase=8;
    }
  }
}
void mass::read(unsigned char* buf, unsigned int cnt, unsigned int lba){
  initphase=5;
  tb=buf;
  struct CBW* cbw=(struct CBW*)outtrb->pointer;
  cbw->cb[0]=0xa8;
  for(int i=0;i<=0x18;i+=8){
    cbw->cb[2+i/8]=lba>>(0x18-i);
  }
  for(int i=0;i<=0x18;i+=8){
    cbw->cb[6+i/8]=cnt>>(0x18-i);
  }
  cbw->transferlength=bpb;
  cbw->cblength=12;
  cbw->flags=0x80;
  outtrb->trbtransferlength=31;
  tr[slot][bulkout]->push((struct TRB*)outtrb);
  db[slot]=bulkout;
  unsigned int r=rflags();
  while(initphase!=8){
    posthandle();
    asm("sti\nhlt");
  }
  srflags(r);
}
void mass::write(unsigned char* buf, unsigned int cnt, unsigned int lba){
  initphase=5;
  tb=buf;
  struct CBW* cbw=mycbw;
  cbw->cb[0]=0xaa;
  for(int i=0;i<=0x18;i+=8){
    cbw->cb[2+i/8]=lba>>(0x18-i);
  }
  for(int i=0;i<=0x18;i+=8){
    cbw->cb[6+i/8]=cnt>>(0x18-i);
  }
  cbw->transferlength=bpb;
  cbw->cblength=12;
  cbw->flags=0;
  outtrb->trbtransferlength=31;
  tr[slot][bulkout]->push((struct TRB*)outtrb);
  db[slot]=bulkout;
  unsigned int r=rflags();
  while(initphase!=8){
    posthandle();
    asm("sti\nhlt");
  }
  srflags(r);
}
usbdrv::usbdrv(unsigned char slot, unsigned char interface){
  intf=(mass*)drivers[slot][interface];
}
void usbdrv::read(unsigned char* buf, unsigned int cnt, unsigned int lba){
  unsigned char tb[2048];
  for(int i=0;i<cnt;i++){
    unsigned int tlba=(lba+i)/(intf->bpb/0x200);
    unsigned int blba=(lba+i)%(intf->bpb/0x200);
    intf->read(tb, 1, tlba);
    for(int j=0;j<512;j++){
      buf[i*512+j]=tb[blba*0x200+j];
    }
  }
}
void usbdrv::write(unsigned char* buf, unsigned int cnt, unsigned int lba){
  unsigned char tb[2048];
  for(int i=0;i<cnt;i++){
    unsigned int tlba=(lba+i)/(intf->bpb/0x200);
    intf->read(tb, 1, tlba);
    unsigned int blba=(lba+i)%(intf->bpb/0x200);
    for(int j=0;j<512;j++){
      tb[blba*512+j]=buf[i*512+j];
    }
    intf->write(tb, 1, tlba);
  }
}
