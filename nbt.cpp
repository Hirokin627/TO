#include "kernel.h"
void nbtprocess(unsigned char* data, unsigned int len, unsigned int fip){
  struct NBTHEAD* nh=(struct NBTHEAD*)data;
  //cns->puts("qcode=%x\n", nh->opcode);
  //cns->puts("nmf=%x\n", nh->nmf());
  if((nh->opcode==0)&&((nh->nmf()&~1)==0x10)){ 
    //cns->puts("NAME REQUEST\n");
    struct NBTHEAD* rh=(struct NBTHEAD*)searchmem(100);
    unsigned char* nstr=(unsigned char*)((unsigned long long)rh+sizeof(struct NBTHEAD));
    strcpy((char*)nstr, (const char*)((unsigned long long)data+sizeof(struct NBTHEAD)));
    nstr+=strlen((const char*)nstr)+1;
    rh->id=nh->id;
    rh->opcode=1<<4;
    rh->setnmf(0b1011000);
    rh->ancount=ipd::conve(1, 2);
  }
}
char* checkname(char* name, unsigned int* len){
  unsigned int l=name[0];
  *len=l;
  char* n=(char*)searchmem(name[0]/2+1);
  n[name[0]/2]=0;
  for(int i=0;i<l;i+=2){
    n[i/2]=((name[i+1]-0x41)<<4)|(name[i+2]-0x41);
  }
  return n;
}
char* calcname(const char* name){
  unsigned int len=strlen(name)*2;
  char* n=(char*)searchmem(len+2);
  for(int i=0;i<len;i+=2){
    n[i+1]=((unsigned char)name[i/2]>>4)+0x41;
    n[i+2]=(name[i/2]&0xf)+0x41;
  }
  n[len-1]=0;
  n[0]=len;
  return n;
}
