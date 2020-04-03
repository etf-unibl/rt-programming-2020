#include <stdio.h>
#include <stdlib.h>
#include <elf.h>
#include <string.h>

void print_magic(Elf64_Ehdr*);
char* ret_class(Elf64_Ehdr*);
char* ret_data(Elf64_Ehdr*);
char* ret_ei_version(Elf64_Ehdr*);
char* ret_type(Elf64_Ehdr*);
char* ret_machine(Elf64_Ehdr*);
		
/*lista naziva preuzetra sa: https://refspecs.linuxfoundation.org/elf/gabi4+/ch4.eheader.html*/
static char* name_machine[]={
			"No machine",
			"AT&T WE 32100",
			"SUN SPARC",
			"Intel 80386",
			"Motorola m68k family",
			"Motorola m88k family",
			"Intel MCU",
			"Intel 80860",
			"MIPS R3000 big-endian",
			"IBM System/370",
			"MIPS R3000 little-endian",
			"reserved","reserved","reserved","reserved",
			"HPPA",
			"reserved",
			"Fujitsu VPP500",
			"Sun's ""v8plus""",
			"Intel 80960",
			"PowerPC",
			"PowerPC 64-bit",
			"IBM S390",
			"IBM SPU/SPC",
			"reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved",
			"NEC V800 series",
			"Fujitsu FR20",
			"TRW RH-32",
			"Motorola RCE",
			"ARM",
			"Digital Alpha",
			"Hitachi SH",
			"SPARC v9 64-bit",
			"Siemens Tricore",
			"Argonaut RISC Core",
			"Hitachi H8/300",
			"Hitachi H8/300H",
			"Hitachi H8S",
			"Hitachi H8/500",
			"Intel Merced",
			"Stanford MIPS-X",
			"Motorola Coldfire",
			"Motorola M68HC12",
			"Fujitsu MMA Multimedia Accelerator",
			"Siemens PCP",
			"Sony nCPU embeeded RISC",
			"Denso NDR1 microprocessor",
			"Motorola Start*Core processor",
			"Toyota ME16 processor",
			"STMicroelectronic ST100 processor",
			"Advanced Logic Corp. Tinyj emb.fam",
			"AMD x86-64 architecture",
			"Sony DSP Processor",
			"Digital PDP-10",
			"Digital PDP-11",
			"Siemens FX66 microcontroller",
			"STMicroelectronics ST9+ 8/16 mc",
			"STmicroelectronics ST7 8 bit mc",
			"Motorola MC68HC16 microcontroller",
			"Motorola MC68HC11 microcontroller",
			"Motorola MC68HC08 microcontroller",
			"Motorola MC68HC05 microcontroller",
			"Silicon Graphics SVx",
			"STMicroelectronics ST19 8 bit mc",
			"Digital VAX",
			"Axis Communications 32-bit emb.proc",
			"Infineon Technologies 32-bit emb.proc",
			"Element 14 64-bit DSP Processor",
			"LSI Logic 16-bit DSP Processor",
			"Donald Knuth's educational 64-bit proc",
			"Harvard University machine-independent object files",
			"SiTera Prism",
			"Atmel AVR 8-bit microcontroller",
			"Fujitsu FR30",
			"Mitsubishi D10V",
			"Mitsubishi D30V",
			"NEC v850",
			"Mitsubishi M32R",
			"Matsushita MN10300",
			"Matsushita MN10200",
			"picoJava",
			"OpenRISC 32-bit embedded processor",
			"ARC International ARCompact",
			"Tensilica Xtensa Architecture",
			"Alphamosaic VideoCore",
			"Thompson Multimedia General Purpose Proc",
			"National Semi. 32000",
			"Tenor Network TPC",
			"Trebia SNP 1000",
			"STMicroelectronics ST200",
			"Ubicom IP2xxx",
			"MAX processor",
			"National Semi. CompactRISC",
			"Fujitsu F2MC16",
			"Texas Instruments msp430",
			"Analog Devices Blackfin DSP",
			"Seiko Epson S1C33 family",
			"Sharp embedded microprocessor",
			"Arca RISC",
			"PKU-Unity & MPRC Peking Uni. mc series",
			"eXcess configurable cpu",
			"Icera Semi. Deep Execution Processor",
			"Altera Nios II",
			"National Semi. CompactRISC CRX",
			"Motorola XGATE",
			"Infineon C16x/XC16x",
			"Renesas M16C",
			"Microchip Technology dsPIC30F",
			"Freescale Communication Engine RISC",
			"Renesas M32C",
			"reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved",
			"Altium TSK3000",
			"Freescale RS08",
			"Analog Devices SHARC family",
			"Cyan Technology eCOG2",
			"Sunplus S+core7 RISC",
			"New Japan Radio (NJR) 24-bit DSP",
			"Broadcom VideoCore III",
			"RISC for Lattice FPGA",
			"Seiko Epson C17",
			"Texas Instruments TMS320C6000 DSP",
			"Texas Instruments TMS320C2000 DSP",
			"Texas Instruments TMS320C55x DSP",
			"Texas Instruments App. Specific RISC",
			"Texas Instruments Prog. Realtime Unit",
			"reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved",
			"reserved","reserved","reserved",
			"STMicroelectronics 64bit VLIW DSP",
			"Cypress M8C",
			"Renesas R32C",
			"NXP Semi. TriMedia",
			"QUALCOMM DSP6",
			"Intel 8051 and variants",
			"STMicroelectronics STxP7x",
			"Andes Tech. compact code emb. RISC",
			"Cyan Technology eCOG1X",
			"Dallas Semi. MAXQ30 mc",
			"New Japan Radio (NJR) 16-bit DSP",
			"M2000 Reconfigurable RISC",
			"Cray NV2 vector architecture",
			" Renesas RX",
			"Imagination Tech. META",
			"MCST Elbrus",
			"Cyan Technology eCOG16",
			"National Semi. CompactRISC CR16",
			"Freescale Extended Time Processing Unit",
			"Infineon Tech. SLE9X",
			"Intel L10M",
			"Intel K10M",
			"reserved",
			"ARM AARCH64",
			"reserved",
			"Amtel 32-bit microprocessor",
			"STMicroelectronics STM8",
			"Tileta TILE64",
			"Tilera TILEPro",
			"Xilinx MicroBlaze",
			"NVIDIA CUDA",
			"Tilera TILE-Gx",
			"CloudShield",
			"KIPO-KAIST Core-A 1st gen.",
			"KIPO-KAIST Core-A 2nd gen.",
			"Synopsys ARCompact V2",
			"Open8 RISC",
			"Renesas RL78",
			"Broadcom VideoCore V",
			"Renesas 78KOR",
			"Freescale 56800EX DSC",
			"Beyond BA1",
			"Beyond BA2",
			"XMOS xCORE",
			"Microchip 8-bit PIC(r)",
			"reserved","reserved","reserved","reserved","reserved",
			"KM211 KM32",
			"KM211 KMX32",
			"KM211 KMX16",
			"KM211 KMX8",
			"KM211 KVARC",
			"Paneve CDP",
			"Cognitive Smart Memory Processor",
			"Bluechip CoolEngine",
			"Nanoradio Optimized RISC",
			"CSR Kalimba",
			"Zilog Z80",
			"Controls and Data Services VISIUMcore",
			" FTDI Chip FT32",
			"Moxie processor",
			"AMD GPU",
			"reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved","reserved",
			"reserved","reserved","reserved","reserved","reserved","reserved",
			"RISC-V",
			"not valid","not valid","not valid",
			"Linux BPF -- in-kernel virtual machine"
		};
		
		
