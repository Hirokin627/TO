#include "kernel.h"
namespace drvd{
  drive* drvs[256];
  unsigned char bdtype;
  unsigned char bdmainaddr;
  unsigned char bdsubaddr;
  void init(EFI_DEVICE_PATH_PROTOCOL* bdp){
    for(int i=0;i<256;i++)drvs[i]=0;
    cns->puts("Drive type=%d subtype=%d\n", bdp->Type, bdp->SubType);
    if(bdp->Type==3){
      if(bdp->SubType==5){
        bdmainaddr=*(unsigned char*)((unsigned long long)bdp+4)+1;
        bdsubaddr=*(unsigned char*)((unsigned long long)bdp+5);
        bdtype=bdp->SubType;
        cns->puts("USB type:%d %d\n", bdmainaddr, bdsubaddr);
      }else if(bdp->SubType==1){
        bdmainaddr=0;
        bdsubaddr=*(unsigned char*)((unsigned long long)bdp+5);
        bdtype=1;
        cns->puts("main=%d su=%d type=%d\n", bdmainaddr, bdsubaddr, bdtype);
      }else{
        cns->puts("Other type:%d\n", bdp->SubType);
      }
      
    }else{
      cns->puts("Not media\n");
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
    cns->puts("Drive type=%d main=%d sub=%d(%d %d %d)\n", type, mainaddr, subaddr, bdtype, bdmainaddr, bdsubaddr);
    if(type==bdtype){
      if(mainaddr==bdmainaddr){
        if(subaddr==bdsubaddr){
          bdl=dl;
        }
      }
    }
    drvs[dl]=drv;
    fsd::recognizefs(dl);
    return dl;
  }
};
void drive::createfs(){
  unsigned char* p0=(unsigned char*)searchmem(bpb);
  p0[510]=0x55;
  p0[511]=0xaa;
  struct part_ent* pe=(struct part_ent*)((unsigned long long)p0+446);
  pe->bf=0x80;
  pe->sh=0;
  pe->cysc=0x20<<6;
  pe->type=0x0c;
  pe->endh=0xfe;
  pe->ecysc=0xff;
  pe->lbaoff=0x20;
  pe->lbasize=0x64000+0x20;
  write((unsigned char*)p0, 1, 0, -1);
  pbase=0x20;
  struct BPB* mbr=(struct BPB*)searchmem(bpb);
  mbr->jump_boot[0]=0xeb;
  mbr->jump_boot[1]=0xfe;
  mbr->jump_boot[2]=0x90;
  strcpy(mbr->oem_name, "UEOSV1.0");
  mbr->bytes_per_sector=bpb;
  cns->puts("BPB=%x\n", bpb);
  mbr->sectors_per_cluster=2;
  mbr->reserved_sector_count=0x20;
  mbr->num_fats=2;
  mbr->root_entry_count=0;
  mbr->total_sectors_16=0;
  mbr->media=0xf8;
  mbr->fat_size_16=0;
  mbr->sectors_per_track=0;
  mbr->num_heads=0;
  mbr->hidden_sectors=0;
  mbr->total_sectors_32=200*1024*1024/bpb;
  mbr->fat_size_32=0x634;
  mbr->root_cluster=2;
  mbr->fs_info=1;
  mbr->drive_number=0x80;
  mbr->boot_signature=0x29;
  mbr->volume_id=0x12345678;
  strcpy(mbr->volume_label, "NO NAME    ");
  strcpy(mbr->fs_type, "FAT32   ");
  *(unsigned short*)((unsigned long long)mbr+510)=0xaa55;
  write((unsigned char*)mbr, 1, 0);
  struct fsinfo* fi=(struct fsinfo*)searchmem(bpb);
  fi->leadsig=0x41615252;
  fi->strucsig=0x61417272;
  fi->freecnt=0xffffffff;
  fi->nxtfree=0xffffffff;
  fi->trailsig=0xaa550000;
  write((unsigned char*)fi, 1, 1);
  freemem((unsigned long long)fi);
  unsigned int* fat=(unsigned int*)searchmem(0x364*bpb);
  fat[0]=0xffffff8;
  fat[1]=0xfffffff;
  for(int i=0;i<0x364;i++){
    write((unsigned char*)fat+i, 1, 0x20+i);
    if(i%9==0){
      cns->puts("%d%%\n", i/9);
    }
  }
  delete files;
  files=new class fat;
  fs* fls=drvd::drvs[bdl]->files;
  files->init(this);
  unsigned char* rd=(unsigned char*)searchmem(mbr->sectors_per_cluster*bpb);
  files->writecluschain(rd, 2, mbr->sectors_per_cluster*bpb);
  freemem((unsigned long long)rd);
  freemem((unsigned long long)mbr);
  drvd::drvs[bdl]->files->preparecluschain(0);
  struct fat_ent* bd=(struct fat_ent*)drvd::drvs[bdl]->files->getclusaddr(0);
  for(int i=0;bd[i].name[0];i++){
    if(bd[i].attr==0x0f){
      struct fat_lent* l=(struct fat_lent*)&bd[i];
      char name[256];
      fatd::makename(l, name);
      cns->puts("creating file%s\n", name);
      asm("cli");
      struct fat_ent* f=files->createe((const char*)name);
      if(f->attr==0x0f)f+=f->name[0]&0x1f;
      struct fat_ent* bf=fls->findfile((const char*)name);
      if(bf->filesize&&bf->attr!=0x10){
        //cns->puts("clus=%x\n", bf->getclus());
        fls->preparecluschain(bf->getclus());
        files->writef(f, fls->getclusaddr(bf->getclus()), bf->filesize);
      }else if(bf->attr==0x10){
        fsd::copyfatdir((class fat*)fls, bf, 2, (class fat*)files, f, 2);
      }
      i+=l->ord&0x1f;
    }else{
    }
  }
}
