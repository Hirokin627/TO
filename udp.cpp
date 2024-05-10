#include "kernel.h"
namespace udpd{
  void recieve(void* pbuf, unsigned short len){
    unsigned long long fip=*(unsigned long long*)pbuf;
    //pbuf=(void*)((unsigned long long)pbuf+8);
    
    //len-=8;
    struct UDPPacket* up=(struct UDPPacket*)pbuf;
    //cns->puts("-----UDP---------\n");
    ipd::convertbig(&up->fromp, 2);
    ipd::convertbig(&up->top, 2);
    ipd::convertbig(&up->len, 2);
    ipd::convertbig(&up->checksum, 2);
    //cns->puts("from port=%d to port=%d\n", up->fromp, up->top);
    unsigned char* data=(unsigned char*)((unsigned long long)up+sizeof(struct UDPPacket));
    for(int i=0;i<len-8;i++){
      if(up->fromp==67||up->fromp==68){
        cns->puts("%02x ", data[i]);
      }
    }
    if(up->fromp==67||up->fromp==68)asm("cli\nhlt");
    else{
    }
   // cns->nline();
    //cns->puts("str: %s\n", data);
    //cns->nline();
  }
  void send(void* pbuf, unsigned short len, unsigned int tip, unsigned short fport, unsigned short tport){ 
    unsigned char* up=(unsigned char*)searchmem(len+8);
    struct UDPPacket* udp=(struct UDPPacket*)((unsigned long long)up-8);
    udp->fromp=fport;
    udp->top=tport;
    udp->len=len+8;
    udp->checksum=0;
    ipd::convertbig(&udp->fromp, 2);
    ipd::convertbig(&udp->top, 2);
    ipd::convertbig(&udp->len, 2);
    for(int i=0;i<len;i++)up[i+8]=*(unsigned char*)((unsigned long long)pbuf+i);
    unsigned char ip[4];
    *(unsigned int*)ip=tip;
    ipd::convertbig(ip, 4);
    ipd::sendIP(17, len+8, (unsigned char*)((unsigned long long)up), ip);
  }
};
