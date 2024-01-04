struct elf_fh{
	unsigned char ident[16];
	unsigned short type;
	unsigned short machine;
	unsigned int version;
	unsigned long long entry;
	unsigned long long phoff;
	unsigned long long shoff;
	unsigned int flags;
	unsigned short ehsize;
	unsigned short phentsize;
	unsigned short phnum;
	unsigned short shentsize;
	unsigned short shnum;
	unsigned short shstrndx;
};
struct elf_ph{
	unsigned int type;
	unsigned int flags;
	unsigned long long offset;
	unsigned long long vaddr;
	unsigned long long paddr;
	unsigned long long filesz;
	unsigned long long memsz;
	unsigned long long align;
};
