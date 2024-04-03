#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Protocol/BlockIo.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/SimpleFileSystem.h>
#include "bootpack.h"
#include "elf.hpp"
unsigned char mems[4096*4];
CHAR8 password[]={
	"Simon"
};
void halt(CHAR16* s __attribute__((unused)),EFI_STATUS E){
	//Print(s);
	Print(L"%r", E);
	//while(1);
}
int count;
EFI_INPUT_KEY abc;
EFI_GRAPHICS_OUTPUT_PROTOCOL* gop=(EFI_GRAPHICS_OUTPUT_PROTOCOL*)0;
EFI_LOADED_IMAGE_PROTOCOL* lip;
EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* sfsp;
EFI_BLOCK_IO_PROTOCOL* bip;
EFI_FILE_PROTOCOL *root, *file;
EFI_STATUS EFIAPI UefiMain(EFI_HANDLE IH, EFI_SYSTEM_TABLE* ST){
	gBS->OpenProtocol(IH, &gEfiLoadedImageProtocolGuid, (void**)&lip, IH, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
	gBS->OpenProtocol(lip->DeviceHandle, &gEfiBlockIoProtocolGuid, (void**)&bip, IH, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
	gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid, NULL, (void**)&gop);
	gBS->OpenProtocol(lip->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (void**)&sfsp, IH, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
	EFI_DEVICE_PATH_PROTOCOL* test;
	gBS->OpenProtocol(lip->DeviceHandle, &gEfiDevicePathProtocolGuid, (void**)&test, IH, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
	//test=lip->FilePath;
	sfsp->OpenVolume(sfsp, &root);
	EFI_STATUS e=root->Open(root, &file, L"haribote.sys", EFI_FILE_MODE_READ, 0);
	if(EFI_ERROR(e))Print(L"Error: Kernel load %r", e);
	UINTN fs=1024*1024;
	VOID* tk;
	gBS->AllocatePool(EfiLoaderData, 1024*1024, &tk);
	file->Read(file, &fs, (VOID*)tk);
	UINTN bsize=4096*4;
	UINTN key;
	UINTN dsize;
	UINT32 dv;
	struct elf_fh* kf=(struct elf_fh*)tk;
	UINT64 epp=*((UINT64*)((UINT64)kf+24));
	struct elf_ph* kp=(struct elf_ph*)((unsigned long long)kf+kf->phoff);
	for(int i=0;i<kf->phnum;i++){
		if(kp[i].type!=1)continue;
		//Print(L"%0lx\n", kp[i].vaddr);
		UINT64 fd=(UINT64)kf+kp[i].offset;
		char* v=(char*)kp[i].vaddr;
		for(int j=0;j<kp[i].filesz;j++){
			v[j]=*(char*)(fd+j);
		}
		EFI_PHYSICAL_ADDRESS addr=(EFI_PHYSICAL_ADDRESS)kp[i].vaddr;
		gBS->AllocatePages(AllocateAddress, EfiLoaderData, ((kp[i].vaddr&0xfff)+kp[i].memsz+4095)/4096, (EFI_PHYSICAL_ADDRESS*)&addr);
	}
	//gBS->AllocatePages(AllocateAddress, EfiLoaderData, 0x42000, 0);
	gBS->FreePool(tk);
	//UINT64 bs=(UINT64)tk;
	VOID* acpi;
	for(int i=0;i<ST->NumberOfTableEntries;i++){
		if(CompareGuid(&gEfiAcpiTableGuid, &ST->ConfigurationTable[i].VendorGuid)){
			acpi=ST->ConfigurationTable[i].VendorTable;
		}
	}
	int mb=bip->Media->LastBlock;
	UINTN s=mb*512;
	VOID* v;
	if(s>16*1024*1024)s=16*1024*1024;
	Print(L"Strage size:%d\n", s);
	EFI_STATUS sss=gBS->AllocatePool(EfiLoaderData, s, &v);
	if(EFI_ERROR(sss)){
	  Print(L"test %r\n", sss);
	}
	//Print(L"%d", (unsigned long long)mb);
	EFI_STATUS st=bip->ReadBlocks(bip, bip->Media->MediaId, 0, s, (void*)v);
	if(EFI_ERROR(st)){
		Print(L"Error d=%r", st);
		asm("cli\nhlt");
	}
	//gBS->GetMemoryMap(&bsize, (EFI_MEMORY_DESCRIPTOR*)mems, &key, &dsize, &dv);
	file->Close(file);
	root->Close(root);
	struct arg ai;
	while(test->Type!=0x7f){
	  Print(L"Device protocol id:%d %d\n", test->Type, test->SubType);
	  if(test->Type==3){
	    if(test->SubType==5){
	      Print(L"Device port : interface = %d : %d\n", *(unsigned char*)((unsigned long long)test+4), *(unsigned char*)((unsigned long long)test+5));
	      for(int j=0;j<*(unsigned short*)test->Length;j++){
	        *(unsigned char*)((unsigned long long)&ai.bd+j)=*(unsigned char*)((unsigned long long)test+j);
	      }
	    }else if(test->SubType==1){
	      Print(L"ATA Device: PS=%d MS=%d\n", *(unsigned char*)((unsigned long long)test+4), *(unsigned char*)((unsigned long long)test+5));
	      for(int j=0;j<*(unsigned short*)test->Length;j++){
	        *(unsigned char*)((unsigned long long)&ai.bd+j)=*(unsigned char*)((unsigned long long)test+j);
	      }
	    }
	  }
	  Print(L"Length=%d\n", *(unsigned short*)test->Length);
	  test=(EFI_DEVICE_PATH_PROTOCOL*)((unsigned long long)test+*(unsigned short*)test->Length);
	}
	Print(L"base %p ptr=%p\n", gRT, &gRT->ResetSystem);
	gBS->GetMemoryMap(&bsize, (EFI_MEMORY_DESCRIPTOR*)mems, &key, &dsize, &dv);
	//Print(L"memory desc addr:%0lx\nMemory desc size: %0lx\nBuffe rsize=%0lx", (UINT64)mems, (UINTN)dsize, (UINTN)bsize);
	gBS->ExitBootServices(IH, key);
	typedef void ep(struct arg*);
	ep* entry=(ep*)epp;
	ai.Frame.fb=(int*)gop->Mode->FrameBufferBase;
	ai.rtb=(unsigned long long)gRT;
	ai.Frame.xsize=gop->Mode->Info->HorizontalResolution;
	ai.Frame.ysize=gop->Mode->Info->VerticalResolution;
	ai.acpi=acpi;
	ai.mems=(EFI_MEM*)mems;
	ai.bsize=bsize;
	ai.size=dsize;
	ai.volume=v;
	//gRT->ResetSystem(EfiResetCold, 0, 0, (VOID*)0);
	entry(&ai);
	while(1);
	return 0;
}
