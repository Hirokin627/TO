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
  unsigned int maxports;
  CR* cr;
  CR* tr[8][32];
  unsigned int eri=0;
  unsigned int *db;
  char c=1;
  struct ERST* erst;
  struct RR* rr;
  char addrport=0;
  classd* drivers[256][50];
  unsigned char getslot(unsigned char port){
    return ports[port].slot;
  }
  void clearbitportsc(unsigned char port, unsigned char b){
    unsigned int sc=ope->portset[port].portsc;
    sc&=~(1<<b);
    ope->portset[port].portsc=sc;
  }
  void resetport(unsigned char port){
    if(addrport>0){
      ports[port].phase=waitfree;
    }else if((ope->portset[port].portsc&1)){
      addrport=port;
      unsigned int sc=ope->portset[port].portsc;
      sc&= 0x0e00c3e0u;
      sc|=1<<4;
      sc|=1<<17;
      sc|=0x200000;
      sc|=0xf<<5;
      ope->portset[port].portsc=sc;
      while(ope->portset[port].portsc&0x10);
      ports[port].phase=resetting;
      //cns->puts("resetting %d\n", addrport);
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
  //cns->puts("port=%d\n", port);
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
  unsigned int decideintf(unsigned int slot, unsigned int ep){
    unsigned int di=-1;
    for(int i=0;i<50;i++){
      if(drivers[slot][i]!=0){
        for(int j=0;j<drivers[slot][i]->me;j++){
          if(ep==drivers[slot][i]->eps[j])di=drivers[slot][i]->id.binterfacenumber;
        }
      }
    }
    return di;
  }
  unsigned int getspeed(unsigned int slot){
    unsigned int portsc=ope->portset[slots[slot].port].portsc;
    return (portsc>>10)&0xf;
  }
  void recievetrb(struct cctrb* trb){
    unsigned char slot=trb->slot;
    struct TRB* t=(struct TRB*)trb->ctrb;
    //cns->puts("cmd=%d code=%d\n", t->type, trb->code);
    if(t->type==9){
      drivers[slot][0]=0;
      //cns->puts("addrport=%d portsc=%08x\n", addrport, ope->portset[addrport].portsc);
      slots[slot].port=addrport;
      ports[addrport].slot=slot;
      dcbaa[slot]=new struct devc;
      struct inputc* ic=new struct inputc;
      slots[slot].icc=ic;
      ic->icc.aflags=3;
      //ic->scc.routestring=0;
      ic->scc.roothubportnumber=addrport;
      ic->scc.contextentries=1;
      unsigned int portsc=ope->portset[addrport].portsc;
      portsc>>=10;
      portsc&=0xf;
      ic->scc.speed=portsc;
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
      slots[slot].phase=addressingdevice;
      db[0]=0;
      //cns->puts("db=%016x\n", db);
      delete at;
    }else if(t->type==11){
      //cns->puts("ep0=%d\n", dcbaa[slot]->epcont[0].epstate);
      addrport=0;
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
      //cns->puts("%016x\n", dcbaa[slot]->scc.contextentries);*/
      ope->portset[slots[slot].port].portsc&=~(1<<9);
      controltrans(slot, 0b10000000, 6, 0x200, 0, 0x100, searchmem(0x100), 1);
      //enableslot();
    }else if(t->type==12){
      if(t->rsv>>8){
        //cns->puts("deconf\n");
        struct disableslotTRB dt{};
        dt.slot=slot;
        cr->push((struct TRB*)&dt);
        db[0]=0;
      }else{
        //cns->puts("setting conf\n");
        controltrans(slot, 0, 9, slots[slot].ds.bconfigurationvalue, 0, 0, 0, 0);
      }
    }else if(t->type==10){
      bool deldesc=1;
      for(int i=0;i<50;i++){
        if(drivers[slot][i]){
          if(deldesc){
          }
          delete drivers[slot][i];
        }
      }
      delete dcbaa[slot];
    }else if(t->type==14){
      struct reseteptrb* rt=(struct reseteptrb*)t;
      unsigned int ep=rt->ep;
      unsigned int di=decideintf(slot, ep);
      //cns->puts("di=%d\n");
      if(di!=-1){
        drivers[slot][di]->reset=1;
      }else di=0;
      drivers[slot][0]->comp(0);
    }else{
      //cns->puts("OTher %d code=%d\n", t->type, trb->code);
    }
  }
  void resetep(unsigned int slot, unsigned int ep){
    asm("cli");
    struct reseteptrb* ret=new struct reseteptrb;
    unsigned int di=decideintf(slot, ep);
    ret->slot=slot;
    ret->ep=ep;
    ret->tsp=0;
    cr->push((struct TRB*)ret);
    db[0]=0;
  }
  unsigned int mostsignbit(unsigned int data){
    if(data==0)return -1;
    for(int i=0;i<32;i++){
      if((data<<i)&(1<<31)){
        return (31-i);
      }
    }
    return -1;
  }
  void recievetrb(struct transfertrb *trb){
    int slot=trb->slot;
    //if(slot==1)setcr3(0);
    struct TRB* t=(struct TRB*)trb->pointer;
    //cns->puts("Trans slot=%d code=%d trbtransferlength=%x type=%d\n", slot, trb->code, trb->trbtransferlength, t->type);
    int ep=trb->endpoint;
    int di=-1;
    di=decideintf(slot, ep);
    if(di==-1){
      struct setupTRB* tb=(struct setupTRB*)t;
      tb--;
      //cns->puts("setup type=%d\n", tb->type);
      di=tb->windex;
      if(tb->brequest==1){
        di=0;
      }
    }else{
    }
    
    /*if(trb->code==4){
      //cns->puts("Error slot=%d code=%d\n", slot, trb->code);
      slots[slot].phase=waitreset;
      unsigned char port=slots[slot].port;
      ports[port].phase=waitreset;
      ports[slot].haveerr=1;
      resetport(port);
    }*/
    if(trb->code!=1&&trb->code!=13){
      cns->puts("TRB Error detect:%d\n", trb->code);
    }
    if(slots[slot].phase==getcdesc){
      if(trb->code==1||trb->code==13){
        //cns->puts("Arrived getc phase\n");
        struct dataTRB* tb=(struct dataTRB*)t;
        unsigned char* p=(unsigned char*)tb->pointer;
        unsigned char* lp=(unsigned char*)((unsigned long long)p+(0x100-trb->trbtransferlength));
        bool once=false;
        struct inputc* icc=slots[slot].icc;
        for(int i=0;i<sizeof(struct inputc);i++)*(unsigned char*)((unsigned long long)icc+i)=0;
        icc->icc.aflags=1;
        icc->scc=dcbaa[slot]->scc;
        bool supported=false;
        slots[slot].ip=0;
        unsigned int in=0;
        for(int i=0;i<50;i++)drivers[slot][i]=0;
        while(lp>p){
          //cns->puts("type=%d\n", p[1]);
          if(p[1]==2){
            struct configurationdescriptor* d=(struct configurationdescriptor*)p;
            //cns->puts("configurationvalue %d\n", d->bconfigurationvalue);
            slots[slot].ds=*d;
          }else if(p[1]==4){
            struct interfacedescriptor* i=(struct interfacedescriptor*)p;
            in=i->binterfacenumber;
            //cns->puts("device class=%d in=%d\n", i->binterfaceclass, in);
            if(i->binterfaceclass==3){ 
              drivers[slot][in]=new hid();
              drivers[slot][in]->initialized=0;
              drivers[slot][in]->id=*i;
              drivers[slot][in]->fulld=(unsigned char*)i;
              slots[slot].ip++;
              supported=true;
              if(i->binterfacesubclass==1){
                //cns->puts("using boot protocol\n");
                if(i->binterfaceprotocol==2){
                  //cns->puts("This is Mouse!\n");
                  slots[slot].type=USBMouse;
                }else{
                  //cns->puts("This is Keyboard!\n");
                  slots[slot].type=USBKeyboard;
                }
              }else{
                //cns->puts("using report protocol\n");
                slots[slot].type=USBRdevice;
              }
            }else if(i->binterfaceclass==8){
              drivers[slot][in]=new mass;
              drivers[slot][in]->id=*i;
              drivers[slot][in]->fulld=(unsigned char*)i;
              supported=true;
            }else{
              drivers[slot][in]=new classd;
              drivers[slot][in]->id=*i;
            }
            drivers[slot][in]->reset=0;
            drivers[slot][in]->fulld=p;
          }else if(p[1]==5){
            struct endpointdescriptor* e=(struct endpointdescriptor*)p;
            unsigned char dci=(e->bendpointaddress&15)*2;
            dci+=(e->bendpointaddress>>7)&1;
            icc->icc.aflags|=1<<dci;
            struct epc* epcont=&icc->epcont[dci-1];
            epcont->eptype=e->bmattributes&3;
            epcont->maxpacketsize=e->wmaxpacketsize;
            epcont->eptype+=((e->bendpointaddress>>7)&1)*4;
            tr[slot][dci]=new CR;
            epcont->trdp=(unsigned long long)tr[slot][dci]->ring|1;
            epcont->cerr=3;
            if(getspeed(slot)==1||(getspeed(slot)==2)){
              if((epcont->eptype&3)==3){
                epcont->interval=mostsignbit(e->binterval*8)+3;
              }else if((epcont->eptype&3)==1){
                epcont->interval=e->binterval-1;
              }
            }else{
              epcont->interval=e->binterval-1;
            }
            epcont->averagetrblength=1;
            //cns->puts("dci=%d eptype=%d in=%d\n", dci, epcont->eptype, in);
            drivers[slot][in]->eps[drivers[slot][in]->me]=dci;
            drivers[slot][in]->me++;
          }else if(p[1]==33){
            //cns->puts("report length=%d type=%d\n", *(unsigned short*)&p[7], *(unsigned char*)&p[6]);
          }
          p+=p[0];
        }
        if(supported){
        }else{
          //cns->puts("Not suported\n");
        }
        slots[slot].phase=setconf;
        struct confeptrb cept{};
        for(int i=0;i<2;i++)*(unsigned long long*)((unsigned long long)&cept+i*8)=0;
        cept.type=12;
        //cns->puts("CONF EP SLOT=%d icc=%0llx\n", slot, icc);
        cept.slot=slot;
        cept.dc=0;
        cept.pointer=(unsigned long long)icc;
          cr->push((struct TRB*)&cept);
          db[0]=0;
      }
    }else if(slots[slot].phase==setconf){
      //cns->puts("starting ");
        //cns->puts("Arrived start class driver phase\n");
      slots[slot].phase=starting;;
      for(int i=0;i<50;i++){
        if(drivers[slot][i]!=0){
          drivers[slot][i]->init(slot);
          break;
        }
      }
    }else{
      for(int i=0;i<50;i++){
        if(drivers[slot][i]!=0){
          if(drivers[slot][i]->initialized==0){
            drivers[slot][i]->init(slot);
            break;
          }
        }
      }
      asm("sti");
      drivers[slot][di]->comp(trb);
      
      //asm("sti");
      for(int i=1;i<=maxports;i++){
        if(ports[i].phase==waitfree)resetport(i);
      }
    }
  }
  void posthandle(){
  //*(unsigned int*)0xfee000b0=0;
  unsigned int usbcmd=ope->usbsts;
  if(usbcmd&0x1000){
    //cns->puts("xHC ERROR\n");
    asm("cli\nhlt");
  }
  struct TRB* erdp=(struct TRB*)(rr->ir[0].erdp&~0xf);
  while(erdp->c==c){
    struct TRB t=*erdp;
    erdp++;
    eri++;
   if(eri>=30){
      eri=0;
      erdp=(struct TRB*)erst->erba;
      c^=1;
    }
    rr->ir[0].erdp=(unsigned long long)erdp|(rr->ir[0].erdp&0xf);
    asm("cli");
    switch(t.type){
      case 34:
        recievetrb((struct psctrb*)&t);
        //asm("sti");
        break;
      case 33:
        recievetrb((struct cctrb*)&t);
        //asm("sti");
        break;
      case 32:
        recievetrb((struct transfertrb*)&t);
        asm("sti");
        break;
      default:
        //cns->puts("Unknown type:%d\n", t.type);
        //asm("sti");
        break;
    }
    *(unsigned int*)&rr->ir[0]|=1;
  }
  }
  __attribute__((interrupt)) void xhcihandle(int* esp){
    *(unsigned int*)&rr->ir[0]|=1;
    if(ope->usbsts&0x1000){
      //cns->puts("xHCI Error found.\n");
      //cns->l->updown(layerd::top+1);
    }
    ope->usbsts|=8; 
    kernelbuf->write(1);
    *(unsigned int*)0xfee000b0=0;
    io_out8(0x20, 0x62);
    io_out8(0xa0, 0x63);
  }
  void init(){
    set_idt(0x2b, (unsigned long long)xhcihandle, 8, 0x8e);
    for(int i=0;i<pci::many;i++){
      if(pci::readpcidata(pci::pcis[i], 8)>>8==0x0c0330){
        xhc=pci::pcis[i];
        xhcp=true;
        //pause();
          unsigned int sp=pci::readpcidata(xhc, 0xdc);
          pci::writepcidata(xhc, 0xd8, sp);
          unsigned int ep=pci::readpcidata(xhc, 0xd4);
          pci::writepcidata(xhc, 0xd0, ep);
          //cns->puts("vendor: %04x\n", pci::readpcidata(xhc, 0)&0xffff);
        //pause();
          break;
      }
    }
    if(!xhcp)return;
    if((pci::readpcidata(xhc, 0)&0xffff)==0x8086){
      for(int i=0;i<pci::many;i++){
        if(pci::readpcidata(pci::pcis[i], 8)>>8==0x0c0320){
          break;
        }
      }
    }
    open_irq(11);
    //cns->puts("XHCI found\n");
    msip=false;
        //pause();
    unsigned char p=pci::readpcidata(xhc, 0x34)&0xff;
    while(p){
      unsigned char t=pci::readpcidata(xhc, p)&0xff;
      if(t==5){
        msip=true;
        break;
      }
      //cns->puts("type=%d\n", t);
      p=(pci::readpcidata(xhc, p)>>8)&0xff;
    }
    eri=0;
        //pause();
    if(msip){
      union pcimsi mr;
      for(int i=0;i<4;i++)mr.data[i]=pci::readpcidata(xhc, p+i*4);
      mr.reg.control|=1;
      mr.reg.control&=~0x70;
      mr.reg.control|=0;
      unsigned int bsp=*(unsigned int*)0xfee00020;
      bsp>>=24;
      cns->puts("BSP ID=%d\n", bsp);
      //cns->puts(" 64bit addr:%x\n", (mr.reg.control>>7)&0x1);
      mr.reg.address=0xfee00000;
      mr.reg.address|=bsp<<12;
      mr.reg.uaddr=0;
      mr.reg.data=0xc02b;
      for(int i=0;i<4;i++)pci::writepcidata(xhc, p+i*4, mr.data[i]);
    }else{
    }
    //cns->puts("using irq=%x\n", pci::readpcidata(xhc, 0x3c));
        //pause();
    unsigned long long mmio=pci::readpcidata(xhc, 0x10)&~0xf;
    mmio|=(unsigned long long)pci::readpcidata(xhc, 0x14)<<32;
    creg=(struct capreg*)mmio;
    db=(unsigned int*)(mmio+creg->dboff);
    //cns->puts("db base=%p\n", db);
    rr=(struct RR*)(mmio+creg->rtsoff);
    ope=(struct opr*)(mmio+creg->caplength);
    //cns->puts("hcc=%x\n", creg->hccparams1);
    unsigned long long cp=creg->hccparams1;
    cp>>=16;
    cp&=0xffff;
    cp<<=2;
    cp+=mmio;
    //cns->puts("cap base=%0llx\n", cp);
    if(cp>mmio){
      unsigned int* p=(unsigned int*)cp;
      unsigned int next;
      do{
        unsigned int cr=*p;
        //cns->puts("id=%d next=%d cr=%x\n", *p&0xff, (*p>>8)&0xff, cr);
        if((cr&0xff)==1){
          //cns->puts("LEGSUP found at %x\n", p);
          unsigned int legsup=cr;
          legsup&=1<<16;
          legsup|=1<<24;
          *p=legsup;
          while(legsup&(1<<16))legsup=*p;
          while(!(legsup&(1<<24)))legsup=*p;
        }
        next=*p;
        next>>=8;
        next&=0xff;
        next<<=2;
        
        //cns->puts("calcing next:%x\n", next);
        p=(unsigned int*)((unsigned long long)p+(((*p>>8)&0xff)<<2));
      }while(next);
    }
    ope->usbcmd|=2;
    while(ope->usbcmd&2);
    while(ope->usbcmd&(1<<11));
    for(int i=0;i<10000000;i++);
    ope->dcbaap=searchmem(8*8+8);
    dcbaa=(struct devc**)ope->dcbaap;
    unsigned short maxbuf=(creg->hcsparams2>>21)&0x1f;
    maxbuf<<=4;
    maxbuf|=creg->hcsparams2>>27;
    maxbuf=64;
    //cns->puts("Max scratch buf=%x\n", maxbuf);
        //pause();
    if(maxbuf>0){
      unsigned long long* bufa=(unsigned long long*)searchmem(8*maxbuf);
      for(int i=0;i<maxbuf;i++){
        bufa[i]=searchmem(4096);
      }
      dcbaa[0]=(struct devc*)bufa;
    }
    slots=(struct slot*)searchmem(sizeof(struct slot)*8);
    ope->config=8;
    maxports=creg->hcsparams1;
    maxports>>=24;
    //cns->puts("MAx ports=%d\n", maxports);
        //pause();
    if(ope->usbsts&0x1000){
      //cns->puts("xHCI Error found.\n");
      //cns->l->updown(layerd::top+1);
    }
    if(maxports==0)return;
    ports=(struct port*)searchmem(sizeof(struct port)*maxports);
    cr=new CR;
    ope->crcr=(unsigned long long)cr->ring|1;
    erst=new struct ERST;
    erst->erba=searchmem(16*30);
    erst->ersz=30;
    *(unsigned int*)&rr->ir[0].erstsz=1;
    rr->ir[0].erdp=(unsigned long long)erst->erba|8;;
    rr->ir[0].erstba=(unsigned long long)erst;
    *(unsigned int *)&rr->ir[0].interval=4000; 
    *(unsigned int*)&rr->ir[0]|=3;
    ope->usbcmd|=4;
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
    *(unsigned int*)&rr->ir[0]|=1;
    //cns->puts("USBSTS=%08x\n", ope->usbsts);
    //io_out8(0x64, 0xfe);
    asm("int $0x2b");
  }
unsigned char calcepaddr(unsigned char a){
  return (a&3)*2+(a>>7);
}
};
