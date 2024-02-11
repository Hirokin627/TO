#include "kernel.h"
namespace drvd{
  drive* drvs[256];
  unsigned char bdtype;
  unsigned char bdmainaddr;
  unsigned char bdsubaddr;
  void init(EFI_DEVICE_PATH_PROTOCOL* bdp){
    if(bdp->Type==3){
      if(bdp->SubType==5){
        bdmainaddr=*(unsigned char*)((unsigned long long)bdp+4)+1;
        bdsubaddr=*(unsigned char*)((unsigned long long)bdp+5);
        bdtype=bdp->SubType;
      }
    }
  }
  unsigned char registdrv(unsigned char type, unsigned char mainaddr, unsigned char subaddr, drive* drv){
    unsigned char dl=0;
    for(int i='A';i<256;i++){
      if(drvs[i]==0){
        dl=i;
        //cns->puts("drvl=%c\n" ,dl);
        break;
      }
    }
    if(type==5){
      if(mainaddr==bdmainaddr){
        if(subaddr==bdsubaddr){
          bdl=dl;
        }
      }
    }
    drvs[dl]=drv;
    return dl;
  }
};
