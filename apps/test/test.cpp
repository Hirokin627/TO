int main(int argc, char** argv){
  asm("int $0x40");
  asm("f:jmp f");
  while(1);
  return 0;
}
