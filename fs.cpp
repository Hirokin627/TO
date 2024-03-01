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
  bool canuse11=strlen(name)<=12;
  struct fat_ent* d=getintdir(dir);
  for(int i=0;d[i].name[0]!=0;i++){
    if(d[i].attr==0x0f){
      char n[256];
      struct fat_lent* ld=(struct fat_lent*)&d[i];
      int j=0;
      while(1){
        int ord=ld[j].ord&0x1f;
        ord--;
        for(int k=0;k<5;k++)
          n[ord*13+k]=ld[j].name[k*2];
        for(int k=0;k<6;k++)
          n[ord*13+5+k]=ld[j].name2[k*2];
        for(int k=0;k<2;k++)
          n[ord*13+11+k]=ld[j].name3[k*2];
        if(!strcmp((const char*)n, name)){
          struct fat_ent* e=new struct fat_ent;
          *e=d[i+(ld->ord&0x1f)];
          freemem((unsigned long long)d);
          return e;
        }
        if(ord==0){
          break;
        }
        j++;
      }
      i+=ld->ord&0x1f;
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
int getclus(struct fat_ent* fe){
  return (fe->clus_l)|(fe->clus_h<<16);
}
int fat::getdn(const char* n, int dir){
  struct fat_ent* fe=search_intent(n, dir);
  if(fe==0)return -1;
  if(fe->attr!=0x10){
    cns->puts("Invalid attr:%02x\n", fe->attr);
    return -1;
  }
  int dn=getclus(fe);
  delete fe;
  return dn;
}
dirent* fat::getd(const char* n, int dir){
  struct fat_ent* fe;
  if(strcmp(n, ".")){
    fe=search_intent(n, dir);
    if(fe==0)return 0;
  }else{
    fe=new struct fat_ent;
    fe->clus_l=dir&0xffff;
    fe->clus_h=(dir>>16)&0xffff;
  }
  struct fat_ent* dc=getintdir(getclus(fe));
  int me=0;
  for(int i=0;dc[i].name[0]!=0;i++){
    if(dc[i].attr==0x0f){
      me++;
      i+=dc[i].name[0]&0x1f;
    }else if(dc[i].attr!=0x08){
      me++;
    }
  }
  dirent* de=(dirent*)searchmem(sizeof(dirent)*(me+1));
  int dp=0;
  for(int i=0;dc[i].name[0]!=0;i++){
    if(dc[i].attr==0x0f){
      struct fat_lent* l=(struct fat_lent*)&dc[i];
      char* n=(char*)de[dp].name;
      for(int j=0;j<5;j++)
        n[j]=l->name[j*2];
      for(int j=0;j<6;j++)
        n[5+j]=l->name2[j*2];
      for(int j=0;j<2;j++)
        n[11+j]=l->name3[j*2];
      de[dp].namelen=strlen((const char*)de[dp].name);
      de[dp].reclen=sizeof(dirent);
      dp++;
    }else{               
    }
  }
  delete fe;
  freemem((unsigned long long)dc);
  return de;
}
struct filefs{
  file* f;
  fs* files;
  char name[256];
};
namespace fsd{
  struct filefs* ffs;
  void init(){
    ffs=(struct filefs*)searchmem(sizeof(struct filefs)*256);
  }
  void recognizefs(unsigned char d){
    drive* drv=drvd::drvs[d];
    unsigned char* fsb=(unsigned char*)searchmem(drv->bpb);
    drv->read(fsb, 1, 0);
    if(!strncmp((const char*)&fsb[0x52], "FAT32", 5)){
      drv->files=new fat;
      drv->files->init(drv);
    }
    freemem((unsigned long long)fsb);
  }
};
void setmustdir(unsigned char* dl, char** path, int* dn, const char* n,char ddl=bdl){
  //cns->puts("bdl=%c\n", ddl);
  int sd=0;
  fs* s=drvd::drvs[ddl]->files;
  unsigned char d=ddl;
  if(n[strlen(n)-1]=='/')*(unsigned char*)&n[strlen(n)-1]=0;
  if(n[1]==':'){
    d=n[0];  
  }
  sd=s->rc;
  //cns->puts("sd=%d\n", sd);
  char* cn=(char*)searchmem(strlen(n)+1);
  strcpy(cn, n);
  int ss=strlen(cn);
  int md=0;
  for(int i=0;i<ss;i++){
    if(cn[i]=='/'){
      cn[i]=0;
      md++;
    }
  }
  //cns->puts("md=%d\n", md);
  int p=0;
  for(int i=0;i<md;i++,p+=strlen((const char*)&cn[p])+1){
    //cns->puts("name=%s dir=%d\n", &cn[p], sd);
    int nd=s->getdn((const char*)&cn[p], sd);
    //cns->puts("nd=%d\n", nd);
    if(nd==-1){
      //cns->puts("invalid\n");
      return;
    }
    sd=nd;
  }
  //cns->puts("path=%s\n", &cn[p]);
  char* pth=(char*)searchmem(strlen((const char*)&cn[p])+1);
  strcpy(pth, (const char*)&cn[p]);
  *dl=d;
  *path=pth;
  *dn=sd;
}
file* fopen(const char* name){
  char* n;
  fs* files;
  unsigned char dl;
  int dn;
  setmustdir(&dl, &n, &dn, name);  
  //cns->puts("recieve data\ndl=%c pth=%s dir=%d\n", dl, n, dn);
  files=drvd::drvs[dl]->files;
  file* f=files->getf((const char*)n, dn);
  freemem((unsigned long long)n);
  return f;
}
void closef(file* f){
  delete f;
}
dirent* opendir(const char* name){
  char* n;
  fs* files;
  unsigned char dl;
  int dn;
  setmustdir(&dl, &n, &dn, name);
  files=drvd::drvs[dl]->files;
  dirent* de=files->getd(n, dn);
  freemem((unsigned long long)n);
  return de;
}
void closedir(dirent* d){
  freemem((unsigned long long)d);
}
