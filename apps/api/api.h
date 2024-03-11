#include <cstdio>
typedef __va_list va_list;
typedef struct {
char drv;
char* ptr;
int cnt;
int size;
char* base;
int based;
char flag;
char file;
int dir;
char attr;
char name[256];
} file;
extern "C" {
  void cons_putc(char c);
  unsigned long long malloc(unsigned long long size);
  void cons_puts(const char* s);
  void free(unsigned long long addr);
  unsigned long long asmgetwin(int xsize, int ysize);
  void closef(file* f);
  file* asm_fopen(const char* name);
};
class system{
  public:
    void putc(char c){
      cons_putc(c);
    }
    void puts(const char* str, ...){
      va_list va;
      char buf[1024];
      va_start(va, str);
      vsprintf(buf, str, va);
      va_end(va);
      cons_puts((const char*)buf);
    }
    file* openf(const char* name){
      return asm_fopen(name);
    }
    unsigned long long getwin(int xsize, int ysize){
      return asmgetwin(xsize, ysize);
    }
    void exit(){
      asm("mov $4,%rax\nint $0x40");
    }
};

