#include "kernel.h"
struct capreg{
  unsigned char caplength;
  unsigned char :8;
  unsigned short hciversion;
  unsigned int hcsparams1;
  unsigned int hcsparams2;
  unsigned int hcsparams3;
  unsigned int mccparams1;
  unsigned int dboff;
  unsigned int rtsoff;
  unsigned int hccparams2;
}__attribute__((packed));
struct opr{
  unsigned int usbcmd;
  unsigned int usbsts;
  unsigned int pagesize;
  unsigned char r[0x13-0xc+1];
  unsigned int dnctrl;
  unsigned long long crcr;
  unsigned char rsv[0x2f-0x20+1];
  unsigned long long dcbaap;
  unsigned int config;
  unsigned char rsv2[0x3ff-0x3c+1-0x10];
  struct ps{
    unsigned int portsc;
    unsigned int portpmsc;
    unsigned int portli;
    unsigned int porthlpmc;
  }__attribute__((packed)) portset[0];
}__attribute__((packed));
struct TRB{
  unsigned int param[2];
  unsigned int status;
  unsigned char c:1;
  unsigned short rsv:9;
  unsigned char type:6;
  unsigned short control;
}__attribute__((packed));
struct enableslotTRB{
  unsigned int rsv[2];
  unsigned int :32;
  unsigned char c:1;
  unsigned short :9;
  unsigned char type:6;
  unsigned char slottype:5;
  unsigned int :11;
  
