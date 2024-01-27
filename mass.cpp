#include "xhci.h"
namespace massd{
};
using namespace xhci;
void mass::init(unsigned char s){
  slot=s;
  cns->puts("mass strage class\n");
  initphase=0;
  cns->puts("sub class=%d protocol=%x\n", id.binterfacesubclass, id.binterfaceprotocol);
  controltrans(slot, 0b00100001, 0xff, 0, id.iinterface, 0, 0, 0);
}
void mass::comp(struct transfertrb* t){
  cns->puts("reset code=%d\n", t->code);
  if(initphase==0){
    maxlun=(unsigned char*)searchmem(1);
    *maxlun=1;
    controltrans(slot, 0b10100001, 0xfe, 0, id.iinterface, 1, (unsigned long long)maxlun, 1);
    initphase=1;
  }else{
    cns->puts("maxlun=%d\n", *maxlun);
  }
}
