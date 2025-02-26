#include "kernel.h"
unsigned char bitmap[0x20000];
const size_t last_addr=sizeof(bitmap)*8*4096;
struct alloclist al[90*512];
int alp=0;
char getbit(int addr){
  unsigned char b=bitmap[addr/8];
  b>>=7-(addr%8);
  return b&1;
}
void setbit(int addr){
  bitmap[addr/8]|=0x80>>(addr%8);
}
void clearbit(int addr){
  bitmap[addr/8]&=~(0x80>>(addr%8));
}
void freebit(addr_t addr, size_t size){
  for(int i=0;i<(size+0xfff)/0x1000;i++){
    clearbit(addr/4096+i);
  }
}
void reservmem(addr_t addr, size_t size){
  for(int i=0;i<size/0x1000;i++){
    setbit(addr/4096+i);
  }
}
unsigned long long iobase;
void memory_init(EFI_MEM* mems, unsigned long long dsize, unsigned long long bsize){
  for(int i=0;i<sizeof(bitmap)*8;i++)setbit(i);
  for(int i=0;i<bsize/dsize;i++){
    if(mems->type==EfiConventionalMemory||
    mems->type==EfiBootServicesCode||
    mems->type==EfiBootServicesData){
      for(int j=0;j<mems->numberofpage;j++){
        clearbit(mems->physicalstart/4096+j);
      }
    }
    /*if(mems->type==EfiMemoryMappedIO){
      iobase=mems->physicalstart/4096;
      for(int j=0;j<(mems->numberofpage+0xff)&~0xff;j++){
        clearbit(mems->physicalstart/4096+j);
      }
    }*/
    mems=(EFI_MEM*)((unsigned long long)mems+dsize);
  }
  reservmem(0, 0x100000);
}
void memory_init(unsigned char* bitmap2){
  for(unsigned int i=0;i<sizeof(bitmap);i++)bitmap[i]=bitmap2[i];
}
void mymemset(void* buf, size_t size, uint8_t data){
  char* b=(char*)buf;
  for(size_t i=0;i<size;i++){
    b[i]=data;
  }
}
unsigned long long searchmem(size_t size){ 
  asm("cli");
  size_t bsize=(size+0xfff)/0x1000;
  unsigned long long c=0,b=0;
  for(unsigned int i=0;i<sizeof(bitmap)*8;i++){
    if(!getbit(i)){
      c++;
      if(c==1)b=i;
      if(c==bsize){
        for(size_t j=0;j<bsize;j++){
          setbit(b+j);
        }
        al[alp].addr=b*4096;
        al[alp].size=bsize*0x1000;
        alp++;
        for(size_t j=0;j<bsize*4096;j++){
          *(unsigned char*)((unsigned long long)b*4096+j)=0;
        }
        return b*4096;
      }
    }else{
      c=0;
    }
  }
  asm("cli\nhlt");
  return -1;
}
unsigned long long searchmemforio(size_t size){
  asm("cli");
  size_t bsize=(size+0xfff)/0x1000;
  unsigned long long c=0,b=0;
  for(unsigned int i=0;i<sizeof(bitmap)*8;i++){
    if(!getbit(i)){
      c++;
      if(c==1)b=i;
      if(c==bsize){
        for(size_t j=0;j<bsize;j++){
          setbit(b+j);
        }
        al[alp].addr=b*4096;
        al[alp].size=bsize*0x1000;
        alp++;
        for(size_t j=0;j<bsize*4096;j++){
          *(unsigned char*)((unsigned long long)b*4096+j)=0;
        }
        return b*4096;
      }
    }else{
      c=0;
    }
  }
  asm("cli\nhlt");
  return -1;
}
void freemem(addr_t addr){
  size_t size=-1;
  int i;
  for(i=0;i<alp;i++){
    if(al[i].addr==addr){
      size=al[i].size;
      break;
    }
  }
  if(size==-1)return;
  for(int j=i;j<alp-1;j++){
    al[j]=al[j+1];
  }
  alp--;
  size/=0x1000;
  for(int i=0;i<size;i++)
    clearbit(addr/4096+i);
}
void* operator new (size_t size){
  return (void*)searchmem(size);
}
void operator delete(void* addr){
  freemem((addr_t)addr);
}
