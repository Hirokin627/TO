#include "kernel.h"
namespace pcnetd{
  struct pcid pn;
  unsigned short iobase;
  bool cp=false;
  struct initst{
    unsigned short mode;
    unsigned char rlen;
    unsigned char tlen;
    unsigned char mac[6];
    unsigned char rsv[2];
    unsigned char rsv2[8];
    unsigned int raddr;
    unsigned int taddr;
  };
  unsigned int readcsr(unsigned int ind){
    io_out32(iobase+0x14, ind);
    return io_in32(iobase+0x10);
  }
  void writecsr(unsigned int ind, unsigned int data){
    io_out32(iobase+0x14, ind);
    io_out32(iobase+0x10, data);
  }
  unsigned int readbcr(unsigned int no){
    io_out32(iobase+0x14, no);
    return io_in32(iobase+0x1c);
  }
  void writebcr(unsigned int no, unsigned int data){
    io_out32(iobase+0x14, no);
    io_out32(iobase+0x1c, data);
  }
  int rx_buffer_ptr = 0;
int tx_buffer_ptr = 0;                 // pointers to transmit/receive buffers
 
int rx_buffer_count = 32;              // total number of receive buffers
int tx_buffer_count = 8;               // total number of transmit buffers
 
const int buffer_size = 1520;          // length of each packet buffer
 
const int de_size = 16;                // length of descriptor entry
 
uint8_t *rdes;                         // pointer to ring buffer of receive DEs
uint8_t *tdes;                         // pointer to ring buffer of transmit DEs
 
uint32_t rx_buffers;                   // physical address of actual receive buffers (< 4 GiB)
uint32_t tx_buffers;                   // physical address of actual transmit buffers (< 4 GiB)
 
// does the driver own the particular buffer?
uint32_t crc32_table[256];
void make_crc32_table(void) {
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t c = i;
        for (int j = 0; j < 8; j++) {
            c = (c & 1) ? (0xEDB88320 ^ (c >> 1)) : (c >> 1);
        }
        crc32_table[i] = c;
    }
}

uint32_t crc32(uint8_t *buf, size_t len) {
    uint32_t c = 0xFFFFFFFF;
    for (size_t i = 0; i < len; i++) {
        c = crc32_table[(c ^ buf[i]) & 0xFF] ^ (c >> 8);
    }
    return c ^ 0xFFFFFFFF;
}
int driverOwns(uint8_t *des, int idx)
{
    return (des[de_size * idx + 7] & 0x80) == 0;
}
 
// get the next transmit buffer index
int nextTxIdx(int cur_tx_idx)
{
    int ret = cur_tx_idx + 1;
    if(ret == tx_buffer_count)
        ret = 0;
    return ret;
}
 
// get the next receive buffer index
int nextRxIdx(int cur_rx_idx)
{
    int ret = cur_rx_idx + 1;
    if(ret == rx_buffer_count)
        ret = 0;
    return ret;
}
 
