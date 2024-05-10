CXXFLAGS = --target=x86_64-elf -fno-exceptions -ffreestanding -fno-rtti -mno-red-zone -mcmodel=large -std=c++17
LDFLAGS  += --entry Kernelmain -z norelro  --static
aLDFLAGS  += --entry Kernelmain -z norelro --image-base 0x1000000 --static
EBD=~/edk2/Build
BLD=$(EBD)/LoadPkg/DEBUG_GCC5/X64/bootloader.efi
objs= asm2.obj kernel.obj asm.obj x64.obj memory.obj layer.obj graphic.obj pci.obj console.obj xhci.obj pic.obj fifo.obj hid.obj ps2.obj window.obj timer.obj mass.obj mtask.obj terminal.obj acpi.obj drive.obj fs.obj api.obj sata.obj ide.obj rtc.obj textbox.obj button.obj pcnet.obj ip.obj udp.obj

objs2=load.obj asm.obj memory.obj x64.obj
all: 
	rm -f haribote.sys
	make haribote.img
haribote.img: kernel.o $(BLD)
	rm -f haribote.img
	qemu-img create -f raw haribote.img 200M
	mkfs.fat -n 'HARIBOTEOS' -s 2 -f 2 -R 32 -F 32 haribote.img
	sudo mkdir -p img
	sudo mount haribote.img img
	echo HIRO IS GOOD > test.txt
	sudo cp test.txt img/
	rm -f test.txt
	sudo mkdir -p img/efi/boot
	sudo touch img/skiplog
	sudo echo USER > root
	sudo mv root img/
	sudo mkdir -p img/info
	sudo mkdir -p img/passwords
	sudo mkdir -p img/test1/test2/test3
	sudo cp $(BLD) img/efi/boot/bootx64.efi
	sudo cp kernel.o img/haribote.sys
	sudo umount img
$(BLD): edk/l.c
	build
mu: mu.cpp
	gcc $< -o $@
load.elf: $(objs2)
	~/clang+llvm-7.1.0-x86_64-linux-gnu-ubuntu-14.04/bin/ld.lld $(LDFLAGS) -o $@.o $+ -lc  --image-base 0x1000000
	objcopy --output-target elf64-x86-64 $@.o $@
test.o: $(objs)
	clang+llvm-7.1.0-x86_64-linux-gnu-ubuntu-14.04/bin/ld.lld $(LDFLAGS) -o $@.o $+ -lc -lc++ -lc++abi --image-base 0xffff800000000000
	objcopy --output-target elf64-x86-64 $@.o $@
kernel.o: $(objs)
	~/clang+llvm-7.1.0-x86_64-linux-gnu-ubuntu-14.04/bin/ld.lld $(LDFLAGS) -o $@.o $+ -lc -lc++ -lc++abi --image-base 0x100000
	objcopy --output-target elf64-x86-64 $@.o $@
%.app: %.obj apps/aapi.obj
	ld.lld --entry main -z norelro --image-base 0xffff800000000000 --static -o $@ $+
%.obj: %.cpp Makefile *.h
	clang++ $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

%.obj: %.c Makefile
	clang $(CPPFLAGS) $(CFLAGS) -c $< -o $@
%.obj: %.asm Makefile
	nasm -f elf64 -o $@ $<
%.nobj: %.cpp Makefile kernel.h
	clang++-10 $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $< -O0
run: 
	./build_apps.sh
	$(HOME)/osbook/devenv/run_image.sh ./haribote.img
clean:
	rm -f *.obj
	rm -f *.nobj
	rm -f mu
	rm -f *.o
	rm -f *.elf
	rm -f haribote.img
clean_most:
	make clean
	rm $(EBD)/LoadPkg -rf
