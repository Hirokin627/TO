#include "xhci.h"
template <class T>
void clearz(T* t){
  //for(int i=0;i<16;i++)*(unsigned char*)((unsigned long long)t+i)=0;
}
void CR::push(struct TRB* t){
  t->c=this->c;
  ring[wp]=*t;
  wp++;
  if(wp==29){
    struct linkTRB lt{};
    lt.pointer=(unsigned long long)ring;
    lt.tc=1;
    lt.c=this->c;
    ring[29]=*(struct TRB*)&lt;
    this->c^=1;
    wp=0;
  }
}
namespace xhci{
  struct pcid xhc;
  bool xhcp=false;
  struct capreg* creg;
  struct opr* ope;
  struct slot* slots;
  struct port* ports;
  bool msip;
  struct devc** dcbaa;
  unsigned char maxports;
  CR* cr;
  CR* tr[8][32];
  unsigned int eri=0;
  unsigned int *db;
  char c=1;
  struct ERST* erst;
  struct RR* rr;
  char addrport=0;
  classd* drivers[256];
  void clearbitportsc(unsigned char port, unsigned char b){
    unsigned int sc=ope->portset[port].portsc;
    sc&=~(1<<b);
    ope->portset[port].portsc=sc;
  }
  void resetport(unsigned char port){
    if(addrport>0){
      ports[port].phase=waitfree;
    }else if((ope->portset[port].portsc&1)){
      unsigned int sc=ope->portset[port].portsc;
      sc&= 0x0e00c3e0u;
      sc|=1<<4;
      sc|=1<<17;
      sc|=0x200000;
      sc|=0xf<<5;
      ope->portset[port].portsc=sc;
      while(ope->portset[port].portsc&0x10);
      ports[port].phase=resetting;
      cns->puts("resetting\n");
      addrport=port;
    }
  }
  void enableslot(){
    enableslotTRB* et=new enableslotTRB;
    cr->push((struct TRB*)et);
    db[0]=0;
    ports[addrport].phase=enablingslot;
    delete et;
  }
  void recievetrb(struct psctrb* trb){
  int port=trb->port;
  cns->puts("port=%d\n", port);
  if(ope->portset[port].portsc&1){
    if(ports[port].phase==waitreset){
      if(!(ope->portset[port].portsc&0x200000))resetport(port);
    }else if(ports[port].phase==resetting){
      unsigned int p=ope->portset[port].portsc;
      p&=0xe00c3e0;
      p|=1<<21;
      ope->portset[port].portsc=p;
      enableslot();
    }
  }else{
    ports[port].phase=waitreset;
    struct confeptrb cet{};
    cet.dc=1;
    cet.slot=ports[port].slot;
    cr->push((struct TRB*)&cet);
    db[0]=0;
  }
  }
  unsigned int maxpacket(int psiv){
    switch(psiv){
      case 3: return 64;
      case 4: return 512;
      default: return 8;
    }
  }
  void controltrans(unsigned char slot,unsigned char bmrequesttype, unsigned char brequest, unsigned short wvalue, unsigned short windex, unsigned short wlength, unsigned long long pointer, unsigned char dir){
  //////cns->puts("slot=%d\n", slot);
  struct setupTRB st{};
  st.bmrequesttype=bmrequesttype;
  st.brequest=brequest;
  st.wvalue=wvalue;
  st.windex=windex;
  st.wlength=wlength;
  st.trbtransferlength=8;
  if(wlength){
    if(dir)st.trt=3;
    else st.trt=2;
  }else{
    st.trt=0;
  }
  tr[slot][0]->push((struct TRB*)&st);
  if(wlength>0){
    struct dataTRB dt{};
    dt.pointer=pointer;
    dt.trbtransferlength=wlength;
    dt.tdsize=0;
    dt.dir=dir;
    dt.target=0;
    dt.ioc=1;
    tr[slot][0]->push((struct TRB*)&dt);
  }
  struct statusTRB stt{};
  stt.dir=dir^1;
  stt.target=0;
  if(wlength<=0)stt.ioc=1;
  tr[slot][0]->push((struct TRB*)&stt);
  db[slot]=1;
}
  void recievetrb(struct cctrb* trb){
    cns->puts("addrport=%d\n", addrport);
    unsigned char slot=trb->slot;
    struct TRB* t=(struct TRB*)trb->ctrb;
    cns->puts("slot=%d code=%d type=%d\n", slot, trb->code, t->type);
    if(t->type==9){
      drivers[slot]=0;
      cns->puts("addrport=%d portsc=%08x\n", addrport, ope->portset[addrport].portsc);
      slots[slot].port=addrport;
      ports[addrport].slot=slot;
      dcbaa[slot]=new struct devc;
      struct inputc* ic=new struct inputc;
      ic->icc.aflags=3;
      //ic->scc.routestring=0;
      ic->scc.roothubportnumber=addrport;
      ic->scc.contextentries=1;
      ic->scc.speed=(ope->portset[addrport].portsc>>10)&0xf;
      struct epc* epcont=&ic->epcont[0];
      tr[slot][0]=new CR;
      epcont->eptype=4;
      epcont->trdp=(unsigned long long)tr[slot][0]->ring|1;
      epcont->maxpacketsize=maxpacket(ic->scc.speed);
      epcont->cerr=3;
      struct addrTRB* at=new addrTRB;
      at->input=(unsigned long long)ic;
      at->slot=slot;
      at->bsr=0;
      cr->push((struct TRB*)at);
      db[0]=0;
      cns->puts("db=%016x\n", db);
      slots[slot].phase=addressingdevice;
      delete at;
    }else if(t->type==11){
      addrport=0;
      for(int i=1;i<=maxports;i++){
        if(ports[i].phase==waitfree)resetport(i);
      }
      slots[slot].phase=getcdesc;
      //for(int i=0;i<100000000;i++);
      /*struct setupTRB st{};
      st.bmrequesttype=0b10000000;
      st.brequest=6;
      st.wvalue=0x100;
      st.windex=0;
      st.wlength=18;
      st.trbtransferlength=8;
      st.trt=3;
      tr[slot][0]->push((struct TRB*)&st);
      struct dataTRB dt{};
      dt.pointer=searchmem(18);
      dt.trbtransferlength=18;
      dt.dir=1;
      dt.ioc=1;
      tr[slot][0]->push((struct TRB*)&dt);
      struct statusTRB stt{};
      stt.dir=0;
      stt.ioc=0;
      tr[slot][0]->push((struct TRB*)&stt);
      db[slot]=1;
      cns->puts("%016x\n", dcbaa[slot]->scc.contextentries);*/
      ope->portset[slots[slot].port].portsc&=~(1<<9);
      controltrans(slot, 0b10000000, 6, 0x200, 0, 0x100, searchmem(0x100), 1);
      //enableslot();
    }else if(t->type==12){
      controltrans(slot, 0, 9, slots[slot].ds.bconfigurationvalue, 0, 0, 0, 0);
    }else{
      cns->puts("OTher %d\n", t->type);
    }
  }
  void recievetrb(struct transfertrb *trb){
    int slot=trb->slot;
    //if(slot==1)setcr3(0);
    struct TRB* t=(struct TRB*)trb->pointer;
    //cns->puts("Trans slot=%d code=%d trbtransferlength=%x type=%d\n", slot, trb->code, trb->trbtransferlength, t->type);
    if(trb->code==4){
      cns->puts("Error slot=%d code=%d\n", slot, trb->code);
      slots[slot].phase=waitreset;
      unsigned char port=slots[slot].port;
      ports[port].phase=waitreset;
      ports[slot].haveerr=1;
      resetport(port);
    }
    if(slots[slot].phase==getcdesc){
      if(trb->code==1||trb->code==13){
        struct dataTRB* tb=(struct dataTRB*)t;
        unsigned char* p=(unsigned char*)tb->pointer;
        unsigned char* lp=(unsigned char*)((unsigned long long)p+(0x100-trb->trbtransferlength));
        bool once=false;
        struct inputc* icc=new struct inputc;
        icc->scc=dcbaa[slot]->scc;
        bool supported=false;
        slots[slot].fulld=p;
        while(lp>p){
          cns->puts("type=%d\n", p[1]);
          if(p[1]==2){
            struct configurationdescriptor* d=(struct configurationdescriptor*)p;
            cns->puts("configurationvalue %d\n", d->bconfigurationvalue);
            slots[slot].ds=*d;
          }else if(p[1]==4&&drivers[slot]==0){
            struct interfacedescriptor* i=(struct interfacedescriptor*)p;
            slots[slot].id=*i;
            cns->puts("device class=%d\n", i->binterfaceclass);
            if(i->binterfaceclass==3){
              drivers[slot]=new hid();
              supported=true;
              if(i->binterfacesubclass==1){
                cns->puts("using boot protocol\n");
                if(i->binterfaceprotocol==2){
                  cns->puts("This is Mouse!\n");
                  slots[slot].type=USBMouse;
                }else{
                  cns->puts("This is Keyboard!\n");
                  slots[slot].type=USBKeyboard;
                }
              }else{
                cns->puts("using report protocol\n");
                slots[slot].type=USBRdevice;
              }
            }else{
              drivers[slot]=new classd;
            }
          }else if(p[1]==5){
            struct endpointdescriptor* e=(struct endpointdescriptor*)p;
            unsigned char dci=e->bendpointaddress*2;
            dci+=(e->bendpointaddress>>7)&1;
            icc->icc.aflags|=1<<dci;
            struct epc* epcont=&icc->epcont[dci-1];
            epcont->eptype=e->bmattributes&3;
            epcont->eptype+=((e->bendpointaddress>>7)&1)*4;
            tr[slot][dci]=new CR;
            epcont->trdp=(unsigned long long)tr[slot][dci]->ring|1;
            epcont->cerr=3;
            epcont->interval=e->binterval;
            if((dci&1)&&(slots[slot].intin==0))slots[slot].intin=dci;
            cns->puts("dci=%d eptype=%d\n", dci, epcont->eptype);
          }else if(p[1]==33){
            cns->puts("report length=%d type=%d\n", *(unsigned short*)&p[7], *(unsigned char*)&p[6]);
          }
          p+=p[0];
        }
        struct confeptrb ct{};
        ct.slot=slot;
        ct.dc=0;
        ct.pointer=(unsigned long long)icc;
          cr->push((struct TRB*)&ct);
          db[0]=0;
        if(supported){
        }else{
          cns->puts("Not suported\n");
        }
        slots[slot].phase=setconf;
      }
    }else if(slots[slot].phase==setconf){
      cns->puts("starting ");
      slots[slot].phase=starting;;
      drivers[slot]->init(slot);
    }else{
      drivers[slot]->comp(trb);
    }
  }
  void posthandle(){
  //*(unsigned int*)0xfee000b0=0;
  struct TRB* erdp=(struct TRB*)(rr->ir[0].erdp&~0xf);
  while(erdp->c==c){
    struct TRB t=*erdp;
    erdp++;
    eri++;
    //cns->puts("eri=%d\n", eri);
   if(eri>=30){
      eri=0;
      erdp=(struct TRB*)erst->erba;
      c^=1;
    }
    rr->ir[0].erdp=(unsigned long long)erdp|(rr->ir[0].erdp&0xf);
    switch(t.type){
      case 34:
        recievetrb((struct psctrb*)&t);
        break;
      case 33:
        recievetrb((struct cctrb*)&t);
        break;
      case 32:
        recievetrb((struct transfertrb*)&t);
        break;
      default:
        cns->puts("Unknown type:%d\n", t.type);
        break;
    }
    rr->ir[0].ip=1;
    *(unsigned int*)0xfee000b0=0;
    io_out8(0x20, 0x62);
    io_out8(0xa0, 0x63);
  }
  }
  __attribute__((interrupt)) void xhcihandle(int* esp){
    rr->ir[0].ip=1;
    ope->usbsts|=8; 
    kernelbuf->write(1);
    *(unsigned int*)0xfee000b0=0;
    io_out8(0x20, 0x62);
    io_out8(0xa0, 0x63);
  }
  void init(){
    asm("cli");
    for(int i=0;i<pci::many;i++){
      if(pci::readpcidata(pci::pcis[i], 8)>>8==0x0c0330){
        xhc=pci::pcis[i];
        xhcp=true;
        if((pci::readpcidata(xhc, 0)&0xffff)==0x8086){
          break;
        }
      }
    }
    if(!xhcp)return;
    cns->puts("XHCI found\n");
    msip=false;
    unsigned char p=pci::readpcidata(xhc, 0x34)&0xff;
    while(p){
      unsigned char t=pci::readpcidata(xhc, p)&0xff;
      if(t==5){
        msip=true;
        break;
      }
      cns->puts("type=%d\n", t);
      p=(pci::readpcidata(xhc, p)>>8)&0xff;
    }
    eri=0;
    if(msip){
      union pcimsi mr;
      for(int i=0;i<4;i++)mr.data[i]=pci::readpcidata(xhc, p+i*4);
      mr.reg.control|=1;
      mr.reg.control&=~0x70;
      mr.reg.control|=0;
      cns->puts(" 64bit addr:%x\n", (mr.reg.control>>7)&0x1);
      mr.reg.address=0xfee00000;
      mr.reg.uaddr=0;
      mr.reg.data=0xc02b;
      for(int i=0;i<4;i++)pci::writepcidata(xhc, p+i*4, mr.data[i]);
    }else{
    open_irq(11);
    }
    cns->puts("using irq=%x\n", pci::readpcidata(xhc, 0x3c));
    unsigned long long mmio=pci::readpcidata(xhc, 0x10)&~0xf;
    mmio|=(unsigned long long)pci::readpcidata(xhc, 0x14)<<32;
    creg=(struct capreg*)mmio;
    db=(unsigned int*)(mmio+creg->dboff);
    rr=(struct RR*)(mmio+creg->rtsoff);
    ope=(struct opr*)(mmio+creg->caplength);
    ope->usbcmd|=2;
    while(ope->usbcmd&2);
    while(ope->usbcmd&(1<<11));
    for(int i=0;i<10000000;i++);
    cns->puts("PPC=%s\n",(creg->mccparams1>>3)&1 ? "true" : "false");
    ope->dcbaap=searchmem(8*8+8);
    dcbaa=(struct devc**)ope->dcbaap;
    slots=(struct slot*)searchmem(sizeof(struct slot)*8);
    ope->config=8;
    maxports=creg->hcsparams1>>24;
    cns->puts("MAx ports=%d\n", maxports);
    if(maxports==0)return;
    ports=(struct port*)searchmem(sizeof(struct port)*maxports);
    cr=new CR;
    ope->crcr=(unsigned long long)cr->ring|1;
    erst=new struct ERST;
    erst->erba=searchmem(16*30);
    erst->ersz=30;
    rr->ir[0].erstsz=1;
    rr->ir[0].erdp=(unsigned long long)erst->erba|8;;
    rr->ir[0].erstba=(unsigned long long)erst;
    rr->ir[0].interval=4000; 
    rr->ir[0].ip=rr->ir[0].ie=1;
    ope->usbcmd|=4;
    set_idt(0x2b, (unsigned long long)xhcihandle, 8, 0x8e);
    asm("sti");
    ope->usbcmd|=1;
    while(ope->usbsts&1);
    for(int i=1;i<=maxports;i++){
      ports[i].phase=waitreset;
      ports[i].haveerr=0;
      if((ope->portset[i].portsc&1)&&(ope->portset[i].portsc&(1<<17))){
        resetport(i);
      }
    }
    io_out8(0x20, 0x62);
    io_out8(0xa0, 0x63);
    rr->ir[0].ip=1;
    cns->puts("USBSTS=%08x\n", ope->usbsts);
  }
};
