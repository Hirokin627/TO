#include "xhci.h"
// USBメモリドライバ
/*
 基本的な概念
 　　 用語説明
 　　       ブロック・・・USBメモリでのデータの単位。初期化時に取得される。
 　　   xHCI・・・平たく言えばUSBデバイスとの仲介役
 　　 　エンドポイント・・・USBデバイスとの論理的な通信路。通信の処理はxHCIドライバー担当。
 　　   デフォルト・コントロール・パイプ・・・どのUSBデバイスにも存在するエンドポイント。主に初期化に使用する。
 　　 　Bulk outエンドポイント　大容量の転送ができるPCからの一方通行のエンドポイント。何番かは調べなきゃわからない
 　　 　Bulk in エンドポイント　Bulk out エンドポイントの通信方向がUSBデバイスから担っただけ。
 　　 関数説明
 　　  cns->puts ・・・画面に文字を出力させる。
 　　  controltrans ・・・デフォルト・コントロール・パイプを通して通信を行う。
 　　  tr[なんとか][なんとか]->push ・・・説明が難しいので例え話でいうと、xHCIに要求の留守電を入れる。といった感じだろうか。db[slot]=ほにゃららでxHCIに通知する。通信が完了するとmass::compが呼ばれる。controltransも一緒
 　　 searchmem・・・指定したサイズのメモリーを確保し、先頭アドレスを返す。
 　　 freemem・・・指定したメモリーを開放する。
 　　 posthandle・・・xHCIドライバーに保留中の処理を実行させる。ちなみにmass::compはこの関数によって呼ばれる。
 　　USBメモリとの通信の大体の手順
 　　　1.CBWと呼ばれる要求やその情報などをまとめた構造体をBulk out エンドポイントから転送する　長さは必ず31バイトでなければならない
 　　　2.転送する必要があるデータを通信方向に応じて転送する
 　　　3.CSWと呼ばれる要求の結果などが記された構造体をBulk in エンドポイントから受けとる
 　　　　(CBWやCSWの具体的な構造はxhci.hのstruct CBW とstruct CSW参照
 　　ドライバーの挙動
 　　　1.USBメモリとの通信の「準備が終わると、init()が呼ばれる。
 　　　2.xHCIドライバから渡された情報をもとにBulk in、Bulk outの番号を特定する　情報はfulldを先頭にしている。
 　　　3.USBメモリが対応しているものだと「無理やり」仮定して、リセットを行わせる。(contorltrans(slot, 0b00100001, 0xff ...のところ）
 　　　4.目的はないけどLUN(これはなんなのかわかんない。）を取得。このドライバーではつかわない。
 　　　5.メディアの容量や１ブロックあたりのバイト数を取得。
 　参考情報
 　　　制作OS・・・Ubuntu 23(仮想マシン　ホストOS:Windows10 64Bit)
 　　　デバッグ環境と動かしたときの状況: 以下の４つ
 　　　　Virtualbox: USBデバイスを認識するが、時々エラー終了したり、USBデバイスとの通信がエラーで終了する。
 　　　　QEMU: USBデバイスを認識し、USBデバイスとの通信やその他エラーを吐かない
 　　　　VMWare: USBデバイスを認識するが、リセット以降デバイスからの応答がない
 　　　　実機(NEC製): USBデバイスを認識するようだが、どこかで応答を停止している。現在調査中。
 　以上詳しい説明は関数に記載されています。
*/
namespace massd{
};
using namespace xhci;
using namespace massd;
void mass::init(unsigned char s){
  asm("cli");
  cns->puts("MASS START\n");
  slot=s; //slot=xHCIによって割り当てられた識別番号
  initialized=1;
  initphase=0;
  unsigned char* p=fulld;
  p+=p[0];
  do {
    if(p[0]==0)break;
    if(p[1]==5){
      unsigned char ep=calcepaddr(p[2]);
      unsigned char ept=p[3]&3;
      ept+=(p[2]>>7)*4;
      if(ept==6)bulkin=ep;
      else if(ept==2)bulkout=ep;
    }
    p+=p[0];
  }while(p[1]!=4);
  outtrb=new struct normalTRB;
  outtrb->trbtransferlength=31;
  outtrb->ioc=outtrb->isp=1;
  intrb=new struct normalTRB;
  intrb->trbtransferlength=13;
  intrb->ioc=intrb->isp=1;
    struct CBW* cbw=new struct CBW;
    mycbw=cbw;
  //controltrans(slot, 0b10000000, 8, 0, 0, 1, searchmem(1), 1);
    controltrans(slot, 0b00100001, 0xff, 0, id.binterfacenumber, 0, 0, 0); //リセットを仕掛ける
}
void mass::comp(struct transfertrb* t){
  if(t){
    if(t->code==6){ //エラーが発見された
    /*freemem((unsigned long long)maxlun);
      initphase=-1;
      intrb->pointer=(unsigned long long)new struct CSW;
      intrb->trbtransferlength=13;
    tr[slot][bulkin]->push((struct TRB*)intrb);
    db[slot]=bulkin;*/
    //controltrans(slot, 0b01000001, 0xff, 0, id.binterfacenumber, 0, 0, 0);
    //controltrans(slot, 2, 1, 0, bulkin>>1, 0, 0, 0);
    //initphase=999;
    return;
    }
  }
  asm("sti");
  if(initphase==999){
  //controltrans(slot, 1, 11, 0, id.binterfacenumber, 0, 0, 0);
    cns->puts("reset comp bulkin=%d\n", dcbaa[slot]->epcont[bulkin-1].epstate);
    //asm("cli\nhlt");
  }else if(initphase==998){
    controltrans(slot, 2, 1, 0, bulkin>>1, 0, 0, 0);
    initphase=999;
  }else if(initphase==997){
    //controltrans(slot, 0b00100001, 0xff, 0, id.binterfacenumber, 0, 0, 0);
    cns->puts("bulkin state=%d\n", dcbaa[slot]->epcont[bulkin-1].epstate);
    controltrans(slot, 2, 1, 0, bulkin>>1, 0, 0, 0);
    initphase=999;
  }else if(initphase==996){
    intrb->pointer=searchmem(13);
    intrb->trbtransferlength=13;
    initphase=995;
  }else if(initphase==995){
    cns->puts("code=%d bulkstate=%d\n", t->code, dcbaa[slot]->epcont[bulkin-1].epstate);
  }else if(initphase==0){
  maxlun=(unsigned char*)searchmem(1);
    initphase=1;
    controltrans(slot, 0b10100001, 0xfe, 0, id.binterfacenumber, 1, (unsigned long long)maxlun, 1);
  }else if(initphase==1){
    struct CBW* cbw=mycbw;
    cbw->tag=1;
    cbw->transferlength=8;
    cbw->lun=0;
    cbw->flags=0x80;
    cbw->cblength=12;
    cbw->cb[0]=0x25;
    outtrb->pointer=(unsigned long long)cbw;
    initphase=2;
    tr[slot][bulkout]->push((struct TRB*)outtrb);
    db[slot]=bulkout;
  }else if(initphase==2){
    intrb->pointer=searchmem(8);
    *(unsigned long long*)intrb->pointer=-1;
    intrb->trbtransferlength=8;
    initphase=3;
    tr[slot][bulkin]->push((struct TRB*)intrb);
    db[slot]=bulkin;
  }else if(initphase==3){
    unsigned int lbasize=0;
    for(int i=0;i<4;i++){
      bpb|=*(unsigned char*)(intrb->pointer+i+4)<<((3-i)*8);
    }
    for(int i=0;i<4;i++)
      lbasize|=*(unsigned char*)(intrb->pointer+i)<<((3-i)*8);
    cns->puts("size=%x\n", lbasize);
    intrb->pointer=searchmem(sizeof(struct CSW));
    intrb->trbtransferlength=13;
    initphase=4;
    tr[slot][bulkin]->push((struct TRB*)intrb);
    db[slot]=bulkin;
  }else if(initphase==4){
    struct CSW* csw=(struct CSW*)intrb->pointer;
    if(csw->sig!=0x53425355){
    tr[slot][bulkin]->push((struct TRB*)intrb);
    db[slot]=bulkin;
    }else freemem((unsigned long long)csw);
    initphase=8;
    drive* drv=new usbdrv(slot, id.binterfacenumber);
    drv->bpb=bpb;;
    cns->puts("bpb=%x code=%d\n", bpb, csw->status);
    //unsigned char dl=drvd::registdrv(5, slots[slot].port, id.binterfacenumber, drv);
    if(bpb<0x1000){
      kernelbuf->write(6);
      kernelbuf->write(5);
      kernelbuf->write(slots[slot].port);
      kernelbuf->write(id.binterfacenumber);
      kernelbuf->write((unsigned long long)drv);
    }
  }else if(initphase==5){
    struct CBW* cbw=(struct CBW*)mycbw;
    struct normalTRB* nt=cbw->flags>>7? intrb : outtrb;
    nt->trbtransferlength=bpb;
    nt->pointer=(unsigned long long)tb;
    unsigned int ep=cbw->flags>>7 ? bulkin : bulkout;
    initphase=6;
    tr[slot][ep]->push((struct TRB*)nt);
    db[slot]=cbw->flags>>7 ? bulkin : bulkout;
  }else if(initphase==6){
    struct CSW* csw=(struct CSW*)searchmem(sizeof(struct CSW));
    intrb->pointer=(unsigned long long)csw;
    intrb->trbtransferlength=13;
    initphase=7;
    tr[slot][bulkin]->push((struct TRB*)intrb);
    db[slot]=bulkin;
  }else if(initphase==7){
    struct CSW* csw=(struct CSW*)intrb->pointer;
    if(csw->sig!=0x53425355){
      asm("cli");
      cns->puts("invalid CSW=%x\n", csw->sig);
      initphase=9;
      asm("sti");
      controltrans(slot, 0b00100001, 0xff, 0, id.binterfacenumber, 0, 0, 0);
      //tr[slot][bulkin]->push((struct TRB*)intrb);
      //db[slot]=bulkin;
    }else{
      if(csw->status!=0){
        cns->puts("Error code=%d cmd=%02x\n", csw->status, mycbw->cb[0]);
      }
      freemem((unsigned long long)csw);
      outtrb->pointer=(unsigned long long)mycbw;
      initphase=8;
    }
  }else if(initphase==9){
    initphase=10;
    controltrans(slot, 2, 1, 0, bulkin>>1, 0, 0, 0);
  }else if(initphase==10){
    initphase=11;
    controltrans(slot, 2, 1, 0, bulkout>>1, 0, 0, 0);
  }else if(initphase==11){
    outtrb->pointer=(unsigned long long)mycbw;
    outtrb->trbtransferlength=31;
    initphase=5;
    asm("cli");
    tr[slot][bulkout]->push((struct TRB*)outtrb);
    asm("sti");
    db[slot]=bulkout;
  }
}
void mass::read(unsigned char* buf, unsigned int cnt, unsigned int lba){
  //while(initphase!=8){
  //  asm("sti\nhlt");
  //}
  asm("cli");
  //srflags(r);
  initphase=5;
  tb=buf;
  struct CBW* cbw=mycbw;
  cbw->cb[0]=0xa8;
  for(int i=0;i<=0x18;i+=8){
    cbw->cb[2+i/8]=lba>>(0x18-i);
  }
  for(int i=0;i<=0x18;i+=8){
    cbw->cb[6+i/8]=cnt>>(0x18-i);
  }
  cbw->transferlength=bpb;
  cbw->cblength=12;
  cbw->flags=0x80;
  outtrb->trbtransferlength=31;
  outtrb->pointer=(unsigned long long)mycbw;
  tr[slot][bulkout]->push((struct TRB*)outtrb);
  asm("sti");
  db[slot]=bulkout;
  while(initphase!=8){
    asm("cli");
    posthandle();
    //asm("sti\nhlt");
  }
    asm("sti\nhlt");
  //for(int i=0;i<1;i++)asm("sti\nhlt");
  asm("cli");
  //srflags(r);
}
void mass::write(unsigned char* buf, unsigned int cnt, unsigned int lba){
  unsigned int r=rflags();
  //while(initphase!=8)asm("sti");
  asm("cli");
  initphase=5;
  tb=buf;
  struct CBW* cbw=mycbw;
  cbw->cb[0]=0xaa;
  for(int i=0;i<=0x18;i+=8){
    cbw->cb[2+i/8]=lba>>(0x18-i);
  }
  for(int i=0;i<=0x18;i+=8){
    cbw->cb[6+i/8]=cnt>>(0x18-i);
  }
  cbw->transferlength=bpb;
  cbw->cblength=12;
  cbw->flags=0;
  cbw->lun=0;
  outtrb->trbtransferlength=31;
  tr[slot][bulkout]->push((struct TRB*)outtrb);
  asm("sti");
  db[slot]=bulkout;
  while(initphase!=8){
    asm("cli");
    posthandle();
    //asm("sti\nhlt");
  }
    asm("sti\nhlt");
  //for(int i=0;i<1;i++)asm("sti\nhlt");
  //srflags(r);
  asm("cli");
}
usbdrv::usbdrv(unsigned char slot, unsigned char interface){
  intf=(mass*)drivers[slot][interface];
  pbase=0;
  type=5;
}
void usbdrv::read(unsigned char* buf, unsigned int cnt, unsigned int lba, unsigned int pn){
  //unsigned char tb[2048];
  asm("cli");
  unsigned char* tb=(unsigned char*)searchmem(2048);
  asm("sti");
  if(pn!=-1)lba+=pbase;
  if(intf->bpb==0){
    cns->puts("BPB ZERO ERR\n");
    asm("cli\nhlt");
  }
  for(int i=0;i<cnt;i++){
      unsigned long tlba=(lba+i)*0x200/intf->bpb;
      //cns->puts("tlba=%x\n", tlba);
      unsigned long blba=(lba+i)*0x200%intf->bpb;
      intf->read(tb, 1, tlba);
      for(int j=0;j<512;j++){
        buf[i*512+j]=tb[blba+j];
      }
  }
  asm("cli");
  freemem((unsigned long long)tb);
}
void usbdrv::write(unsigned char* buf, unsigned int cnt, unsigned int lba, unsigned int pn){
  asm("cli");
  unsigned char* tb=(unsigned char*)searchmem(2048);
  asm("sti");
  if(pn!=-1)lba+=pbase;
  for(int i=0;i<cnt;i++){
    unsigned long tlba=(lba+i)*0x200/intf->bpb;
    intf->read(tb, 1, tlba);
    unsigned long blba=(lba+i)*0x200%intf->bpb;
    for(int j=0;j<512;j++){
      tb[blba+j]=buf[i*512+j];
    }
    intf->write(tb, 1, tlba);
  }
  asm("cli");
  freemem((unsigned long long)tb);
}
