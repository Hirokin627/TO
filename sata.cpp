#include "kernel.h"
#include "sata.h"
namespace satad{
  struct pcid ahc;
  struct sata_mmio* stm;
  void portreset(unsigned int port){
    unsigned int pxcmd=stm->portc[port].pxcmd;
    pxcmd&=~1;
    stm->portc[port].pxcmd=pxcmd;
    unsigned int pxsctl=stm->portc[port].pxsctl;
    pxsctl&=~0xf;
    stm->portc[port].pxsctl=pxsctl;
  }
  void init(){
    bool ahcp=false;
    for(int i=0;i<pci::many;i++){
      if((pci::readpcidata(pci::pcis[i], 8)>>8)==0x010601){
        ahc=pci::pcis[i];
        ahcp=true;
      }
    }
    if(!ahcp)return;
    cns->puts("AHC found\n");
    unsigned long long mmio=pci::readpcidata(ahc, 0x24)&~0xf;
    //mmio|=(unsigned long long)pci::readpcidata(ahc, 0x14)<<32;
    cns->puts("ahci mmio=%p\n", mmio);
    //return;
    stm=(struct sata_mmio*)mmio;
    unsigned int* global_hba_control=(unsigned int*)(mmio+4);
    *global_hba_control|=1;
    *global_hba_control|=1<<31;
    for(int i=0;i<32;i++){ 
      unsigned int pxssts=stm->portc[i].pxssts;
      if(pxssts&0xf){
        cns->puts("Port %d present detection=%d\n", i, pxssts&0xf);
        portreset(i);
        unsigned int pxcmd=stm->portc[i].pxcmd;
        pxcmd&=~(0xf<<28);
        pxcmd|=(1<<28);
        stm->portc[i].pxcmd=pxcmd;
      }
    }
  }
};