// initialize a DE
int sendPacket(void *packet, size_t len, uint8_t *dest)
{
    // the next available descriptor entry index is in tx_buffer_ptr
    if(!driverOwns(tdes, tx_buffer_ptr))
    {
        // we don't own the next buffer, this implies all the transmit
        //  buffers are full and the card hasn't sent them yet.
        // A fully functional driver would therefore add the packet to
        //  a queue somewhere, and wait for the transmit done interrupt
        //  then try again.  We simply fail and return.  You can set
        //  bit 3 of CSR0 here to encourage the card to send all buffers.
        return 0;
    }
 
    // copy the packet data to the transmit buffer.  An alternative would
    //  be to update the appropriate transmit DE to point to 'packet', but
    //  then you would need to ensure that packet is not invalidated before
    //  the card has a chance to send the data.
    memcpy((void *)(tx_buffers + tx_buffer_ptr * buffer_size), packet, len);
 
    // set the STP bit in the descriptor entry (signals this is the first
    //  frame in a split packet - we only support single frames)
    tdes[tx_buffer_ptr * de_size + 7] |= 0x2;
 
    // similarly, set the ENP bit to state this is also the end of a packet
    tdes[tx_buffer_ptr * de_size + 7] |= 0x1;
 
    // set the BCNT member to be 0xf000 OR'd with the first 12 bits of the
    //  two's complement of the length of the packet
    uint16_t bcnt = (uint16_t)(-len);
    bcnt &= 0xfff;
    bcnt |= 0xf000;
    *(uint16_t *)&tdes[tx_buffer_ptr * de_size + 4] = bcnt;
 
    // finally, flip the ownership bit back to the card
    tdes[tx_buffer_ptr * de_size + 7] |= 0x80;
 
    // update the next transmit pointer
    tx_buffer_ptr = nextTxIdx(tx_buffer_ptr);
    return 0;
}
void initDE(uint8_t *des, int idx, int is_tx)
{
    memset(&des[idx * de_size], 0, de_size);
 
    // first 4 bytes are the physical address of the actual buffer
    uint32_t buf_addr = rx_buffers;
    if(is_tx)
        buf_addr = tx_buffers;
    *(uint32_t *)&des[idx * de_size] = buf_addr + idx * buffer_size;
 
    // next 2 bytes are 0xf000 OR'd with the first 12 bits of the 2s complement of the length
    uint16_t bcnt = (uint16_t)(-buffer_size);
    bcnt &= 0x0fff;
    bcnt |= 0xf000;
    *(uint16_t *)&des[idx * de_size + 4] = bcnt;
 
    // finally, set ownership bit - transmit buffers are owned by us, receive buffers by the card
    if(!is_tx)
        des[idx * de_size + 7] = 0x80;
}
  struct ARP{
    unsigned short hard;
    unsigned short protocol;
    unsigned char hin;
    unsigned char pin;
    unsigned short operation;
    unsigned  char smac[6];
    unsigned  char sip[4];
    unsigned  char dmac[6];
    unsigned char dip[4];
  }__attribute__((packed));
  char once=0;
  unsigned char amac[6]={0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
  unsigned char mac[8];
  void sendData(void* buf, unsigned short len, unsigned short protocol){
    unsigned char* b=(unsigned char*)searchmem(len+18);
    for(int i=0;i<6;i++)b[i]=amac[i];
    for(int i=0;i<6;i++)b[6+i]=mac[i];
    *(unsigned short*)&b[12]=ipd::convertbig(&protocol, 2);
    for(int i=0;i<len;i++){
      *(unsigned char*)&b[i+14]=*(unsigned char*)((unsigned long long)buf+i);
    }
    unsigned int c=crc32((uint8_t*)b, len+14);
    *(unsigned int*)&b[len+14]=ipd::convertbig(&c, 4);
    sendPacket(b, len+18, 0);
  }
  void arprecieve(void* pbuf, unsigned short len){
    struct ARP* arp=(struct ARP*)pbuf;
    ipd::convertbig(&arp->hard, 2);
    ipd::convertbig(&arp->protocol, 2);
    ipd::convertbig(&arp->operation, 2);
    if(arp->operation==1){
      lip=(192<<24)|(168<<16)|(123);
      struct ARP* na=new struct ARP;
      na->hard=ipd::conve(1, 2);
      na->protocol=ipd::conve(0x800, 2);
      na->hin=6;
      na->pin=4;
      na->operation=ipd::conve(2, 2);
      for(int i=0;i<6;i++)na->smac[i]=mac[i];
      *(unsigned int*)&na->sip[0]=ipd::conve(lip, 4);
      for(int i=0;i<6;i++)na->dmac[i]=arp->smac[i];
      *(unsigned int*)&na->dip[0]=*(unsigned int*)&arp->sip[0];
      //sendData(na, 28, 0x806);
    }else if(arp->operation==2){
      cns->puts("abc %08x\n", *(unsigned int*)((unsigned long long)pbuf+14));
      //for(int i=0;i<6;i++)cns->puts("%02x ", *(unsigned char*)((unsigned long long)pbuf+8+i));
      cns->nline();
    }
  }
  int pcnt=0;
  void handlepacket(void* pbuf, unsigned short plen){
    unsigned char* buf=(unsigned char*)pbuf;
    //cns->puts("from: ");
    //for(int i=0;i<6;i++)cns->puts("%02x ", buf[i+6]);
    for(int i=0;i<6;i++){
      amac[i]=buf[i+6];
    }
    //cns->nline();
    ipd::convertbig(&buf[12], 2);
    unsigned short type=*(unsigned short*)&buf[12];
    //cns->puts("Ether type: %04x\n", type);
    pcnt++;
    //cns->puts("%04d",pcnt);
    vram[pcnt]=0xff0000;
    switch(type){
      case 0x800:
        //cns->puts("sending to IP\n");
        ipd::recieve(&buf[14], plen-18);
        break;
      case 0x806:
        arprecieve(&buf[14], plen-18);
        break;
      default:
        cns->puts("Othe\n");
        asm("cli\nhlt");
    }
  }
  void polling(){
    if(!cp)return;
    //if(!(readcsr(0)&(1<<9)))return;
    while(driverOwns(rdes, rx_buffer_ptr)){
      unsigned short plen=*(unsigned char*)&rdes[rx_buffer_ptr*de_size+8];
      void* pbuf=(void*)(rx_buffers+rx_buffer_ptr*buffer_size);
      handlepacket(pbuf, plen);
      rdes[rx_buffer_ptr*de_size+7]=0x80;
      rx_buffer_ptr=nextRxIdx(rx_buffer_ptr);
    }
  }
  __attribute__((interrupt))void nethandle(unsigned long long* rsp){
    polling();
    writecsr(0, readcsr(0)|0x400);
    io_out8(0x20, 0x62);
    io_out8(0xa0, 0x67);
  }
  unsigned char dhcp[244]={
    1, 1, 6, 0,
    0x39, 0x03, 0xf3, 0x26,
    0, 0, 00, 0, 
    0,0,0,0,
    0,0,0,0,
    0,0,0,0,
    0,0,0,0,
    
    
    
  };
  void init(){
    for(int i=0;i<pci::many;i++){
      unsigned int vendor=pci::readpcidata(pci::pcis[i], 8)>>8;;
      vendor&=~0xff;
      if(vendor==0x20000){
        unsigned int vi=pci::readpcidata(pci::pcis[i], 0)&0xffff;
        if(vi==0x1022||vi==0x2000||vi==0x2001){
          cp=true;
          pn=pci::pcis[i];
          cns->puts("netc found\n");
        }else{
          cns->puts("NET unsupported: vendor=%04x\n", vi);
        }
      }
    }
    if(!cp)return;
    unsigned int conf=pci::readpcidata(pn, 4);
    conf&=~0xffff;
    conf|=5;
    pci::writepcidata(pn, 4, conf);
    unsigned int ir=pci::readpcidata(pn, 0x3c);
    ir&=~0xffff;
    ir|=0x010f;
    //pci::writepcidata(pn, 0x3c, ir);
    open_irq(15);
    for(int i=0;i<5;i++){
      unsigned int bar=pci::readpcidata(pn, i*4+0x10);
      if(bar&1){
        cns->puts("io base %d: %x\n", i, bar&~3);
        iobase=bar&~3;
      }
    }
    io_in16(iobase+0x18);
    io_in16(iobase+0x14);
    io_out32(iobase+0x10, 0);
    for(int i=0;i<2;i++)*(unsigned int*)&mac[i*4]=io_in32(iobase+i*4);
    cns->puts("mac: ");
    for(int i=0;i<6;i++)cns->puts("%02x ", mac[i]);
    cns->nline();
    unsigned int csr58=readcsr(58);
    csr58&=0xff00;
    csr58|=2;
    writecsr(58, csr58);
    unsigned int bcr2=readbcr(2);
    bcr2|=2;
    writebcr(2, bcr2);
    rdes=(uint8_t*)searchmem(16*32);
    tdes=(uint8_t*)searchmem(16*8);
    cns->puts("tdes=%p rdes=%p\n", tdes, rdes);
    rx_buffers=searchmem(1520*32);
    tx_buffers=searchmem(1520*8);
    for(int i=0;i<32;i++){ 
      initDE(rdes, i, 0);
    }
    for(int i=0;i<8;i++)initDE(tdes, i, 1);
    struct initst* is=new struct initst;
    is->mode=0;
    is->rlen=5<<4;
    is->tlen=3<<4;
    for(int i=0;i<6;i++)
      is->mac[i]=mac[i];
    is->raddr=(unsigned int)((unsigned long long)rdes);
    is->taddr=(unsigned int)((unsigned long long)tdes);
    cns->puts("raddr=%p\n", is->raddr);
    //writecsr(0, readcsr(0)|(1<<6));
    writecsr(1, (unsigned int)((unsigned long long)is)&0xffff);
    writecsr(2, ((unsigned int)((unsigned long long)is)>>16)&0xffff);
    unsigned int csr3=readcsr(3);
    csr3&=~(1<<2);
    csr3&=~(1<<10);
    csr3&=~(1<<14);
    csr3&=~(1<<12);
    cns->puts("csr=%5x\n", csr3);
    
    writecsr(3, csr3);
    //writecsr(0, readcsr(0)|(3<<6));
    writecsr(0, readcsr(0)|1);
    while(!(readcsr(0)&(1<<8)));
    unsigned int csr0=readcsr(0);
    csr0&=~(1<<0);
    csr0&=~(1<<2);
    csr0|=2;
    writecsr(0, csr0);
    cns->puts("csr0=%08x\n", readcsr(0));
    unsigned char dest[4]={192, 168, 0, 1};
    unsigned char data2[8]={8, 0, 0xff, 0xf7, 0, 0, 0, 0};
    //ipd::sendIP(1, 8, data2, dest);
    unsigned char data[42+4]{
      /*0xff, 0xff, 0xff, 0xff, 0xff, 0xff, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], 0x08, 0x06, */0x00, 0x01, 0x08, 0x00, 6, 4, 0, 1, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], 192, 168, 225, 2, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 192, 168, 225, 1//, 0x36, 0x11, 0xe9, 0xb6
    };
    make_crc32_table();
    //sendData(data, 28, 0x806);
    //while(1)polling();
    //sendPacket(data, 48, 0);
    cns->puts("!\n");
  }
}
unsigned int lip=0x7b00a8c0;
