#include "kernel.h"
fat::fat(){
}
void fat::init(drive* drv){
  dv=drv;
  bpb=(struct BPB*)searchmem(512);
  dv->read((unsigned char*)bpb, 1, 0);
  rc=bpb->root_cluster;
  fats=(unsigned int*)searchmem(bpb->fat_size_32*512);
  ff=(unsigned char*)searchmem(bpb->fat_size_32);
}
fat::~fat(){
  freemem((unsigned long long)bpb);
  freemem((unsigned long long)fats);
  freemem((unsigned long long)ff);
}
int fat::calcclus(int c){
  return (c-2)*bpb->sectors_per_cluster+bpb->reserved_sector_count+bpb->fat_size_32*bpb->num_fats;
}
void fat::readclus(unsigned char* buf, int cnt, int clus){
  for(int i=0;i<cnt;i++){
    dv->read(&buf[i*512*bpb->sectors_per_cluster], bpb->sectors_per_cluster, calcclus(clus+i));
  }
}
int fat::readfat(int ind){
  if(ff[ind/0x80]==0){
    dv->read((unsigned char*)&fats[ind&~0x7f], 1, bpb->reserved_sector_count+(ind/0x80));
  }
  return fats[ind];
}
int fat::getchainsize(int clus){
  int size=bpb->sectors_per_cluster*512;
  while(1){
    if(readfat(clus)>=0xffffff8){
      return size;
    }
    size+=bpb->sectors_per_cluster*512;
    clus=readfat(clus);
  }
}
void fat::readcluschain(unsigned char* buf, int clus){
  while(1){
    readclus(buf, 1, clus);
    if(readfat(clus)>=0xffffff8)return;
    buf+=bpb->sectors_per_cluster*512;
    clus=readfat(clus);
  }
}
struct fat_ent* fat::getintdir(int clus){
  struct fat_ent* d=(struct fat_ent*)searchmem(getchainsize(clus));
  readcluschain((unsigned char*)d, clus);
  return d;
}