  enableslotTRB(){
    type=9;
  }
}__attribute__((packed));
struct psctrb{
  unsigned int :24;
  unsigned char port;
  unsigned int :32;
  unsigned int :24;
  unsigned char code;
  unsigned char c:1;
  unsigned short :9;
  unsigned char type:6;
  unsigned short :16;
}__attribute__((packed));
struct cctrb{
  struct TRB* ctrb;
  unsigned int param:24;
  unsigned char code;
  unsigned char c:1;
  unsigned short:9;
  unsigned char type:6;
  unsigned char vfid;
  unsigned char slot;
}__attribute__((packed));
struct epc{
  unsigned char epstate:3;
  unsigned char :5;
  unsigned char mult:2;
  unsigned char maxpstreams:5;
  unsigned char lsa:1;
  unsigned int interval:8;
  unsigned char maxesitpayloadhi;
  unsigned char :1;
  unsigned char cerr:2;
  unsigned char eptype:3;
  unsigned char :1;
  unsigned char hid:1;
  unsigned char maxburstsize;
  unsigned short maxpacketsize;
  unsigned long long trdp;
  unsigned short averagetrblength;
  unsigned short maxesitpayloadlo;
  unsigned int rsv[3];
}__attribute__((packed));
struct sc{
  unsigned int routestring:20;
  unsigned char speed:4;
  unsigned char :1;
  unsigned char mit:1;
  unsigned char hub:1;
  unsigned char contextentries:5;
  unsigned short maxexit;
  unsigned char roothubportnumber;
  unsigned char numberofports;
  unsigned char tthubslotid;
  unsigned char ttportnumber;
  unsigned char ttt:2;
  unsigned char :4;
  unsigned char target;
  unsigned char deviceaddr;
  unsigned int :19;
  unsigned char slotstate:5;
  unsigned int rsv[4];
}__attribute__((packed));
struct ic{
  unsigned int dflags;
  unsigned int aflags;
  unsigned int rsv[5];
  unsigned char configurationvalue;
  unsigned char interfacenumber;
  unsigned char alternatesetting;
  unsigned char :8;
  
}__attribute__((packed));
struct confeptrb{
  unsigned long long pointer;
  unsigned int :32;
  unsigned char c:1;
  unsigned char :8;
  unsigned char dc:1;
  unsigned char type:6;
  unsigned char :8;
  unsigned char slot;
  confeptrb(){
    type=12;
    dc=0;
  }
}__attribute__((packed));
struct endpointdescriptor{
  unsigned char blength;
  unsigned char bdescriptortype;
  unsigned char bendpointaddress;
  unsigned char bmattributes;
  unsigned short wmaxpacketsize;
  unsigned char binterval;
}__attribute__((packed));
struct interfacedescriptor{
  unsigned char blength;
  unsigned char bdescriptortype;
  unsigned char binterfacenumber;
  unsigned char balternatesetting;
  unsigned char bnumendpoints;
  unsigned char binterfaceclass;
  unsigned char binterfacesubclass;
  unsigned char binterfaceprotocol;
  unsigned char iinterface;
}__attribute__((packed));
struct devicedescriptor{
  unsigned char blength;
  unsigned char bdescriptortype;
  unsigned short bcdusb;
  unsigned char bdeviceclass;
  unsigned char bdevicesubclass;
  unsigned char bdeviceprotocol;
  unsigned char maxpacketsize0;
  unsigned short idvendor;
  unsigned short ifproduct;
  unsigned short bcddevice;
  unsigned char imanufacturer;
  unsigned char bnumconfigurations;
}__attribute__((packed));
struct configurationdescriptor{
  unsigned char blength;
  unsigned char descriptortype;
  unsigned short wtotallength;
  unsigned char bnuminterfaces;
  unsigned char bconfigurationvalue;
  unsigned char iconfiguration;
  unsigned char bmattributes;
  unsigned char bmaxpower;
}__attribute__((packed));
struct normalTRB{
  unsigned long long pointer;
  unsigned int trbtransferlength:17;
  unsigned char tdsize:5;
  unsigned short target:10;
  unsigned char c:1;
  unsigned char ent:1;
  unsigned char isp:1;
  unsigned char ns:1;
  unsigned char ch:1;
  unsigned char ioc:1;
  unsigned char idc:1;
  unsigned char :2;
  unsigned char bei:1;
  unsigned char type:6;
  unsigned short :16;
  normalTRB(){
    type=1;
    ioc=1;
    target=0;
  }
}__attribute__((packed));
struct inputc{
  struct ic icc;
  struct sc scc;
  struct epc epcont[31];
}__attribute__((packed));
struct devc{
  struct sc scc;
  struct epc epcont[31];
}__attribute__((packed));
struct addrTRB{
  unsigned long long input;
  unsigned int rsv;
  unsigned char c:1;
  unsigned char :8;
  unsigned char bsr:1;
  unsigned char type:6;
  unsigned char :8;
  unsigned char slot;
  addrTRB(){
    type=11;
  }
}__attribute__((packed));
struct linkTRB{
  unsigned long long pointer;
  unsigned int :22;
  unsigned short target:10;
  unsigned char c:1;
  unsigned char tc:1;
  unsigned char :2;
  unsigned char ch:1;
  unsigned char ioc:1;
  unsigned char :4;
  unsigned char type:6;
  unsigned short :16;
  linkTRB(){
    type=6;
    target=0;
  }
}__attribute__((packed));
struct disableslotTRB{
  unsigned int rsv[3];
  unsigned char c:1;
  unsigned short :9;
  unsigned char type:6;
  unsigned char :8;
  unsigned char slot;
  disableslotTRB(){
    type=10;
  }
}__attribute__((packed));
struct setupTRB{
  unsigned char bmrequesttype;
  unsigned char brequest;
  unsigned short wvalue;
  unsigned short windex;
  unsigned short wlength;
  unsigned int trbtransferlength:17;
  unsigned char :5;
  unsigned short target:10;
  unsigned char c:1;
  unsigned char :4;
  unsigned char ioc:1;
  unsigned char idc:1;
  unsigned char :3;
  unsigned char type:6;
  unsigned char trt:2;
  unsigned short :14;
  setupTRB(){
    type=2;
    idc=1;
    target=0;
  }
}__attribute__((packed));
struct dataTRB{
  unsigned long long pointer;
  unsigned int trbtransferlength:17;
  unsigned char tdsize:5;
  unsigned short target:10;
  unsigned char c:1;
  unsigned char ent:1;
  unsigned char isp:1;
  unsigned char ns:1;
  unsigned char ch:1;
  unsigned char ioc:1;
  unsigned char idc:1;
  unsigned char :3;
  unsigned char type:6;
  unsigned char dir:1;
  unsigned short :15;
  dataTRB(){
    type=3;
    ioc=1;
    target=0;
  }
}__attribute__((packed));
struct statusTRB{
  unsigned long long :64;
  unsigned int :22;
  unsigned short target:10;
  unsigned char c:1;
  unsigned char ent:1;
  unsigned char :2;
  unsigned char ch:1;
  unsigned char ioc:1;
  unsigned char :4;
  unsigned char type:6;
  unsigned char dir:1;
  unsigned short :15;
  statusTRB(){
    type=4;
    target=0;
  }
}__attribute__((packed));
struct transfertrb{
  unsigned long long pointer;
  unsigned int trbtransferlength:24;
  unsigned char code;
  unsigned char c:1;
  unsigned char :1;
  unsigned char ed:1;
  unsigned char :7;
  unsigned char type:6;
  unsigned char endpoint:5;
  unsigned char :3;
  unsigned char slot:8;
}__attribute__((packed));
class CR{
  public:
    CR(){
      c=1;
      wp=0;
      ring=(struct TRB*)searchmem(sizeof(struct TRB)*30);
      
    };
    ~CR(){
      freemem((unsigned long long)ring);
    };
    void push(struct TRB*);
    struct TRB* ring;
    int wp;
    char c;
};
struct msireg{
  unsigned char id;
  unsigned char next;
  unsigned short control;
  unsigned int address;
  unsigned int uaddr;
  unsigned short data;
}__attribute__((packed));
union pcimsi{
  int data[4];
  struct msireg reg;
};
struct ERST{
  unsigned long long erba;
  unsigned short ersz;
  unsigned short rsv1;
  unsigned int rsv2;
}__attribute__((packed));
struct RR{
  unsigned int MFINDEX;
  unsigned char rsv1[0x1f-0x4+1];
  struct IR{
    unsigned char ip:1;
    unsigned char ie:1;
    unsigned int rsv0:30;
    unsigned short interval;
    unsigned short counter;
    unsigned short erstsz;
    unsigned short rsv2;
    unsigned int rsv;
    unsigned long long erstba;
    unsigned long long erdp;
  }__attribute__((packed)) ir[1024];
}__attribute__((packed));

