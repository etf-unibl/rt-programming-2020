#include <stdio.h>
#include <stdlib.h>
#include <elf.h>
/*
 * U elf.h su definisane konstante za sve validne vrijednosti razlicitih polja
 * u zaglavlju, kao i struktura podataka kojom se predstavlja zaglavlje
 * fajla, ali i druga zaglavlja u elf formatu.
 * 
 * #define EI_NIDENT 16
 * 
 * typedef struct {
 * unsigned char e_ident[EI_NIDENT];
 *      uint16_t      e_type;
 *      uint16_t      e_machine;
 *      uint32_t      e_version;
 *      ElfN_Addr     e_entry;
 *      ElfN_Off      e_phoff;
 *      ElfN_Off      e_shoff;
 *      uint32_t      e_flags;
 *      uint16_t      e_ehsize;
 *      uint16_t      e_phentsize;
 *      uint16_t      e_phnum;
 *      uint16_t      e_shentsize;
 *      uint16_t      e_shnum;
 *      uint16_t      e_shstrndx;
 * } ElfN_Ehdr;
*/

/*
 * ELF64 i ELF32 zagavlje nisu iste duzine, tako da se ne zna tokom prevodjenja da li ce biti potrebna promjenljiva Elf32_Ehdr ili Elf63_Ehdr.
 * Da se ne bi pisalo puno uslova koristi se void*.
 * Posto se koristi void*, a ne moze se kastovati(opet sirimo kod sa mnogim uslovima), pa se ne moze koristiti operator ->.
 * Potrebno je napraviti posebnu funkciju koja ce omoguciti rad sa void* koji pokazuje na podatak tipa Elf32_Ehdr ili Elf63_Ehdr.
 * Dva niza nose u sebi informaciju o pomjeraju zasebnih podataka u strukturi u odnosu na pocetak strukture.
 * Na osnovu velicine zaglavlja koja se utvrdi na pocetku izvrsavanja programa odlucuje se koji niz ce biti koristen.
*/
static int off32[]={0,16,18,20,24,28,32,36,40,42,44,46,48,50};
static int off64[]={0,16,18,20,24,32,40,48,52,54,56,58,60,62};
void* get_header_field(void* p, int size, int num){
	if(size==52){
		return p+off32[num-1];
	}
	return p+off64[num-1];
}

char* get_os(unsigned char num){
	switch(num){
		case ELFOSABI_NONE:
			return "UNIX System V ABI";
		case ELFOSABI_HPUX:
			return "HP-UX";
		case ELFOSABI_NETBSD:
			return "NetBSD";
		case ELFOSABI_GNU:
			return "Object uses GNU ELF extensions";
		case ELFOSABI_SOLARIS:
			return "Sun Solaris";
		case ELFOSABI_AIX:
			return "IBM AIX";
		case ELFOSABI_IRIX:
			return "SGI Irix";
		case ELFOSABI_FREEBSD:
			return "FreeBSD";
		case ELFOSABI_TRU64:
			return "Compaq TRU64 UNIX";
		case ELFOSABI_MODESTO:
			return "Novell Modesto";
		case ELFOSABI_OPENBSD:
			return "OpenBSD";
		case ELFOSABI_ARM_AEABI:
			return "ARM EABI";
		case ELFOSABI_ARM:
			return "ARM";
		case ELFOSABI_STANDALONE:
			return "Standalone (embedded) application";
		default:
			return "unknown";
	}
}

char* get_type(uint16_t num){
	if(num>=ET_LOOS && num<=ET_HIOS){
		printf("%4x",num);
		return " OS-specific";
	}
	else{
		if(num>ET_LOPROC && num<=ET_HIPROC){
			printf("%4x",num);
			return " Processor-specific";
		}
	}
	switch(num){
		case ET_NONE:
			return "No file type";
		case ET_REL:
			return "REL (Relocatable file)";
		case ET_EXEC:
			return "EXEC (Executable file)";
		case ET_DYN:
			return "DYN (Shared object file)";
		case ET_CORE:
			return "CORE (Core file)";
		default:
			return "reserved value";
	}
}

