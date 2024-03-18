#include "kernel.h"
void fat::init(drive* drv){
  asm("cli");
  d=drv;
  buf=(unsigned char*)searchmem(2*1024*1024);
  bpb=(struct BPB*)buf;
  d->read(buf, 1, 0);
  fats=(unsigned int*)&buf[bpb->reserved_sector_count*0x200];
  ff=(unsigned char*)searchmem(bpb->fat_size_32);
  d->read((unsigned char*)fats, 1, bpb->reserved_sector_count);
}
unsigned int fat::getfat(unsigned int ind){
  if(ind==0)ind=bpb->root_cluster;
  if(!ff[ind/0x80]){
    d->read((unsigned char*)&fats[ind&~0x7f], 1, ind/0x80+bpb->reserved_sector_count);
    ff[ind/0x80]=1;
  }
  return fats[ind];
}
unsigned int fat::calcblock(unsigned int clus){
  if(clus==0)clus=bpb->root_cluster;
  unsigned int block=bpb->reserved_sector_count+bpb->num_fats*bpb->fat_size_32+(clus-2)*bpb->sectors_per_cluster;
  return block;
}
unsigned char* fat::getclusaddr(unsigned int clus){
  if(clus==0)clus=bpb->root_cluster;
  return &buf[calcblock(clus)*512];
}
void fat::preparecluschain(unsigned int clus){
  if(clus==0)clus=bpb->root_cluster;
  if(clus<2)return;
  while(1){
    int lba=calcblock(clus);
    for(int i=0;i<bpb->sectors_per_cluster;i++){
      d->read(&buf[(lba+i)*512], 1, lba+i);
    }
    if(getfat(clus)>=0xffffff8)return;
    clus=getfat(clus);
  }
}
struct fat_ent* fat::findfile(const char* n, int dir){
  if(dir==0)dir=bpb->root_cluster;
  if(n[1]==':'){
    char drv=n[0];
    if(drv>=0x60)drv-=0x40;
    return drvd::drvs[drv]->files->findfile((const char*)&n[3]);
  }
  char* nb=(char*)searchmem(strlen(n));
  strcpy(nb, n);
  int md=0;
  if(nb[strlen(n)-1]=='/')nb[strlen(n)-1]=0;
  for(int i=0;i<strlen(n);i++){
    if(nb[i]=='/'){
      md++;
      nb[i]=0;
    }
  }
  int sp=0;
  for(int i=0;i<md;i++){
    struct fat_ent* f=findfile((const char*)&nb[sp], dir); 
    if(f==0){
      freemem((unsigned long long)nb);
      return 0;
    }
    dir=f->getclus();
    sp+=strlen((const char*)&nb[sp])+1;
  }
  if(md){
    strcpy(nb, &nb[sp]);
  }
  preparecluschain(dir);
  //cns->puts("final name=%s\n", nb);
  struct fat_ent* de=(struct fat_ent*)getclusaddr(dir);
  for(int i=0;de[i].name[0]!=0;i++){
    if(de[i].attr==0x0f){
      struct fat_lent* l=(struct fat_lent*)&de[i];
      char en[256];
      fatd::makename(l, (char*)en);
      if(!strcmp((const char*)nb, en)){
        freemem((unsigned long long)nb);
        return (struct fat_ent*)&l[l->ord&0x1f];
      }
    }
  }
  freemem((unsigned long long)nb);
  return 0;
}
namespace fatd{
  void makename(struct fat_lent* l, char* n){
    for(int i=0;i<(l->ord&0x1f);i++){
      int sp=l[i].ord&0x1f;
      sp--;
      for(int j=0;j<5;j++)
        n[sp*13+j]=l[i].name[j*2];
      for(int j=0;j<6;j++)
        n[sp*13+j+5]=l[i].name2[j*2];
      for(int j=0;j<2;j++)
        n[sp*13+11+j]=l[i].name3[j*2];
    }
  }
};
namespace fsd{
  void init(){
  }
  void recognizefs(unsigned char d){
    drive* drv=drvd::drvs[d];
    unsigned char* fsb=(unsigned char*)searchmem(512);
    drv->read(fsb, 1, 0);
    if(!strncmp((const char*)&fsb[0x52], "FAT32", 5)){
      drv->files=new fat;
      fat* ft=(fat*)drv->files;
      drv->files->dl=d;
      drv->files->init(drv);
      //cns->puts("bpc=%x\n", ft->bpb->sectors_per_cluster*512);
    }else{
      //cns->puts("FS not recognized\n");
    }
    freemem((unsigned long long)fsb);
    //cns->puts("rc=%d mtas=%p\n", drv->files->rc, mtaskd::current);
    mtaskd::current->cd=drv->files->rc;
  }
};
