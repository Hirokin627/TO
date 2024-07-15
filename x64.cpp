#include "kernel.h"
unsigned long long gdt[]={
  0,
  0x00af9a000000ffff,
  0x00cf92000000ffff,
  0x00affa000000ffff,
  0x00cff2000000ffff
};
struct IDT* idt;
alignas(4096) unsigned long long op4[512];
alignas(4096) unsigned long long opd[64*512];
alignas(4096) unsigned long long ope[512*512];
struct iframe{
  unsigned long long rip;
}__attribute__((packed));
__attribute__((interrupt)) void GPhandle(iframe* f, unsigned long long ec){
  for(int i=0;i<scrxsize*scrysize;i++)vram[i]=0x237dff;
  svgad::putfifo(1);
  svgad::putfifo(0);
  svgad::putfifo(0);
  svgad::putfifo(scrxsize);
  svgad::putfifo(scrysize);
  svgad::writereg(21, 1);
  *(unsigned long long*)12=f->rip;
  setr10withhlt(f->rip);
  asm("cli\nhlt");
  return;
  layer l{400, 400};
  l.bxsize=scrxsize;
  l.bysize=scrysize;
  l.buf=(unsigned int*)vram;
  graphic::putfont(&l, 0xffffff, 0, 0, 'A', false);
  asm("cli\nhlt");
}
void set_idt(int n, unsigned long long offset, short sel, unsigned char attr){
  idt[n].o_l=offset&0xffff;
  idt[n].sel=sel;
  idt[n].attr=attr;
  idt[n].o_m=(offset>>16)&0xffff;
  idt[n].o_h=(offset>>32);
  idt[n].rsv=0;
}
void x64_init(){
  loadgdt(sizeof(gdt)-1, gdt);
  idt=(struct IDT*)searchmem(sizeof(struct IDT)*256);
  loadidt(sizeof(struct IDT)*256-1, idt);
  //op4=(unsigned long long*)searchmem(4096);
  //opd=(unsigned long long*)searchmem(8*64);
  //ope=(unsigned long long*)searchmem(8*512*64);
  op4[0]=(unsigned long long)&opd[0]|3;
  for(unsigned long long i=0;i<64;i++){
    opd[i]=(unsigned long long)&ope[i*512]|3;
    for(unsigned long long j=0;j<512;j++){
      ope[i*512+j]=(i*0x40000000)+(j*0x200000)+0x83;
    }
  }
  setcr3(op4);
  set_idt(0x08, (unsigned long long)GPhandle, 8, 0x8e);
}
void breakp4(unsigned long long* ap4){ 
  asm("cli");
  for(unsigned int p4=0;p4<512;p4++){
    if(ap4[p4]&1){
      unsigned long long* apd=(unsigned long long*)(ap4[p4]&~0xfff);
      for(unsigned int pd=0;pd<512;pd++){
        if(apd[pd]&1){
          unsigned long long* ape=(unsigned long long*)(apd[pd]&~0xfff);
          for(unsigned int pe=0;pe<512;pe++){
            freemem(ape[pe]&~0xfff);
          }
          freemem((unsigned long long)ape);
        }
      }
      freemem((unsigned long long)apd);
    }
  }
  freemem((unsigned long long)ap4);
}
unsigned long long* makep4(){
  asm("cli");
  unsigned long long* ap4=(unsigned long long*)searchmem(8*512);
  unsigned long long* apd=(unsigned long
  long*)searchmem(8*512);
  unsigned long long* ape=(unsigned long long*)searchmem(8*512*64);
  ap4[0]=(unsigned long long)&apd[0]|3;
  for(unsigned long long i=0;i<64;i++){
    apd[i]=(unsigned long long)&ape[i*512]|3;
    for(unsigned long long j=0;j<512;j++){
      ape[i*512+j]=(i*0x40000000)+(j*0x200000)+0x83;
    }
  }
  setcr3(ap4);
  return ap4;
}
void allocpagesub(unsigned long long* p4, addr_t vaddr, addr_t paddr, char flags){
  unsigned int p4p=(vaddr>>39)&0x1ff;
  if(!(p4[p4p]&1)){
    p4[p4p]=searchmem(8*512)|flags;
  }
  p4[p4p]=(p4[p4p]&~0xfff)|flags;
  unsigned long long* pd=(unsigned long long*)(p4[p4p]&~0xfff);
  unsigned int pdp=(vaddr>>30)&0x1ff;
  if(!(pd[pdp]&1)){
    pd[pdp]=searchmem(8*512)|flags;
  }
  pd[pdp]=(pd[pdp]&~0xfff)|flags;
  unsigned int pep=(vaddr>>21)&0x1ff;
  unsigned long long* pe=(unsigned long long*)(pd[pdp]&~0xfff);
  if(!(pe[pep]&1)){
    pe[pep]=searchmem(8*512)|flags;
  }
  if(pe[pep]&0x80){
    unsigned long long base=pe[pep]&~0xfff;
    unsigned long long *npt=(unsigned long long*)searchmem(8*512);
    for(unsigned int i=0;i<512;i++){
      npt[i]=base+i*0x1000|flags;
    }
    pe[pep]=(unsigned long long)npt|flags;
    /**/
  }
  pe[pep]=(pe[pep]&~0xfff)|flags;
  unsigned long long* pt=(unsigned long long*)(pe[pep]&~0xfff);
  unsigned int ptp=(vaddr>>12)&0x1ff;
  pt[ptp]=paddr|flags;
}
bool isallocated(unsigned long long* p4, addr_t vaddr){
  vaddr&=~0xfff;
  unsigned int p4p=(vaddr>>39)&0x1ff;
  if(!(p4[p4p]&1))return false;
  unsigned long long* pd=(unsigned long long*)(p4[p4p]&~0xfff);
  unsigned int pdp=(vaddr>>30)&0x1ff;
  if(!(pd[pdp]&1))return false;
  unsigned long long* pe=(unsigned long long*)(pd[pdp]&~0xfff);
  unsigned int pep=(vaddr>>21)&0x1ff;
  if(!(pe[pep]&1))return false;
  unsigned long long* pt=(unsigned long long*)(pe[pep]&~0xfff);
  unsigned int ptp=(vaddr>>12)&0x1ff;
  if(!(pt[ptp]&1))return false;
  return true;
}
unsigned long long getpaddr(unsigned long long* p4, unsigned long long vaddr){
  int p4p=(vaddr>>39)&0x1ff;
  if(!(p4[p4p]&1))return -1;
  unsigned long long* pd=(unsigned long long*)(p4[p4p]&~0xfff);
  int pdp=(vaddr>>30)&0x1ff;
  if(!(pd[pdp]&1))return -1;
  unsigned long long* pe=(unsigned long long*)(pd[pdp]&~0xfff);
  int pep=(vaddr>>21)&0x1ff;
  if(!(pe[pep]&1))return -1;
  if(pe[pep]&0x80)return pe[pep]&~0xfff;
  unsigned long long* pt=(unsigned long long*)(pe[pep]&~0xfff);
  int ptp=(vaddr>>12)&0x1ff;
  return (pt[ptp]&~0xfff)|(vaddr&0xfff);

}
unsigned char mpb=0xff;
unsigned char spb=0xff;
bool intf=true;
void cli(){
}
void sti(){
}
void allocpage(unsigned long long* p4, addr_t vaddr, addr_t paddr, size_t size, char flags){
  for(size_t i=0;i<(size+0xfff);i+=0x1000){
    allocpagesub(p4, vaddr+i, paddr+i, flags);
  }
}