/*
 * Imena su poredana tako da njihovi indeksi u nizu odgovaraju vrijednostima konstanti u elf.h
 */
static char* machine[]={
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

char* get_machine(uint16_t num){
	if(num>=EM_NUM){
		return "not valid";
	}
	return machine[num];
}

int main(int argc, char** argv){
	//Ime fajla se salje kao argument komandne linije.
	if(argv[1]==NULL){
		printf("Ime fajla nije proslijedjeno!\n");
		return -1;
	}
	FILE* f=fopen(argv[1],"rb");
	if(f==NULL){
		printf("Datoteka ne postoji ili je putanja do nje losa!\n");
		return -1;
	}

	//Provjera da li se radi o objektnom fajlu. Prva 4 bajta moraju biti redom 0x7f, 'E', 'L', 'F'.
	char provjera[]={0x7f,'E','L','F'};
	int c=0,i;
	for(i=0;i<4;i++){
		c=fgetc(f);
		if(c!=provjera[i] || feof(f)!=0){
			printf("Proslijedjena datoteka nije ELF datoteka!\n");
			return -1;
		}
	}

	/*
	   Kod 32 bitnih sistema velicina zaglavlja je 52 bajta, a kod 64 bitnih sistema je 64 bajta.
	   Tako da prije citanja zaglavlja treba provjeriti na kojem sistemu je objektni fajl napisan.
	   Peti bajt sluzi za prepoznavanje da li je program napisan da radi na 32 ili 64 bitnim arhitekturama.

	   ELFCLASSNONE  This class is invalid.
       ELFCLASS32    This defines the 32-bit  architecture.
                         It  supports  machines  with files and
                         virtual address spaces up to  4  Giga‐
                         bytes.
       ELFCLASS64    This defines the 64-bit architecture.
	*/
	void* header=NULL;
	int size=0;
	c=fgetc(f);
	if(c!=1 && c!=2){
		/*
		 * Prilikom testiranja readelf --file-header je davao i ispis za fajlove koji su imali klasu 0, ali posto sam
		 * na mnogo razlicitih mjesta nasao da su samo 1 i 2 validne vrijednosti ovaj program ce prijaviti gresku i nece
		 * pokusati dalje citanje zaglavlja takvih falova. Iako postoji posebno polje na osnovu kojeg bi se mogla odrediti velicina zaglavlja
		 * i onda na osnovu toga procitati zaglavlje ono se nalazi poslije e_entry, e_phoff i e_shoff cija velicina zavisi od toga da li
		 * radi o elf32 ili elf64. Ovo znaci da se polozaj tog pola ne moze sigurno odrediti ako ne poznajemo klasu sto dalje znaci
		 * da ako bi htjeli da program cita zaglavlja morali bi pretpostviti velicinu sto dalje znaci da ne bi bili sigurni za tacnost
		 * vrijednosti u ostalim poljima.
		 * */
		printf("Nevalidna klasa!\n");
		return -1;
	}
	else{
		if(c==1){
			size=sizeof(Elf32_Ehdr);
		}
		else{
			size=sizeof(Elf64_Ehdr);
		}
	}

	//alokacija memorije za zaglavlje
	header=calloc(1,size);
	//citanje zaglavlja
	rewind(f);
	c=fread(header,size,1,f);
	/*
	 * provjeravamo da nije doslo do greske pri citanju. 
	 * Ako nije procitan 1 blok od size najtova, mozda je pocetak fajla dobar, a ostatak nedostaje(funkcija dosla do EOF).
	 * */
	if(c==1){
		printf("File header:\n  Magic:   ");

		//magic numbers
		for(int i=0;i<EI_NIDENT;i++){
			printf("%02x ",*((unsigned char*)(get_header_field(header,size,1))+i));
		}


		/* class
		   Vec smo utvrdili prije citanja na osnovu 5. bajta da li se radi o elf32 ili elf64.
		*/
		printf("\n  Class:                             ELF");
		if(size==52){
			printf("32");
		}
		else{
			printf("64");
		}


		/* 
		 * data
		 * Sesti bajt se nosi informaciju o tome  kako su kodovani podaci. Podrzane tri vrijednosti.
		 * 
		 *     ELFDATANONE   Unknown data format.
		 *     ELFDATA2LSB   Two's complement, little-endian.
		 *     ELFDATA2MSB   Two's complement, big-endian.
		 * 
		 * readelf za 0 ispisuje none, a za sve preko 1 i 2 unknown    
		*/
		printf("\n  Data:                              ");
		if((c=*((unsigned char*)(get_header_field(header,size,1))+5))==ELFDATANONE){
			printf("none");
		}
		else{
			if(c==ELFDATA2LSB){
				printf("2's complement, little endian");
			}
			else{
				if(c==ELFDATA2MSB){
					printf("2's complement, big endian");
				}
				else{
					printf("unknown");
				}
			}
		}


		//verzija
		printf("\n  Version:                           ");
		if(*((unsigned char*)(get_header_field(header,size,1))+6)==EV_NONE){
			printf("0 (invalid)");
		}
		else{
			if(*((unsigned char*)(get_header_field(header,size,1))+6)==EV_CURRENT){
				printf("%d (current)",EV_CURRENT);
			}
			else{
				printf("%d (unknown)",*((unsigned char*)(get_header_field(header,size,1))+6));
			}
		}


		//OS/ABI
		printf("\n  OS/ABI:                            ");
		printf("%s",get_os(*((unsigned char*)(get_header_field(header,size,1))+7)));


		//ABI version
		printf("\n  ABI Version:                       %d",*((unsigned char*)(get_header_field(header,size,1))+8));


		//Tip datoteke
		printf("\n  Type:                              %s",get_type(*((uint16_t*)(get_header_field(header,size,2)))));


		//Machine
		printf("\n  Machine:                           %s",get_machine(*((uint16_t*)(get_header_field(header,size,3)))));


		//Version
		printf("\n  Version:                           0x%x",*((uint32_t*)get_header_field(header,size,4)));

		printf("\n  Entry point address:               ");
		if(size==52){
			printf("0x%x",*((Elf32_Addr*)get_header_field(header,size,5)));
		}
		else{
			printf("0x%lx",*((Elf64_Addr*)get_header_field(header,size,5)));
		}


		printf("\n  Start of program headers:          ");
                if(size==52){
                        printf("%ud",*((Elf32_Addr*)get_header_field(header,size,6)));
                }
                else{
                        printf("%lu",*((Elf64_Addr*)get_header_field(header,size,6)));
                }
		printf(" (bytes into file)");


		printf("\n  Start of section headers:          ");
                if(size==52){
                        printf("%ud",*((Elf32_Addr*)get_header_field(header,size,7)));
                }
                else{
                        printf("%lu",*((Elf64_Addr*)get_header_field(header,size,7)));
                }
		printf(" (bytes into file)");


		printf("\n  Flags:                             0x%x",*((uint32_t*)get_header_field(header,size,8)));


		printf("\n  Size of this header:               %d (bytes)",size);


		printf("\n  Size of program headers:           %d (bytes)",*((uint16_t*)get_header_field(header,size,10)));


		printf("\n  Number of program headers:         %d",*((uint16_t*)get_header_field(header,size,11)));


		printf("\n  Size of section headers:           %d (bytes)",*((uint16_t*)get_header_field(header,size,12)));


		printf("\n  Number of section headers:         %d",*((uint16_t*)get_header_field(header,size,13)));


		printf("\n  Section header string table index: %d\n",*((uint16_t*)get_header_field(header,size,14)));
	}
	else{
		printf("Greska pri citanju!\n");
	}
	free(header);
	fclose(f);
	return 0;
}
