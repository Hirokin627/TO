
extern "C" {
  void cons_putc(char c);
  unsigned long long malloc(unsigned long long size);
  void cons_puts(const char* s);
  unsigned long long asmgetwin(int xsize, int ysize);
};
class system{
  public:
    void putc(char c){
      cons_putc(c);
    }
    void puts(const char* str){
      cons_puts(str);
    }
    unsigned long long getwin(int xsize, int ysize){
      return asmgetwin(xsize, ysize);
    }
};
typedef unsigned long size_t;
void* operator new(size_t size){
  return (void*)malloc(size);
}

