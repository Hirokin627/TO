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
typedef struct {
char drv;
char* ptr;
char written;
int cnt;
int size;
char* base;
int based;
char flag;
char file;
int dir;
char attr;
char name[256];
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
    int lines,rows;
    layer* l;
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
    void setactive(bool ac);
    layer* cs;
    layer* edge;
    layer* tb;
    task* owner;
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
    void writef(struct fat_ent* f, unsigned char* buf, int size, int dir=0) override;
    unsigned int getfat(unsigned int ind) override;
    unsigned int calcblock(unsigned int clus) override;
    void preparecluschain(unsigned int clus=0) override;
    unsigned char* getclusaddr(unsigned int clus) override;
    void writefat(int ind, unsigned int d) override;
    struct fat_ent* findfile(const char* n, int dir=0) override;
    void writecluschain(unsigned char* buf, int clus, int size) override;
    struct fat_ent* createe(const char* n, int dir=0) override;
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
  layer* checkcrick(int mx, int my);
};
namespace graphic{
  void drawbox(layer* l, int c, int x0, int y0, int x1, int y1, bool nf=true);
  void putfont(layer* l, int c, int bx, int by, char chr, bool nf=true);
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
