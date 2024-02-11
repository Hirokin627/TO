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
    ff[ind/0x80]=1;
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
struct fat_ent* fat::search_intent(const char* name, int dir){
  unsigned int r=rflags();
  asm("cli");
  bool canuse11=strlen(name)<=12;
  struct fat_ent* d=getintdir(dir);
  for(int i=0;d[i].name[0]!=0;i++){
    if(d[i].attr==0x0f){
    }else if(d[i].attr!=8&&canuse11){
      char n[13];
      for(int j=0;j<13;j++)n[j]=0;
      int j=0;
      for(j=0;d[i].name[j]!=' '&&j<8;j++){
        n[j]=d[i].name[j];
      }
      if(d[i].name[8]!=' '){ //拡張子がある
        n[j]='.';
        j++;
        for(int k=0;k<3&&d[i].name[k+8]!=' ';k++){
          n[j+k]=d[i].name[k+8];
        }
      }
      char n11[13];
      strcpy(n11, name);
      for(int j=0;n11[j]!=0;j++){
        if(n11[j]>='a'){
          n11[j]-=0x20;
        }
      }
      if(!strcmp((const char*)n, (const char*)n11)){
        struct fat_ent* e=new struct fat_ent;
        *e=d[i];
        freemem((unsigned long long)d);
        return e;
      }
    }
  }
  freemem((unsigned long long)d);
  srflags(r);
  return 0;
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
file* fat::getf(const char* n, int dir){
  struct fat_ent* fe=search_intent(n, dir);
  if(fe==0)return 0;
  file* f=new file;
  int size=fe->filesize;
  f->ptr=(char*)searchmem((size+1023)/1024);
  f->base=f->ptr;
  f->cnt=f->size;
  strcpy(f->name, n);
  readcluschain((unsigned char*)f->ptr, (fe->clus_h<<16)|fe->clus_l);
  delete fe;
  return f;
  
}
namespace fsd{
  void init(){
  }
  void recognizefs(unsigned char d){
    drive* drv=drvd::drvs[d];
    unsigned char* fs=(unsigned char*)searchmem(drv->bpb);
    drv->read(fs, 1, 0);
    freemem((unsigned long long)fs);
  }
};
