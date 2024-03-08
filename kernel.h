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
    void putc(char chr, bool nf=true);
    void putsns(const char* str);
    void puts(const char* format,...);
    void nline();
    int lines,rows;
    layer* l;
    int cx,cy;
};
class terminal;
class task{
  public:
    task(unsigned long long ep);
    void run();
    void sleep();
    struct tc* ct;
    fifo* f;
    int cd;
    terminal* tm;
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
    virtual void read(unsigned char* buf, unsigned int cnt, unsigned int lba512){
    };
    virtual void write(unsigned char* buf, unsigned int cnt, unsigned int lba512){
    };
    unsigned int bpb;
    fs* files;
};
class mass;
class usbdrv : public drive{
  public:
    usbdrv(unsigned char slot, unsigned char interface);
    void read(unsigned char* buf, unsigned int cnt, unsigned int lba512) override;
    void write(unsigned char* buf, unsigned int cnt, unsigned int lba512) override;
    mass* intf;
};
class fs{
  public:
    virtual file* getf(const char* n, int dn){
      return 0;
    };
    virtual dirent* getd(const char* n, int dn){
      return 0;
    };
    virtual int getdn(const char* n, int dn){
      return -1;
    };
    virtual void init(drive* d){
      
    };
    unsigned int rc;
};
class fat : public fs{
  public:
    fat();
    ~fat();
    void init(drive*) override;
    int calcclus(int clus);
    struct fat_ent* getintdir(int clus);
    int readfat(int ind);
    void readclus(unsigned char* buf, int cnt, int clus);
    void readcluschain(unsigned char* buf, int clus);
    struct fat_ent* search_intent(const char* name, int dir);
    int getchainsize(int clus);
    file* getf(const char* n, int dir) override;
    dirent* getd(const char* n, int dir) override;
    int getdn(const char* n, int dir) override;
    unsigned char* ff;
    struct BPB* bpb;
    drive* dv;
    unsigned int* fats;
};
class terminal{
  public:
    void m(task* t);
    console* cns;
    task* tsk;
};
void memory_init(EFI_MEM* mems, unsigned long long dsize, unsigned long long bsize);
void x64_init();
unsigned long long getpaddr(unsigned long long* p4, unsigned long long vaddr);
unsigned long long searchmem(size_t size);
void allocpage(unsigned long long* p4, addr_t vaddr, addr_t paddr, size_t size, char flags);
void cli();
void freemem(addr_t addr);
void set_idt(int n, unsigned long long offset, short sel, unsigned char attr);
void pic_init();
void sti();
unsigned long long getpaddr(unsigned long long* p4, unsigned long long vaddr);
file* fopen(const char* name);
void allocpage(unsigned long long* p4, addr_t vaddr, addr_t paddr, size_t size, char flags);
unsigned long long* makep4();
void closef(file* f);
dirent* opendir(const char* name);
void closedir(dirent*);
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
};
namespace terminald{
  void main(task* tsk);
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
  void setcr4(unsigned long long data);
  void writemsr(unsigned int id, unsigned int data);
};
