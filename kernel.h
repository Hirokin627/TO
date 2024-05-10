typedef enum {
 EfiReservedMemoryType,
 EfiLoaderCode,
 EfiLoaderData,
 EfiBootServicesCode,
 EfiBootServicesData,
 EfiRuntimeServicesCode,
 EfiRuntimeServicesData,
 EfiConventionalMemory,
 EfiUnusableMemory,
 EfiACPIReclaimMemory,
 EfiACPIMemoryNVS,
 EfiMemoryMappedIO,
 EfiMemoryMappedIOPortSpace,
 EfiPalCode,
 EfiPersistentMemory,
 EfiMaxMemoryType
} EFI_MEMORY_TYPE;
extern unsigned long long op4[512];
typedef struct {
  unsigned char Type;
  unsigned char SubType;
  unsigned char Length[2];
} EFI_DEVICE_PATH_PROTOCOL;
#include "edk/bootpack.h"
#include "edk/elf.hpp"
#include <cstdint>
#include <cstdio>
#include <cstring>
#define ITS_WINDOW 2
#define ITS_CS 4
#define ITS_TB 8
#define ITS_MADE_FOR_APP 16
#define ITS_BUTTON 32
#define ITS_TEXTBOX 64
#define ITS_CONSOLE 128
class console;
class fifo;
class task;
class window;
extern int* vram;
extern int scrxsize,scrysize;
extern unsigned char bdl;
extern const char keytable0[0x80];
extern task* ta;
extern window* nowb;
extern console* cns;
extern fifo* kernelbuf;
extern int mx,my;
extern int termlock;
extern char cuser[60];
typedef unsigned long addr_t;
typedef void event(unsigned long long obj);
struct pcid{
	unsigned char bus,device,function;
};
struct IDT{
	unsigned short o_l;
	unsigned short sel;
	unsigned char zero;
	unsigned char attr;
	unsigned short o_m;
	unsigned int o_h;
	unsigned int rsv;
}__attribute__((packed));
struct profile{
  unsigned char sig[4];
  unsigned int bc;
  profile(){
    sig[0]='U';
    sig[1]='S';
    sig[2]='E';
    sig[3]='R';
  }
};
typedef struct {
char drv;
char* ptr;
int cnt;
int size;
char* base;
int based;
char flag;
char file;
int dir;
char attr;
char name[256];
char written;
} file;
#include "dirent.h"
struct fsinfo{
  unsigned int leadsig;
  unsigned char rsv1[480];
  unsigned int strucsig;
  unsigned int freecnt;
  unsigned int nxtfree;
  unsigned char rsv2[12];
  unsigned int trailsig;
}__attribute__((packed));
struct BPB {
  uint8_t jump_boot[3];
  char oem_name[8];
  uint16_t bytes_per_sector;
  uint8_t sectors_per_cluster;
  uint16_t reserved_sector_count;
  uint8_t num_fats;
  uint16_t root_entry_count;
  uint16_t total_sectors_16;
  uint8_t media;
  uint16_t fat_size_16;
  uint16_t sectors_per_track;
  uint16_t num_heads;
  uint32_t hidden_sectors;
  uint32_t total_sectors_32;
  uint32_t fat_size_32;
  uint16_t ext_flags;
  uint16_t fs_version;
  uint32_t root_cluster;
  uint16_t fs_info;
  uint16_t backup_boot_sector;
  uint8_t reserved[12];
  uint8_t drive_number;
  uint8_t reserved1;
  uint8_t boot_signature;
  uint32_t volume_id;
  char volume_label[11];
  char fs_type[8];
} __attribute__((packed));
struct part_ent{
  unsigned char bf;
  unsigned char sh;
  unsigned short cysc;
  unsigned char type;
  unsigned char endh;
  unsigned short ecysc;
  unsigned int lbaoff;
  unsigned int lbasize;
}__attribute__((packed));
struct fat_ent{
	unsigned char name[11];
	char attr;
	char NTres;
	char ctt;
	short crt;
	short crtd;
	short lstaccd;
	short clus_h;
	short wrtl;
	short wtd;
	short clus_l;
	int filesize;
	
