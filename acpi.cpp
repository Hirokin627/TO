#include "kernel.h"
namespace acpi{
  struct FADT* fadt;
  void init(struct RSDP* rsdp){
    struct XSDT* xsdt=(struct XSDT*)rsdp->xsdt;
    for(int i=0;i<(xsdt->header.length-36)/8;i++){
      if(!strncmp((const char*)xsdt->entrys[i], "FACP", 4)){
        fadt=(struct FADT*)xsdt->entrys[i];
        break;
      }
    }
  }
  void shutdown(){
    unsigned int r=rflags();
    asm("cli");
    unsigned int PM1a=fadt->PM1a_CNT_BLK;
    unsigned int PM1b=fadt->PM1b_CNT_BLK;
    unsigned char* s5=(unsigned char*)fadt->dsdt;
    while(1){
      if(!strncmp((const char*)s5, "_S5_", 4)){
        break;
      }
      s5++;
    }
    s5+=7;
    unsigned char al,ah;
    unsigned short ax;
    unsigned char* p=s5;
    switch(p[0]){
      case 0x0b:
      case 0x0c:
      case 0x0e:
        al=p[1];
        al=p[2];
        break;
      default:
        al=p[0];
        if(al==0x0a){
          al=p[1];
          p++;
        }
        ah=p[1];
        if(ah==0x0a){
          ah=p[2];
          p++;
        }
        break;
    }
    ax=(ah<<8)|al;
    unsigned short d=(ax<<10)|(1<<13);
    io_out16(PM1a, d);
    io_out16(PM1b, d);
    srflags(r);
  }
};
