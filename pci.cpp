#include "kernel.h"
namespace pci{
  unsigned int many;
  struct pcid* pcis;
  uint32_t MakeAddress(uint8_t bus, uint8_t device,
                       uint8_t function, uint8_t reg_addr) {
    auto shl = [](uint32_t x, unsigned int bits) {
        return x << bits;
    };

    return shl(1, 31)  // enable bit
        | shl(bus, 16)
        | shl(device, 11)
        | shl(function, 8)
        | (reg_addr & 0xfcu);
  }
  unsigned int readpcidata(unsigned char buf, unsigned char device, unsigned char function, unsigned char offset){
    io_out32(0xcf8, MakeAddress(buf, device, function, offset));
    return io_in32(0xcfc);
  }
  unsigned int readpcidata(struct pcid d, unsigned char offset){
    return readpcidata(d.bus, d.device, d.function, offset);
  }
  void writepcidata(struct pcid d, unsigned char offset, unsigned int data){
    io_out32(0xcf8, MakeAddress(d.bus, d.device, d.function, offset));
    io_out32(0xcfc, data);
  }
  unsigned int getmmiosize(struct pcid d, unsigned int n){
          writepcidata(d, 4, readpcidata(d, 0x4)&~3);
    unsigned int mmio=readpcidata(d, 4*n+0x10);
    writepcidata(d, 4*n+0x10, 0xffffffff);
    unsigned int size=~(readpcidata(d, 4*n+0x10)&~0xf)+1;
    writepcidata(d, 4*n+0x10, mmio);
      pci::writepcidata(d, 4, pci::readpcidata(d, 4)|3);
    return size;
  }
  void init(){
    pcis=(struct pcid*)searchmem(sizeof(struct pcid)*256);
    many=0;
    for(unsigned char b=0;b<8;b++){
      for(unsigned char d=0;d<32;d++){
        if((readpcidata(b, d, 0, 0)&0xffff)==0xffff){
          continue;
        }
        for(unsigned char f=0;f<8;f++){
          unsigned int vi=readpcidata(b, d, f, 0)&0xffff;
          if((vi)==0xffff){
            continue;
          }
          unsigned int cc=readpcidata(b, d, f, 8)>>8;
          cc&=0xffffff;
          if(((cc>>16)&0xff)==0x06)continue;
          pcis[many].bus=b;
          pcis[many].device=d;
          pcis[many].function=f;
          for(int i=0;i<6;i++){
          }
          if(cc==0x30000){
            for(int i=0;i<6;i++){
              //cns->puts("%p (size%p)\n", readpcidata(pcis[many], 4*i+0x10)&~0xf, getmmiosize(pcis[many], i));
            }
          }
          //writepcidata(pcis[many], 4, readpcidata(pcis[many], 0x4)&~3);
          many++;
        }
      }
    }
    //cns->puts("%d device found\n", many);
    for(int i=0;i<many;i++){
      //cns->puts("%d %d %d: classcode=%0lx\n", pcis[i].bus, pcis[i].device, pcis[i].function, readpcidata(pcis[i], 8)>>8);
    }
  }
};
