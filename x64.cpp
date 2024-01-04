#include "kernel.h"
unsigned long long gdt[]={
  0,
  0x00af9a000000ffff,
  0x00cf92000000ffff
};
struct IDT* idt;
unsigned long long* op4;
unsigned long long* opd;
unsigned long long* ope;
void x64_init(){
  loadgdt(sizeof(gdt)-1, gdt);
  idt=(struct IDT*)searchmem(sizeof(struct IDT)*256);
  loadidt(sizeof(struct IDT)*256-1, idt);
  op4=(unsigned long long*)searchmem(4096);
  opd=(unsigned long long*)searchmem(8*64);
  ope=(unsigned long long*)searchmem(8*512*64);
  op4[0]=(unsigned long long)&opd[0]|3;
  for(unsigned long long i=0;i<64;i++){
    opd[i]=(unsigned long long)&ope[i*512]|3;
    for(unsigned long long j=0;j<512;j++){
      ope[i*512+j]=(i*0x40000000)+(j*0x200000)+0x83;
    }
  }
  setcr3(op4);
}
void allocpagesub(unsigned long long* p4, addr_t vaddr, addr_t paddr, char flags){
  unsigned int p4p=(vaddr>>39)&0x1ff;
  if(!(p4[p4p]&1)){
    p4[p4p]=searchmem(8*512)|flags;
  }
  unsigned long long* pd=(unsigned long long*)(p4[p4p]&~0xfff);
  unsigned int pdp=(vaddr>>30)&0x1ff;
  if(!(pd[pdp]&1)){
    pd[pdp]=searchmem(8*512)|flags;
  }
  unsigned int pep=(vaddr>>21)&0x1ff;
  unsigned long long* pe=(unsigned long long*)(pd[pdp]&~0xfff);
  if(!(pe[pep]&1)){
    pe[pep]=searchmem(8*512)|flags;
  }
  if(pe[pep]&0x80){
    unsigned long long base=pe[pep]&~0xfff;
    pe[pep]=searchmem(8*512)|flags;
    unsigned long long* pts=(unsigned long long*)(pe[pep]&~0xfff);
    for(unsigned int i=0;i<512;i++){
      pts[i]=base+i*0x1000|flags;
    }
  }
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
  io_out8(0x21, 0xff);
  io_out8(0xa1, 0xff);
}
void sti(){
  io_out8(0x21, mpb);
  io_out8(0xa1, spb);
}
void allocpage(unsigned long long* p4, addr_t vaddr, addr_t paddr, size_t size, char flags){
  for(size_t i=0;i<(size+0xfff);i+=0x1000){
    allocpagesub(p4, vaddr+i, paddr+i, flags);
  }
}
void set_idt(int n, unsigned long long offset, short sel, unsigned char attr){
  idt[n].o_l=offset&0xffff;
  idt[n].sel=sel;
  idt[n].attr=attr;
  idt[n].o_m=(offset>>16)&0xffff;
  idt[n].o_h=(offset>>32);
  idt[n].rsv=0;
}