enum portp{
  waitreset,
  waitfree,
  resetting,
  enablingslot,
  addressingdevice,
  getddesc,
  getcdesc,
  setconf,
  setprotocol,
  starting,
  getdata,
  disablingep
};
enum devicetype{ 
  USBMouse,
  USBKeyboard,
  USBRdevice,
  USBRMouse,
  USBRKeyboard
};
struct port{
  enum portp phase;
  unsigned char slot;
    unsigned char haveerr;
};
struct slot{
  enum portp phase;
  unsigned char port;
  unsigned char intn;
  unsigned int intin;
  int isr;
  char ci;
  unsigned char nt;
  enum devicetype type;
    struct configurationdescriptor ds;
    int ip;
};
class classd{
  public:
    virtual void init(unsigned char s){
      
    }
    virtual void comp(struct transfertrb* t){
    }
    char initialized;
    unsigned char* fulld;
    struct interfacedescriptor id;
    unsigned char slot;
    unsigned int reportlength;
    int me;
    int eps[31];
};
class hid : public classd{
  public:
    void init(unsigned char s) override;
    void comp(struct transfertrb* t) override;
    unsigned int intin;
    int initphase;
    bool isr;
    unsigned char* buf;
    struct normalTRB* nt;
    unsigned int xoff,yoff,boff;
    unsigned int xsize,ysize,bsize;
    unsigned int xmax,xmin,ymax,ymin;
    unsigned int kaoff,kasize;
    unsigned char off;
};
class mass : public classd{
  public:
    void init(unsigned char s) override;
    void comp(struct transfertrb* t) override;
    int initphase;
    unsigned char* maxlun;
    struct normalTRB* intrb;
    unsigned int bulkin,bulkout;
};
namespace xhci{
  extern CR* tr[8][32];
  extern unsigned int *db;
  extern struct slot* slots;
  extern struct port* ports;
  void controltrans(unsigned char slot,unsigned char bmrequesttype, unsigned char brequest, unsigned short wvalue, unsigned short windex, unsigned short wlength, unsigned long long pointer, unsigned char dir);
  void resetport(unsigned char port);
  unsigned char calcepaddr(unsigned char a);
};
