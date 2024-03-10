#include "kernel.h"
namespace ided{ 
  void init(){
    io_out8(0x3f6, 4);
    io_out8(0x3f6, 2);
  }
};
