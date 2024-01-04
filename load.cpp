#include "kernel.h"
int* vram;
int scrxsize,scrysize;
alignas(0x200000) unsigned char buf[0x400000];
struct arg a;
extern unsigned char bitmap[0x20000];
alignas(4096) unsigned long long pm4[512];
alignas(4096) unsigned long long opd[512];
alignas(4096) unsigned long long ope[64][512];
alignas(4096) unsigned long long bpd[512];
alignas(4096) unsigned long long bpe[512];
extern "C" void Kernelmain(struct arg* ai, char* kbuf){
  a=*ai;
  vram=ai->Frame.fb;
  scrxsize=ai->Frame.xsize;
  scrysize=ai->Frame.ysize;
  vram[0]=0xff0000;
  memory_init(ai->mems, ai->size, ai->bsize);
  for(int i=0;i<512;i++)pm4[i]=0;
  pm4[0]=(unsigned long long)&opd[0]|3;
  for(unsigned int i=0;i<64;i++){
    opd[i]=(unsigned long long)&ope[i][0]|3;
    for(unsigned int j=0;j<512;j++){
      ope[i][j]=i*0x40000000+j*0x200000|0x83;
    }
  }
  setcr3(pm4);
  x64_init();
  struct elf_fh* fh=(struct elf_fh*)kbuf;
  struct elf_ph* ph=(struct elf_ph*)((unsigned long long)fh+fh->phoff);
  unsigned long long start=0xffffffffffffffff,end=0;
  for(int i=0;i<fh->phnum;i++){
    if(ph[i].type!=1)continue;
    if(start>ph[i].vaddr)start=ph[i].vaddr;
    if(end<ph[i].vaddr+ph[i].memsz)end=ph[i].vaddr+ph[i].memsz;
  }
  allocpage(getcr3(), start, searchmem(end-start+1), end-start+1, 3);
  for(int i=0;i<fh->phnum;i++){
    unsigned char* vaddr=(unsigned char*)ph[i].vaddr;
    unsigned char* seg=(unsigned char*)((unsigned long long)fh+ph[i].offset);
    for(unsigned long long j=0;j<ph[i].filesz;j++)vaddr[j]=seg[j];
    for(unsigned long long j=ph[i].filesz;j<ph[i].memsz;j++)vaddr[j]=0;
  }
  typedef void ke(struct arg*, unsigned long long);
  ke* entry=(ke*)*(unsigned long long*)((unsigned long long)fh+24);
  a.bm=(char*)bitmap;
  entry(&a, searchmem(1024*1024));
  asm("cli\nhlt");
}
