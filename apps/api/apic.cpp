#include "api.h"
typedef unsigned long size_t;
void* operator new(size_t size){
  return (void*)malloc(size);
}
void operator delete(void* base){
  free((unsigned long long)base);
}
extern "C"{
  unsigned long long sbrk(size_t size){
    return malloc((unsigned long long)size);
  }
};
