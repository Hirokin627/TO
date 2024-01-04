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
	/*Print(L"Enter Passphrase:");
	while(1){
		ST->ConIn->ReadKeyStroke(ST->ConIn, &abc);
		if(abc.UnicodeChar!=0){
			CHAR16 test[2];
			test[0]=abc.UnicodeChar;
			test[1]=0;
			if(test[0]>=0x30&&test[0]<=0x7e)Print((CHAR16*)test);
			if(password[count]==abc.UnicodeChar){
				count++;
				if(count==5)break;
			}else{
				for(int i=0;i<count+1;i++){
					Print(L"\b");
					Print(L" ");
					Print(L"\b");
				}
				count=0;
			}
		}
	}*/
	if(EFI_ERROR(gBS->OpenProtocol(IH, &gEfiLoadedImageProtocolGuid, (void**)&lip, IH, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL)))halt(L"t", 5);
	gBS->OpenProtocol(lip->DeviceHandle, &gEfiBlockIoProtocolGuid, (void**)&bip, IH, NULL, EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
	gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid, NULL, (void**)&gop);
	gBS->LocateProtocol(&gEfiSimpleFileSystemProtocolGuid, NULL, (void**)&sfsp);
	sfsp->OpenVolume(sfsp, &root);
	EFI_STATUS s=root->Open(root, &file, L"boot.cnf", 1, 0);
	if(EFI_ERROR(s))halt(L"file open", s);
	UINTN cfilesize=40;
	void* caddr;
	gBS->AllocatePool(EfiLoaderData, 4, (VOID**)&caddr);
	s=file->Read(file, &cfilesize, (VOID*)caddr);
	if(s!=0)halt(L"file read",s);
	int ssize=0;
	char* sr=(char*)caddr;
	for(int i=0;i<cfilesize;i++){
		if(sr[i]>='0'&&sr[i]<='9'){
			ssize*=10;
			ssize+=sr[i]-0x30;
		}
	}
	if(ssize>=2000)return 0;
	Print(L"%d\n", ssize);
	gBS->FreePool(caddr);
	file->Close(file);
	root->Close(root);
	if(gop==0)return 0;
	if(gop->Mode->FrameBufferBase==0)return 0;
	for(int i=0;i<gop->Mode->MaxMode&&ssize!=gop->Mode->Info->HorizontalResolution;i++){
		EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* info;
		UINTN infosize;
		gop->QueryMode(gop, i, &infosize, &info);
		if(info->HorizontalResolution==ssize){
			gop->SetMode(gop, i);
			break;
		}
	}
	sfsp->OpenVolume(sfsp, &root);
	if(root->Open(root, &file, L"haribote.sys", 1, 0)!=0)return 0;
	UINTN filesize=1024*1024;
	void* addr;
	gBS->AllocatePool(EfiLoaderData, 1024*1024*2, (VOID**)addr);
	s=file->Read(file, &filesize, (VOID*)addr);
	if(s!=0)halt(L"file read", s);;
	struct elf_fh *fh=(struct elf_fh*)addr;
	//if(fh->ident[1]!='E')while(1);
	unsigned short phnum=fh->phnum;
	struct elf_ph* ph=(struct elf_ph*)((unsigned long long)addr+fh->phoff);
	for(unsigned short i=0;i<phnum;i++){
		if(ph[i].type!=1)continue;
		void* a=(void*)ph[i].vaddr;
		gBS->AllocatePages(AllocateAddress, EfiLoaderData, (ph[i].vaddr+0xfff)>>12, (EFI_PHYSICAL_ADDRESS*)&a);
		Print(L"%0lx\n", ph[i].vaddr);
		for(int j=0;j<ph[i].memsz-ph[i].filesz;j++)
			*((char*)((unsigned long long)ph[i].vaddr+ph[i].filesz+j))=0;
		for(int j=0;j<ph[i].filesz;j++)
			*((char*)((unsigned long long)ph[i].vaddr+j))=*((char*)((unsigned long long)addr+ph[i].offset+j));
	}
	gBS->FreePool(addr);
	VOID* image=(VOID*)0x100000;
	EFI_LBA l=bip->Media->LastBlock;
	if(l>16*1024*1024/512)l=16*1024*1024/512;
	gBS->AllocatePool(EfiLoaderData, l*512, (VOID**)&image);
	EFI_STATUS f=bip->ReadBlocks(bip, bip->Media->MediaId, 0, l*512, image);
	if(EFI_ERROR(f))halt(L"read blocks", f);
	typedef int ep(struct arg*);
	ep* Entry=(ep*)*(unsigned long long*)(24+0x1000000);
	UINTN key;
	UINTN size=4096*4;
	struct arg ai;
	gBS->GetMemoryMap(&size, (EFI_MEMORY_DESCRIPTOR*)mems, &key, (UINTN*)&ai.size, (UINT32*)0x100000);
	gBS->ExitBootServices(IH, key);
	void* acpi=(void*)0;
	for(int i=0;i<ST->NumberOfTableEntries;i++){
		if(CompareGuid(&gEfiAcpiTableGuid, &ST->ConfigurationTable[i].VendorGuid)){
			acpi=ST->ConfigurationTable[i].VendorTable;
		}
	}
	ai.Frame.fb=(int*)gop->Mode->FrameBufferBase;
	ai.Frame.xsize=(int)gop->Mode->Info->HorizontalResolution;
	ai.Frame.ysize=(int)gop->Mode->Info->VerticalResolution;
	ai.acpi=(struct RSDP*)acpi;
	ai.bsize=(unsigned long long)size;
ai.	mems=(EFI_MEM*)mems;
	ai.volume=image;
	ai.reset=(void (*)(int, int, int, int))gRT->ResetSystem;
	*((int*)gop->Mode->FrameBufferBase)=0xff0000;
	Entry(&ai);
	if(*((char*)3)==0)
		gRT->ResetSystem(EfiResetCold, 0, 0, 0);
	else
		gRT->ResetSystem(EfiResetShutdown, 0, 0, 0);
	while(1);
	return 0;
}