int main(int argc, char *argv[])
{
	if(argc != 2)
	{
		printf("Greska, provjerite argumente!\n");
		return -1;
	}
			
	FILE* file = fopen(argv[1],"rb");
	if(!file)
	{
		printf("readelf: Error: '%s': No such file\n",argv[1]);
		return -1;
	}
    char uslov[]={0x7f,'E','L','F','\0'};
     char buffer[5];
			
			fread(buffer, 1, sizeof(uslov),file);
			buffer[5] = '\0';
			fseek(file,0,SEEK_SET);
			if(strcmp(buffer,uslov) == 0 )
			{
				printf("readelf: Error: '%s': No such file\n",argv[1]);
				return -1;
			}
            else
{

	Elf64_Ehdr header;
		
	fread(&header, 1, sizeof(header),file);
		
	printf("ELF Header:\n");

	printf("  Magic:   ");
	print_magic(&header);

	printf("  Class:			     %s",ret_class(&header));
		
	printf("  Data:				     %s",ret_data(&header));
		
	printf("  Version:			     %s",ret_ei_version(&header));
		
	printf("  OS/ABI:			     UNIX - System V\n");
		
	printf("  ABI Version:			     0\n");
		
	printf("  Type:				     %s", ret_type(&header));
		
	printf("  Machine:			     %s\n",ret_machine(&header));
		
	printf("  Version:			     0x%02X\n",header.e_version);
		
	printf("  Entry point address:		     0x%lX\n",header.e_entry);
		
	printf("  Start of program headers:	     %ld (bytes into file)\n",header.e_phoff);
		
	printf("  Start of section headers:	     %ld (bytes into file)\n",header.e_shoff);
		
	printf("  Flags:			     0x%x\n",header.e_flags);
		
	printf("  Size of this headers:		     %hd (bytes)\n",header.e_ehsize);
		
	printf("  Size of program headers:	     %hd (bytes)\n",header.e_phentsize);
		
	printf("  Number of program headers:	     %hd\n",header.e_phnum);
		
	printf("  Size of section headers:	     %hd (bytes)\n",header.e_shentsize);
		
	printf("  Number of section headers:	     %hd\n",header.e_shnum);
		
	printf("  Section header string table index: %hd\n",header.e_shstrndx);
	}		
	return 0;
}
void print_magic(Elf64_Ehdr* header)
{
	int i;
	for(i=0;i<16;i++){
		printf("%02x ",header->e_ident[i]);
	}
	printf("\n");
}
char* ret_class(Elf64_Ehdr* header)
{
	switch(header->e_ident[EI_CLASS])
	{
		case 0 : return("invalid class\n"); break;
		case 1 : return("ELF32\n"); break;
		case 2 : return("ELF64\n"); break;
		default : break;
	}
}
char* ret_data(Elf64_Ehdr* header)
{
	switch(header->e_ident[EI_DATA])
	{
		case 0 : return("invalid data encoding\n"); break;
		case 1 : return("2's complement, little endian\n"); break;
		case 2 : return("2's complement, big endian\n"); break;
		default : break;
	}
}
char* ret_ei_version(Elf64_Ehdr* header)
{
	switch(header->e_ident[EI_VERSION])
	{
		case 1 : return("1 (corrent)\n"); break;
		default : break;
	}
}
char* ret_type(Elf64_Ehdr* header)
{
	switch(header->e_type)
	{
		case 0 : return("No fle type\n"); break;
		case 1 : return("Relocatable file\n"); break;
		case 2 : return("Executable file\n"); break;
		case 3 : return("Shared object file\n"); break;
		case 4 : return("Core file\n"); break;
		default : break;
	}
}

char* ret_machine(Elf64_Ehdr* header)
{
	return(name_machine[header->e_machine]);
}

