#include "kernel.h"
fat::fat(){
}
void fat::init(drive* drv){
  dv=drv;
  bpb=(struct BPB*)searchmem(512);
  dv->read((unsigned char*)bpb, 1, 0);
  unsigned char* buf=(unsigned char*)bpb;
  for(int i=0;i<32;i++){
    for(int j=0;j<16;j++){
      cns->puts("%02x ", buf[i*16+j]);
    }
    cns->nline();
  }
  rc=bpb->root_cluster;
  fats=(unsigned int*)searchmem(bpb->fat_size_32*512);
  
}
