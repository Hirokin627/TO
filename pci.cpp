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
  void init(){
    pcis=(struct pcid*)searchmem(sizeof(struct pcid)*256);
    many=0;
    for(unsigned char b=0;b<8;b++){
      for(unsigned char d=0;d<32;d++){
        if((readpcidata(b, d, 0, 0)&0xffff)==0xffff){
          continue;
        }
        for(unsigned char f=0;f<8;f++){
          if((readpcidata(b, d, f, 0)&0xffff)==0xffff){
            continue;
          }
          pcis[many].bus=b;
          pcis[many].device=d;
          pcis[many].function=f;
          many++;
        }
      }
    }
    cns->puts("%d device found\n", many);
    for(int i=0;i<many;i++){
      cns->puts("%d %d: classcode=%0lx\n", pcis[i].device, pcis[i].function, readpcidata(pcis[i], 8)>>8);
    }
  }
};
