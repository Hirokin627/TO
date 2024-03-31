#include "kernel.h"
fs::fs(){
}
fs::~fs(){
}
fat::fat(){
}
void fat::init(drive* drv){
  asm("cli");
  d=drv;
  buf=(unsigned char*)searchmem(2*1024*1024);
  bpb=(struct BPB*)buf;
  d->read(buf, 1, 0);
  fats=(unsigned int*)&buf[bpb->reserved_sector_count*bpb->bytes_per_sector];
  ff=(unsigned char*)searchmem(bpb->fat_size_32);
  d->read((unsigned char*)fats, 1, bpb->reserved_sector_count);
}
fat::~fat(){
  freemem((unsigned long long)buf);
}
unsigned int fat::getfat(unsigned int ind){
  if(ind==0)ind=bpb->root_cluster;
  if(!ff[ind/(bpb->bytes_per_sector/4)]){
    d->read((unsigned char*)&fats[ind&~0x7f], 1, ind/(bpb->bytes_per_sector/4)+bpb->reserved_sector_count);
    ff[ind/(bpb->bytes_per_sector/4)]=1;
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
  return &buf[calcblock(clus)*bpb->bytes_per_sector];
}
void fat::preparecluschain(unsigned int clus){
  if(clus==0)clus=bpb->root_cluster;
  if(clus<2)return;
  while(1){
    int lba=calcblock(clus);
    for(int i=0;i<bpb->sectors_per_cluster;i++){
      d->read(&buf[(lba+i)*bpb->bytes_per_sector], bpb->bytes_per_sector/512, lba+i);
    }
    if(getfat(clus)>=0xffffff8)return;
    clus=getfat(clus);
  }
}
struct fat_ent* fat::findfile(const char* n, int dir){
  if(dir==0)dir=bpb->root_cluster;
  if(n[1]==':'){
    char drv=n[0];
    if(drv>=0x60)drv-=0x20;
    //cns->puts("drv=%c\n", drv);
  /*  return 0;
  asm("sti");*/
    if(drvd::drvs[drv])return drvd::drvs[drv]->files->findfile((const char*)&n[3]);
    else return 0;
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
  int epc=bpb->bytes_per_sector*bpb->sectors_per_cluster/sizeof(struct fat_ent);
  //cns->puts("epc=%x\n", epc);
  for(int i=0;de[i].name[0]!=0;i++){
    if(de[i].attr==0x0f){
      struct fat_lent* l=(struct fat_lent*)&de[i];
      char en[256];
      fatd::makename(l, (char*)en);
      if(!strcmp((const char*)nb, en)){
        freemem((unsigned long long)nb);
        //cns->puts("pnt=%x\n", i);
        return (struct fat_ent*)&l[l->ord&0x1f];
      }
      i+=l->ord&0x1f;
    }else if(!(de[i].attr&8)){
      char name82[13];
      name82[11]=0;
      strncpy((char*)name82, (const char*)de[i].name, 11);
      for(int j=0;j<11;j++){
        if(name82[j]==' ')name82[j]=0;
      }
      if(name82[8]!=0){
        char ext[4];
        strncpy(ext, (const char*)&name82[8], 3);
        strncpy(&name82[9], ext, 3);
        name82[8]='.';
        strcpy(&name82[strlen((const char*)name82)], &name82[8]);
      }
      for(int j=0;j<11;j++){
        if(name82[j]<'a'&&name82[j]!='.'&&name82[j]!=0){
          name82[j]+=0x20;
        }
      }
        
      //cns->puts("83 name=%s\n", name82);
    }
    if(i+1>=epc){
      if(getfat(dir)>=0xffffff8){
        break;
      }else{
        de=(struct fat_ent*)getclusaddr(getfat(dir));
        i=-1;
      }
    }
  }
  freemem((unsigned long long)nb);
  return 0;
}
unsigned int fat::getchainsize(int c){
  unsigned long long size=0;
  int bpc=bpb->bytes_per_sector*bpb->sectors_per_cluster;
  if(getfat(c))size+=bpc;
  else return 0;
  while(1){
    if(getfat(c)>=0xffffff8){
      return size;
    }
    c=getfat(c);
    size+=bpc;
  }
}
unsigned int fat::allocfat(){
  for(int i=0;i<bpb->bytes_per_sector*bpb->fat_size_32/4;i++){
    int d=getfat(i);
    if(d==0){
      writefat(i, 0xffffff8);
      return i;
    }
  }
  return -1;
}
void fat::writecluschain(unsigned char* db, int clus, int size){
  int cc=clus;
  while(1){
    if(getfat(cc)==0)break;
    unsigned int d=getfat(cc);
    writefat(cc, 0);
    if(d>=0xffffff8)break;
    cc=d;
  }
  int bpc=bpb->sectors_per_cluster*bpb->bytes_per_sector;
  int uvb=0;
  if(db==0){
    db=getclusaddr(clus);
    uvb=1;
  }
  while(1){
    for(int i=0;i<bpb->sectors_per_cluster;i++){
      d->write(&db[bpb->bytes_per_sector*i], bpb->bytes_per_sector/512, calcblock(clus)+i);
    }
    writefat(clus, 0xffffff8);
    if(size<=bpc){
      break;
    }else{
      int nc=allocfat();
      writefat(nc, 0xffffff8);
      writefat(clus, nc);
      /*if(clus!=nc)cns->puts("different %d %d\n", clus, nc);
      else cns->puts("same %d\n", clus);*/
      clus=nc;
      size-=bpc;
      //cns->puts("size=%x next=%x\n", size, clus);
      if(uvb)db=getclusaddr(clus);
      else db+=bpc;
    }
  }
}
void fat::writefat(int ind, unsigned int dt){
  getfat(ind);
  fats[ind]=dt;
  int eps=d->bpb/4;
  d->write((unsigned char*)&fats[ind&~(eps-1)], 1, bpb->reserved_sector_count+(ind/eps));
  d->write((unsigned char*)&fats[ind&~(eps-1)], 1, bpb->reserved_sector_count+(ind/eps)+bpb->fat_size_32);
}
struct fat_ent* fat::getnext(struct fat_ent* e, int dir){
  unsigned long long base=(unsigned long long)getclusaddr(dir);
  unsigned long long boff=(unsigned long long)e-base;
  if(boff>=bpb->bytes_per_sector*bpb->sectors_per_cluster){
    return (struct fat_ent*)getclusaddr(getfat(dir));
  }else{
    return &e[1];
  }
}
unsigned char calcsum(struct fat_ent* e){
  unsigned char sum=0;
  for(int i=0;i<11;i++){
    sum=(sum>>1)+(sum<<7)+e->name[i];
  }
  return sum;
}
void fat::generatee(struct fat_ent* e, const char* n, int dir){
  int me=(strlen(n)+25)/13;
  struct fat_ent temp;
  int ss=strlen(n)+1;
  strcpy((char*)temp.name, "F          ");
  //cns->puts("me=%d e=%p base=%p\n", me, e, getclusaddr(dir));
  for(int i=0;i<me-1;i++){ 
    struct fat_lent* l=(struct fat_lent*)e;
    l->attr=0x0f;
    l->checksum=calcsum(&temp);
    int sp=me-2-i;
    l->ord=sp+1;
    if(l->ord==1)l->ord+=0x40;
    for(int j=0;j<5;j++){
      if(sp*13+j<ss)l->name[j*2]=n[sp*13+j];
      else *(unsigned short*)&l->name[j*2]=-1;
    }
    for(int j=0;j<6;j++){
      if(sp*13+j+5<ss)l->name2[j*2]=n[sp*13+j+5];
      else *(unsigned short*)&l->name2[j*2]=-1;
    }
    for(int j=0;j<2;j++){
      if(sp*13+j+11<ss)l->name3[j*2]=n[sp*13+j+11];
      else *(unsigned short*)&l->name3[j*2]=-1;
    }
    e=getnext(e, dir);
  }
  strcpy((char*)e->name, "F          ");
  e->filesize=0;
  e->clus_l=e->clus_h=0;
  e->attr=0x20;
  writecluschain(0, dir, getchainsize(dir));
}
struct fat_ent* fat::createe(const char* n, int dir){
  if(dir==0)dir=bpb->root_cluster;
  if(n[1]==':'){
    char drv=n[0];
    if(drv>=0x60)drv-=0x40;
    if(drvd::drvs[drv])return drvd::drvs[drv]->files->findfile((const char*)&n[3]);
    else return 0;
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
  int b=0,c=0;
  int me=(strlen(nb)+25)/13;
  
  struct fat_ent* de=(struct fat_ent*)getclusaddr(dir);
  int epc=bpb->bytes_per_sector*bpb->sectors_per_cluster/sizeof(struct fat_ent);
retry:
  int i=0;
  while(1){
    if(de[i].name[0]==0xe5){
      c++;
      if(c==1)b=i;
      if(c==me){
        generatee(&de[b], (const char*)nb, dir);
        return &de[b];
      }
    }else if(de[i].name[0]!=0){
      c=0;
    }
    if(de[i].name[0]==0){
      if(epc<=i+1){
        cns->puts("border thecluster\n");
        return 0;
      }else{
        //cns->puts("name=%s\n", nb);
        generatee(&de[i], (const char*)nb, dir);
        return &de[i];
      }
    }
    if(epc<=i+1){
      if(getfat(dir)>=0xffffff8){
        break;
      }else{
        dir=getfat(dir);
        de=(struct fat_ent*)getclusaddr(dir);
        i=0;
        continue;
      }
    }
    i++;
  }
  return 0;
}
void fat::writef(struct fat_ent* f, unsigned char* buf, int size, int dir){
  if(dir==0){
    dir=bpb->root_cluster;
  }
  if(f->attr==0x0f){
    while(f->attr==0x0f)f++;
  }
  struct fat_ent* bd=(struct fat_ent*)getclusaddr(dir);
  f->filesize=size;
  if(f->getclus()==0){
    unsigned int c=allocfat();
    f->clus_l=c&0xffff;
    f->clus_h=c>>16;
  }
  writecluschain(0, dir, getchainsize(dir));
  writecluschain(buf, f->getclus(), size);
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
    cns->puts("first b:%02x 446=%x\n", fsb[0], fsb[446]);
    if(fsb[446]==0x80||fsb[450]==0x0c){
      drv->pbase=*(unsigned int*)&fsb[446+8];
      drv->read(fsb, 1, 0);
      cns->puts("part found base=%x 80b:%x\n", drv->pbase, fsb[0]);
    }
    if(!strncmp((const char*)&fsb[0x52], "FAT32", 5)){
      drv->files=new fat;
      fat* ft=(fat*)drv->files;
      drv->files->dl=d;
      drv->files->init(drv);
      //cns->puts("bpc=%x\n", ft->bpb->sectors_per_cluster*512);
    }else{
      drv->files=new fs;
      cns->puts("FS not recognized str=%s\n", &fsb[0x52]);
    }
    freemem((unsigned long long)fsb);
    //cns->puts("rc=%d mtas=%p\n", drv->files->rc, mtaskd::current);
    mtaskd::current->cd=drv->files->rc;
  }
  void createdir(fat* tos, struct fat_ent* toent, int todir){
    int c=toent->getclus();
    if(c==0){
      c=tos->allocfat();
      toent->clus_l=c&0xffff;
      toent->clus_h=(c>>16)&0xffff;
      tos->writefat(c, 0xffffff8);
    }
    struct fat_ent* dc=(struct fat_ent*)searchmem(tos->getchainsize(c));
    strcpy((char*)dc[0].name, ".          ");
    dc[0].attr=0x10;
    dc[0].filesize=0;
    dc[0].clus_l=c&0xffff;
    dc[0].clus_h=(c>>16)&0xffff;
    strcpy((char*)dc[1].name, "..         ");
    dc[1].attr=0x10;
    dc[1].filesize=0;
    int ud=todir;
    if(todir==tos->bpb->root_cluster)ud=0;
    dc[1].clus_l=ud&0xffff;
    dc[1].clus_h=(ud>>16)&0xffff;
    tos->writecluschain((unsigned char*)dc, c, tos->getchainsize(c));
    freemem((unsigned long long)dc);
    toent->filesize=0;
    toent->attr=0x10;
    tos->writecluschain(0, todir, tos->getchainsize(todir));
  }
  void copyfatdir(fat* froms, struct fat_ent* fent, int fdir, fat* tos, struct fat_ent* toent, int todir){
    asm("cli");
    createdir(tos, toent, todir);
    //cns->puts("fdir=%d\n", fdir);
    /*int tdc=toent->getclus();
    cns->puts("tdc=%d\n", tdc);
    int fepc=froms->bpb->sectors_per_cluster*froms->bpb->bytes_per_sector/sizeof(struct fat_ent);
    int tepc=tos->bpb->sectors_per_cluster*tos->bpb->bytes_per_sector/sizeof(struct fat_ent);
    cns->puts("fclus~%d size=%d\n", fent->getclus(), froms->getchainsize(fent->getclus()));
    if(fdir!=2)return;
    //if(fdir!=2)return;;
    froms->preparecluschain(fent->getclus());
    if(fdir!=2)return;
    struct fat_ent* fdc=(struct fat_ent*)froms->getclusaddr(fent->getclus());
    for(int i=0;fdc[i].name[0]!=0;i++){
      if(fdc[i].attr==0x0f){
        char n[256];
        struct fat_lent* l=(struct fat_lent*)&fdc[i];
        fatd::makename(l, n);
        i+=l->ord&0x1f;
        struct fat_ent* te=tos->createe((const char*)n, tdc);
        if(te->attr==0x0f)te+=te->name[0]&0x1f;
        if(fdc[i].attr==0x10){
          cns->puts("argment: %p %p %d %p %p %d\n", froms, &fdc[i],fent->getclus(), tos, te, tdc);
          //createdir(tos, te, tdc);
          copyfatdir(froms, &fdc[i],fent->getclus(), tos, te, tdc);
        }
      }else if(fdc[i].attr!=0x08){
      }
    }*/
    int tdc=toent->getclus();
    int fdc=fent->getclus();
    froms->preparecluschain(fdc);
    struct fat_ent* fd=(struct fat_ent*)froms->getclusaddr(fdc);
    for(int i=0;fd[i].name[0]!=0;i++){
      if(fd[i].attr==0x0f){
        char n[60];
        struct fat_lent* l=(struct fat_lent*)&fd[i];
        fatd::makename(l, n);
        struct fat_ent* tfe=tos->createe((const char*)n, tdc);
        if(tfe->attr==0x0f)tfe+=tfe->name[0]&0x1f;
        i+=l->ord&0x1f;
        //cns->puts("attr=%x\n", fd[i].attr);
        if(fd[i].attr==0x10){
          /*createdir(tos, tfe, tdc);
          froms->preparecluschain(fd[i].getclus());*/
          copyfatdir(froms, &fd[i], fdc, tos, tfe, tdc);
        }else{
          //cns->puts("%s filesize=%d\n", n, fd[i].filesize);
          froms->preparecluschain(fd[i].getclus());
          tos->writef(tfe, froms->getclusaddr(fd[i].getclus()), fd[i].filesize, tdc);
        }
      }
    }
  }
};
