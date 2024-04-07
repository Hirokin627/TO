#include "kernel.h"
namespace ided{ 
  void waitfordrv(){
    /*while(!(io_in8(0x1f7)&8)){
      if(io_in8(0x1f7)&1)break;
    }*/
  }
  unsigned char sendcmd(unsigned char p2, unsigned char p3, unsigned char p4, unsigned char p5, unsigned char p6, unsigned char p7){
    while(io_in8(0x1f7)&0x80){
      if(io_in8(0x1f7)&1)return io_in8(0x1f7);
    }
    io_out8(0x1f6, p6);
    io_out8(0x1f2, p2);
    io_out8(0x1f3, p3);
    io_out8(0x1f4, p4);
    io_out8(0x1f5, p5);
    io_out8(0x1f7, p7);
    while(io_in8(0x1f7)&0x80){
      if(io_in8(0x1f7)&1)return io_in8(0x1f7);
    }
    
    //if(!(io_in8(0x1f7)&1))waitfordrv();
    return io_in8(0x1f7);
  }
  void detectdevice(unsigned char addr){
    unsigned char stt=sendcmd(0, 0, 0, 0, 0xa0|(addr<<4), 0xec);
    cns->puts("checking addr=%d\n", addr);
    if(stt&1){
    io_out8(0x3f6, 4);
    io_out8(0x3f6, 2);
      unsigned char cmd[12]{
        0x25,0,0,0,0,0,0,0,0,0,0,0
      };
      cns->puts("this may be CD!\n");
      if(!(sendcmd(0, 0, 0, 0, 0xa0|(addr<<4), 0xa0)&1)){
        cns->puts("checking is this cd...\n");
        for(int i=0;i<6;i++)io_out16(0x1f0, *(unsigned short*)&cmd[i*2]);
        while(io_in8(0x1f7)&0x80);
        //waitfordrv();
        unsigned char d[8];
        unsigned char b[8];
        unsigned char* p=(unsigned char*)d;
        unsigned int sz=io_in8(0x1f4)|(io_in8(0x1f5)<<8);
        cns->puts("size=%d\n", sz);
        for(int i=0;i<sz/2;i++)*(unsigned short*)&p[i*2]=io_in16(0x1f0);
        for(int i=0;i<sz/2;i++){
          b[i]=d[3-i];
          b[i+4]=d[7-i];
        }
        unsigned int bpb=*(unsigned int*)&b[4];
        unsigned int lba=*(unsigned int*)&b[0];
        cns->puts("bpb=%x lba=%x\n", bpb, lba); 
        idecdrv* drv=new idecdrv(addr);
        drv->bpb=bpb;
        if(lba<0xffffffff&&lba)drvd::registdrv(1, 0, addr, (drive*)drv);
      }else{
        cns->puts("error code=%02x\n", io_in8(0x1f7));
      }
    }else{
      //while(!(io_in8(0x1f7)&8));
      idedrv* ide=new idedrv(addr);
      ide->bpb=0x200;
      for(int i=0;i<256/2;i++)*(unsigned short*)&ide->identd[i*2]=io_in16(0x1f0);
      cns->puts("LBA=%x\n", *(unsigned int*)&ide->identd[0x78]);
      asm("cli");
      unsigned char* buf=(unsigned char*)searchmem(512); //なんか最初はわけわかんないデータが来るので、テキトーなバッファーによませとく
      ide->read(buf, 1, 0);
      freemem((unsigned long long)buf);
      if(*(unsigned int*)&ide->identd[0x78]<0xfffffff&&*(unsigned int*)&ide->identd[0x78])drvd::registdrv(1, 0, addr, (drive*)ide);
    //while(io_in8(0x1f7)&8);
      asm("sti");
    }
  }
  void init(){
    io_out8(0x3f6, 4);
    io_out8(0x3f6, 2);
    for(int i=0;i<2;i++){
      detectdevice(i);
    }
  }
};
using namespace ided;
idedrv::idedrv(unsigned char a){
  addr=a;
  type=1;
  
}
idecdrv::idecdrv(unsigned char a){
  addr=a;
  type=1;
}
void idedrv::read(unsigned char* buf, unsigned int cnt, unsigned int lba512, unsigned int pn){
  if(pn!=-1)lba512+=pbase;
  for(int i=0;i<cnt;i++){
    if(sendcmd(1, (lba512+i), (lba512+i)>>8, (lba512+i)>>16, 0xe0|(addr<<4)|((lba512>>24)&0xf), 0x20)&1){
      cns->puts("read error\n");
      return;
    }
    //waitfordrv();
    for(int j=0;j<256;j++){
      *(unsigned short*)&buf[512*i+j*2]=io_in16(0x1f0);
    }
    while(io_in8(0x1f7)&8)io_in8(0x1f0);
  }
}
void idedrv::write(unsigned char* buf, unsigned int cnt, unsigned int lba512, unsigned int pn){
  if(pn!=-1)lba512+=pbase;
  for(int i=0;i<cnt;i++){
    if(sendcmd(1, (lba512+i), (lba512+i)>>8, (lba512+i)>>16, 0xe0|(addr<<4)|((lba512>>24)&0xf), 0x30)&1)return;;
    waitfordrv();
    for(int j=0;j<256;j++){
      io_out16(0x1f0, *(unsigned short*)&buf[i*512+j*2]);
    }
  }
}
int idedrv::getsectorsize(){
  return *(int*)&identd[0x78];
}
void idecdrv::phyread(unsigned char* buf, unsigned int lba512){
  unsigned char cmd[12]={
    0xa8,0,0
  };
  int p=2;
  for(int i=0x18;i>=0;i-=8,p++){
    cmd[p]=(lba512>>i);
  }
  for(int i=0x18;i>=0;i-=8,p++){
    cmd[p]=(1>>i);
  }
  sendcmd(0, 0, 2048&0xff, 2048>>8, 0xa0|(addr<<4), 0xa0);
  for(int i=0;i<6;i++)io_out16(0x1f0, *(unsigned short*)&cmd[i*2]);
  unsigned int size=io_in8(0x1f4)|(io_in8(0x1f5)<<8);
  for(unsigned int i=0;i<size/2;i++){
    while((io_in8(0x1f7)&0x80)|!(io_in8(0x1f7)&0x8));
    *(unsigned short*)&buf[i*2]=io_in16(0x1f0);
  }
}
void idecdrv::read(unsigned char* buf, unsigned int cnt, unsigned int lba512, unsigned int pn){
  asm("cli");
  unsigned char* tbuf=(unsigned char*)searchmem(bpb);
  asm("sti");
  for(int i=0;i<cnt;i++){
    unsigned int tlba=(lba512+i)*512/bpb;
    unsigned int bufp=(lba512+i)*512%bpb;
    phyread(tbuf, tlba);
    for(int j=0;j<512;j++){
      buf[i*512+j]=tbuf[bufp+j];
    }
  }
  asm("cli");
  freemem((unsigned long long)tbuf);
  asm("sti");
}