	unsigned int getclus(){
	  return (clus_h<<16)|clus_l;
	}
}__attribute__((packed));
struct fat_lent{
	char ord;
	char name[10];
	char attr;
	char type;
	char checksum;
	char name2[12];
	short fstcl;
	char name3[4];
}__attribute__((packed));
struct tc{
	unsigned long long cr3, rip, rflags, rsv;
	unsigned long long cs,ss,fs,gs;
	unsigned long long rax,rbx,rcx,rdx,rdi,rsi,rsp,rbp;
	unsigned long long r8,r9,r10,r11,r12,r13,r14,r15;
	unsigned char fx_area[512];
}__attribute__((packed));
struct alloclist{
  addr_t addr;
  size_t size;
};
union moused{
  int d;
  struct {
    char btn;
    char x;
    char y;
    char rsv;
  } byte;
};
union keybd{
  struct {
    unsigned int d[2];
  } input;
  struct {
    unsigned char d[8];
  } byte;
};
class layer{
  public:
    layer(int xsize, int ysize);
    ~layer();
    void updown(int h);
    void refresh();
    void refreshconfro(int,int,int,int);
    void slide(int,int);
    unsigned int* buf;
    int bxsize,bysize;
    int x,y;
    unsigned int col_inv;
    int height;
    int manye;
    void registss(layer* s);
    window* wc;
    unsigned int flags;
    layer* slaves[256];
    layer* master;
    unsigned long long owner;
    event* oncrick;
};
class console{
  public:
    console(int line, int row);
    ~console(){
      delete l;
    }
    void putc(char chr, bool nf=true);
    void putsns(const char* str);
    void puts(const char* format,...);
    void nline();
    unsigned int fc;
    int bc;
    int lines,rows;
    layer* l;
    char buf[60];
    int cx,cy;
};
class terminal;
void freemem(addr_t addr);
class task{
  public:
    task(unsigned long long ep);
    ~task(){
      freemem(irsp);
      delete ct;
    }
    void run();
    void sleep();
    unsigned long long brsp;
    unsigned long long irsp;
    struct tc* ct;
    fifo* f;
    int cd;
    unsigned int flags;
    task* parent;
    terminal* tm;
    struct alloclist al[200];
    int alp;
};
class fifo{
  public:
    fifo(int sz, task* t=0);
    void write(unsigned long long d);
    unsigned long long read();
    unsigned long long front(){return datas[rp];};
    unsigned long long* datas;
    int rp,wp;
    char lock;
    int len,size;
    task* tsk;
};
class window{
  public:
    window(int,int);
    ~window(){
      delete cs;
      delete edge;
      delete tb;
    };
    void setactive(bool ac);
    layer* cs;
    layer* edge;
    layer* tb;
    task* owner;
};
typedef void event(unsigned long long obj);
class button{
  public:
    button(int, int, const char* label=0);
    ~button(){
      delete l;
    };
    layer* l;
};
class textbox{
  public:
    textbox(unsigned csize);
    ~textbox(){
      delete l;
      delete c;
    };
    layer* l;
    console* c;
    char chrs[60];
    int chrp;
};
class timer{
  public:
    void set(unsigned int c, unsigned char cy=0);
    unsigned int timeout;
    unsigned char cyc;
    timer* next;
    timer* prev;
    unsigned int flags;
};
class fs;
class drive{
  public:
    virtual void read(unsigned char* buf, unsigned int cnt, unsigned int lba512, unsigned int pn=0){
    };
    virtual void write(unsigned char* buf, unsigned int cnt, unsigned int lba512, unsigned int pn=0){
    };
    virtual int getsectorsize(){
      return 0;
    };
    void createfs();
    unsigned int bpb;
    unsigned int type;
    unsigned int pbase;
    fs* files;
};
class mass;
class usbdrv : public drive{
  public:
    usbdrv(unsigned char slot, unsigned char interface);
    void read(unsigned char* buf, unsigned int cnt, unsigned int lba512, unsigned int pn=0) override;
    void write(unsigned char* buf, unsigned int cnt, unsigned int lba512, unsigned int pn=0) override;
    mass* intf;
};
class idedrv: public drive{
  public:
    idedrv(unsigned char addr);
    unsigned char identd[512];
    void read(unsigned char* buf, unsigned int cnt, unsigned int lba512, unsigned int pn=0) override;
    void write(unsigned char* buf, unsigned int cnt, unsigned int lba512, unsigned int pn=0) override;
    int getsectorsize() override;
    unsigned char addr;
};
class idecdrv : public drive{
  public:
    idecdrv(unsigned char addr);
    void phyread(unsigned char* buf, unsigned int lba512);
    void read(unsigned char* buf, unsigned int cnt, unsigned int lba512, unsigned int pn=0) override;
    unsigned char addr;
};
class fs{
  public:
    fs();
    virtual void init(drive* drv){
      
    };
    ~fs();
    virtual void preparecluschain(unsigned int clus){
    };
    virtual unsigned int calcblock(unsigned int clus){
      return -1;
    };
    virtual unsigned char* getclusaddr(unsigned int clus){
      return 0;
    };
    virtual struct fat_ent* getd(const char* n, int dn){
      return 0;
    };
    virtual unsigned int getfat(unsigned int ind){
      return 0;
    };
    virtual void writefat(int ind, unsigned int d){
    };
    virtual struct fat_ent* findfile(const char* n, int dir=0){
      return 0;
    };
    virtual void writef(struct fat_ent*, unsigned char* buf, int size, int dir=0){
    };
    virtual void writecluschain(unsigned char* buf, int clus, int size){
    };
    virtual struct fat_ent* createe(const char* n, int dir=0){
      return 0;
    };
    virtual void loadfile(struct fat_ent* f, unsigned char* b){
    };
    unsigned int rc;
    unsigned char dl;
    unsigned char* ff;
    unsigned char* buf;
    drive* d;
};
class fat : public fs{
  public:
    void init(drive* d) override;
    fat();
    ~fat();
    unsigned int allocfat();
    void generatee(struct fat_ent* e, const char* n, int dir);
    struct fat_ent* getnext(struct fat_ent* e, int dir);
    unsigned int getchainsize(int c);
    const char* setmustdir(const char* name, int *dir);
    void writef(struct fat_ent* f, unsigned char* buf, int size, int dir=0) override;
    unsigned int getfat(unsigned int ind) override;
    unsigned int calcblock(unsigned int clus) override;
    void preparecluschain(unsigned int clus=0) override;
    unsigned char* getclusaddr(unsigned int clus) override;
    void writefat(int ind, unsigned int d) override;
    struct fat_ent* findfile(const char* n, int dir=0) override;
    void writecluschain(unsigned char* buf, int clus, int size) override;
    struct fat_ent* createe(const char* n, int dir=0) override;
    void loadfile(struct fat_ent* f, unsigned char* b)override;
    struct BPB* bpb;
    unsigned int* fats;
};
namespace fatd{
  
