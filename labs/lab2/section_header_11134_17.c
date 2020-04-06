#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elf.h>

void print_type(Elf64_Word type)
{
	if(type==0) printf (" NULL            ");
	if(type==1) printf (" PROGBITS        ");
	if(type==2) printf (" SYMTAB          ");
	if(type==3) printf (" STRTAB          ");
	if(type==4) printf (" RELA            ");
	if(type==5) printf (" HASH            ");
	if(type==6) printf (" DYNAMIC         ");
	if(type==7) printf (" NOTE            ");
	if(type==8) printf (" NOBITS          ");
	if(type==9) printf (" REL             ");
	if(type==10) printf (" SHLIB           ");
	if(type==11) printf (" DYNSYM          ");
	if(type==0x70000000) printf (" LOPROC          ");
	if(type==0x7fffffff) printf (" HIPROC          ");
	if(type==0x80000000) printf (" LOUSER          ");
	if(type==0xffffffff) printf (" HIUSER          ");
}

void print_flag(Elf64_Word flag)
{
	if(flag==0x1) printf("W    ");
	if(flag==0x2) printf("A    ");
	if(flag==0x4) printf("X    ");
	if(flag==0xf0000000) printf("M    ");
	if(flag==0x3) printf("WA   ");
	if(flag==0x5) printf("WE   ");
	if(flag==0x6) printf("AX   ");
	if(flag==0x20) printf("S    ");
	if(flag==0x40) printf("I    ");
	if(flag==0x80) printf("L    ");
	if(flag==0x100) printf("O    ");
	if(flag==0x200) printf("G    ");
	if(flag==0x400) printf("T    ");
	if(flag==0x80000000) printf("E    ");
}



int main(int argv, char *args[]) 
{
	if(argv!=3) return -1;
	if(strcmp(args[1],"--section-header")!=0) 
	{
		printf("command not found\n");
		printf("Did you mean --section-header\n\n");
		return -1;
	}

	FILE *file;
	if((file=fopen(args[2],"rb"))==NULL)
	{
     		printf("Error in file opening...\n");
     		return -1;
    	}
	
	Elf64_Ehdr ehdr64;
	fread(&ehdr64, 1, sizeof(Elf64_Ehdr), file);

	int number_of_sections = ehdr64.e_shnum;
	int offset_for_sections = ehdr64.e_shoff;
	
	Elf64_Shdr *shdr64 = (Elf64_Shdr*) malloc (sizeof(Elf64_Shdr)*number_of_sections);	

	fseek(file, offset_for_sections, SEEK_SET);	
	fread(shdr64, number_of_sections, sizeof(Elf64_Shdr), file);

	printf("There are %d section headers starting at position 0x%x\n", 
		number_of_sections, offset_for_sections);

	printf("Section Headers:\n");
	printf("[Nr] NAME              TYPE             ADRESS            OFFSET\n");
	printf("     SIZE              ENTSIZE          FLAGS  LINK  INFO  ALIGN\n");
	
	for(int i=0; i<number_of_sections; i++)
	{
		printf("[%2d] ", i);
		printf("%-15d  ", shdr64[i].sh_name);
		print_type(shdr64[i].sh_type);		
		printf("%016lx  ", shdr64[i].sh_addr);
		printf("%08lx", shdr64[i].sh_offset);
		printf("\n");
		printf("     %016lx", shdr64[i].sh_size);
		printf("%16lx   ", shdr64[i].sh_entsize);

		if(shdr64[i].sh_flags)
			print_flag(shdr64[i].sh_flags); else
				printf("none  ");
		
		printf("%5d", shdr64[i].sh_link);
		printf("%6d", shdr64[i].sh_info);
		printf(" %5ld", shdr64[i].sh_addralign);

		printf("\n");
	}
		
	return 0;
}
