struct fi{
	int* fb;
	int xsize;
	int ysize;
};
struct __attribute__((packed)) acpi_h {
	char sig[4];
	unsigned int length;
	unsigned char revision;
	unsigned char checksum;
	unsigned char oemid[6];
	unsigned char oem_table[8];
	unsigned int oem_revision;
	unsigned int creatorID;
	unsigned int Creator_revision;
};
typedef struct {
	EFI_MEMORY_TYPE type;
	unsigned long long physicalstart;
	unsigned long long reserv;
	unsigned long long numberofpage;
	unsigned long long attr;
} EFI_MEM;
struct RSDP{
	char sig[8];
	char iran[12];
	int length;
	unsigned long long xsdt;
}__attribute__((packed));

struct __attribute__((packed)) XSDT {
	struct acpi_h header;
	struct acpi_h* entrys[0];
};
struct FADT{
	struct acpi_h head;
	char iran2[4];
	unsigned int dsdt;
	char iran4[4];
	unsigned int smicommand;
	unsigned char acpienable;
	char iran3[11];
	unsigned int PM1a_CNT_BLK;
	unsigned int PM1b_CNT_BLK;
	unsigned int iran5;
	unsigned int pm_blk;
	unsigned char iran6[112-80];
	unsigned int flags;
} __attribute__ ((packed));
struct arg{
	struct fi Frame;
	struct RSDP* acpi;
	char* bm;
	EFI_MEM* mems;
	unsigned long long size;
	unsigned long long bsize;
	EFI_DEVICE_PATH_PROTOCOL bd;
	void* volume;
	void (*reset)(int, int, int, int);
};
