#include "kernel.h"
namespace timerd{
  void init(){
    io_out8(0x43, 0x34);
    io_out8(0x40, 0x9c);
    io_out8(0x40, 0x2e);
  }
};