  void makename(struct fat_lent* l, char* n);
};
class terminal{
  public:
    void m(task* t);
    char getc();
    unsigned char* gets();
    console* cns;
    window* w;
    task* tsk;
};
void memory_init(EFI_MEM* mems, unsigned long long dsize, unsigned long long bsize);
void x64_init();
unsigned long long getpaddr(unsigned long long* p4, unsigned long long vaddr);
unsigned long long searchmem(size_t size);
void allocpage(unsigned long long* p4, addr_t vaddr, addr_t paddr, size_t size, char flags);
void cli();
void fputc(int c, file* f);
void set_idt(int n, unsigned long long offset, short sel, unsigned char attr);
void pic_init();
void sti();
unsigned long long getpaddr(unsigned long long* p4, unsigned long long vaddr);
//file* fopen(const char* name);
void allocpage(unsigned long long* p4, addr_t vaddr, addr_t paddr, size_t size, char flags);
unsigned long long* makep4();
void closef(file* f);
dirent* opendir(const char* name);
void breakp4(unsigned long long* ap4);
void closedir(dirent*);
void createf(const char* name);
void api_init();
void open_irq(char irq);
namespace layerd{
  void init();
  void refreshsub(int x0, int y0, int x1, int y1);
  void trefreshsub(int x0, int y0, int x1, int y1);
  extern int top;
  extern layer* bl;
  layer* checkcrick(int mx, int my);
};
namespace graphic{
  void drawbox(layer* l, int c, int x0, int y0, int x1, int y1, bool nf=true);
  void putfont(layer* l, int c, int bx, int by, char chr, bool nf=true);
  void putfontstr(layer* l, int c, int bx, int by, const char* str, bool nf=true);
};
namespace pci{
  void init();
  unsigned int readpcidata(struct pcid d, unsigned char offset);
  void writepcidata(struct pcid d, unsigned char offset, unsigned int data);
  extern struct pcid* pcis;
  extern unsigned int many;
};
namespace xhci{
  
