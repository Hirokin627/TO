#include "kernel.h"
namespace ipd{
  unsigned short calcsum(unsigned short* buf, unsigned short tlen){
    unsigned int sum=0;
    for(int i=0;i<(tlen+1)/2;i++){
      unsigned short t=0;
      for(int j=0;j<4;j++){
        t|=(buf[i]>>((3-j)*4))<<(j*4);
      }
      sum+=t;
      cns->puts("%04x ", t);
    }
    cns->nline();
    if(sum>=0x10000){
      sum+=(sum>>16);;
      sum&=0xffff;
    }
    return ~sum;
  }
  unsigned long long convertbig(void* p, unsigned int size){
    unsigned long long v=*(unsigned long long*)p;
    unsigned char* cp=(unsigned char*)p;
    for(int i=0;i<size;i++){
      cp[i]=v>>((size-1-i)*8);
    }
    return *(unsigned long long*)cp;
  }
  unsigned long long conve(unsigned long long v, unsigned int size){
    return convertbig(&v, size);
  }
  void sendIP(unsigned char protocol, unsigned short len, unsigned char* data, unsigned char dest[4]){
    struct IPPacket* ipp=new struct IPPacket(len);
    ipp->protocol=protocol;
    for(int i=0;i<len;i++){
      *(unsigned char*)((unsigned long long)ipp+sizeof(struct IPPacket)+i)=data[i];
    }
    ipp->headercheck=0;
    for(int i=0;i<4;i++)ipp->dip[i]=dest[i];
    convertbig(&ipp->totall, 2);
    convertbig(&ipp->fragment, 2);
    ipp->headercheck=calcsum((unsigned short*)ipp, sizeof(struct IPPacket));
    cns->puts("sum=%x size=%d\n", ipp->headercheck, sizeof(struct IPPacket)+len);
    convertbig(&ipp->headercheck, 2);
    pcnetd::sendData(ipp, sizeof(struct IPPacket)+len, 0x800);
  }
  void recieve(void* buf, unsigned short len){
    struct IPPacket* ip=(struct IPPacket*)buf;
    //cns->puts("---------IP-----------\n");
    cns->puts("to ip:%04x from: %04x\n", conve(*(unsigned int*)ip->dip, 4), conve(*(unsigned int*)ip->dip, 4));
    convertbig(&ip->totall, 2);
    convertbig(&ip->headercheck, 2);
    convertbig(&ip->fragment, 2);
    unsigned int dataoff=ip->hlen*4;
    switch(ip->protocol){
      case 17:
        udpd::recieve((void*)((unsigned long long)buf+dataoff-8), len-dataoff+8);
        break;
    }
  }
};
