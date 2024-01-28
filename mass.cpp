#include "xhci.h"
namespace massd{
};
using namespace xhci;
using namespace massd;
void mass::init(unsigned char s){
  slot=s;
  initialized=1;
  cns->puts("mass strage class\n");
  initphase=0;
  cns->puts("sub class=%d protocol=%x\n", id.binterfacesubclass, id.binterfaceprotocol);
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
  cns->puts("bulkin=%d\n", bulkin);
}
void mass::comp(struct transfertrb* t){
}