  void posthandle();
  void init();
  unsigned char getslot(unsigned char port);
};
namespace ps2{
  void init();
};
namespace timerd{
  void init();
  void sleep(unsigned int ms10);
};
namespace mtaskd{
  void taskswitch(bool cc=false);
  task* init();
  extern timer* mt;
  extern task* current;
};
namespace acpi{
  void init(struct RSDP*);
  void shutdown();
};
namespace drvd{
  void init(EFI_DEVICE_PATH_PROTOCOL* bdp);
  unsigned char registdrv(unsigned char type, unsigned char mainaddr, unsigned char subaddr, drive* drv);
  extern drive* drvs[256];
};
namespace fsd{
  void recognizefs(unsigned char d);
  void init();
  void copyfatdir(fat* froms, struct fat_ent* fent, int fdir, fat* tos, struct fat_ent* toent, int todir);
};
namespace terminald{
  void main(task* tsk);
};
namespace satad{
  void init();
};
namespace ided{
  void init();
};
namespace rtcd{
  void init();
  extern unsigned short y,m,d,h,mt,s;
};
namespace udpd{
  void recieve(void* pbuf, unsigned short len);
  void send(void* pbuf, unsigned short len, unsigned int tip, unsigned short fport, unsigned short tport);
};

namespace ipd{
  void sendIP(unsigned char protocol, unsigned short len, unsigned char* data, unsigned char dest[4]);
  void recieve(void* buf, unsigned short len);
  unsigned long long convertbig(void* p, unsigned int size);
  unsigned long long conve(unsigned long long v, unsigned int size);
};
extern unsigned int lip;
  struct IPPacket{
    unsigned char hlen:4;
    unsigned char ver:4;
    unsigned char ecn:2;
    unsigned char dscp:6;
    unsigned short totall;
    unsigned short fragment;
    unsigned char flags:4;
    unsigned short fragmentoff:12;
    unsigned char ttl;
    unsigned char protocol;
    unsigned short headercheck;
    unsigned char sip[4];
    unsigned char dip[4];
    //unsigned int option:24;
    IPPacket(unsigned short datalen){
      ver=4;
      hlen=sizeof(struct IPPacket)/4;
      totall=sizeof(struct IPPacket)+datalen;
      ttl=100;
      *(unsigned int*)sip=ipd::conve(lip, 4);
    };
  }__attribute__((packed));
  struct UDPPacket{
    unsigned char fip[4];
    unsigned char tip[4];
    unsigned short fromp;
    unsigned short top;
    unsigned short len;
    unsigned short checksum;
  }__attribute__((packed));
namespace pcnetd{
  void init();
  int sendPacket(void *packet, size_t len, uint8_t *dest);
  void polling();
  void sendData(void* buf, unsigned short len, unsigned short protocol);
};
extern "C"{
  void setcr3(unsigned long long*);
  unsigned long long* getcr3();
  void io_out8(unsigned short, unsigned char);
  unsigned char io_in8(unsigned short);
  void loadgdt(short, unsigned long long*);
  void loadidt(short, struct IDT*);
  unsigned int io_in32(short);
  void io_out32(short, unsigned int);
  void io_out16(unsigned short, unsigned short);
  unsigned int readmsr(unsigned int id);
  unsigned int rflags();
  void asmapihandle();
  void srflags(unsigned int);
  void switchcont(struct tc*, struct tc*);
  unsigned short io_in16(unsigned short);
  unsigned long long getcr4();
  void jumpasapp(int argc, char** argv, unsigned long long rip, task* tsk);
  void setcr4(unsigned long long data);
  void setr10withhlt(unsigned long long data);
  void writemsr(unsigned int id, unsigned int data);
};
