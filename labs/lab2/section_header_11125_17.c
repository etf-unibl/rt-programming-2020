#include <stdio.h>
#include <stdlib.h>
#include <elf.h>
#include <string.h>

void read_elfh_64(FILE*);

int main(int argc, char* argv[])
{
	if (argc < 2)
		return -1;
	
	FILE* file = fopen(argv[1], "rb");
	char arc[5];
	
	if(file != NULL)
	{
		
		fread(&arc, 1, 5, file); 
		fseek(file, 0, 0); 
		if(arc[4] == 1)
			printf("32byte arh");
		else if(arc[4] == 2)
			read_elfh_64(file);
		fclose(file);
	}
	return 0;
}


void type(Elf64_Word sh_type)
{
	switch (sh_type) {
		case 0:
			printf("%-20s","NULL");
			break;
		case 1:
			printf("%-20s","PROGBITS");
			break;
		case 2:
			printf("%-20s","SYMTAB");
			break;
		case 3:
			printf("%-20s","STRTAB");
			break;
		case 4:
			printf("%-20s","RELA");
			break;
		case 5:
			printf("%-20s","GNU_HASH");
			break;
		case 6:
			printf("%-20s","DYNAMIC");
			break;
		case 7:
			printf("%-20s","NOTE");
			break;
		case 8:
			printf("%-20s","NOBITS");
			break;
		case 9:
			printf("%-20s","REL");
			break;
		case 10:
			printf("%-20s","SHLIB");
			break;
		case 11:
			printf("%-20s","DYNSYM");
			break;
		case 0x70000000:
			printf("%-20s","LOPROC");
			break;
		case 0x7fffffff:
			printf("%-20s","HIPROC");
			break;
		case 0x80000000:
			printf("%-20s","LOUSER");
			break;
		case 0xffffffff:
			printf("%-20s","HIUSER");
			break;
		default:
			printf("%-20s","");
	}
}
   
   
 void flags(Elf64_Word sh_flags)
 {
	 	switch (sh_flags) {
                case 0x1:
			printf("%5s","W");
			break;
		case 0x2:
			printf("%5s","A");
			break;
		case 0x4:
			printf("%5s","E");
			break;
		
		case 0xf0000000:
			printf("%5s","M");
			break;
		default:
			printf("%5s"," ");
		
	}
	 
 } 


void read_elfh_64(FILE* file)


{
	Elf64_Ehdr h;
	fread(&h, sizeof(h), 1, file);
	int number;
	number=h.e_shnum;
	int offset=h.e_shoff;

        Elf64_Shdr *h64;
        h64=(Elf64_Shdr *)malloc(number*sizeof(Elf64_Shdr));
        fseek(file, offset, SEEK_SET);
	fread(h64, number, sizeof(Elf64_Shdr), file);

        printf("There are %d section headers, starting at offset 0x%x:\n\n", number, offset);
        printf("Section Headers:\n");
	printf("\n");
        printf("  [Nr] Name                     Type                Address             Offset\n");
        printf("       Size                     EntSize             Flags  Link  Info   Align\n");
	for(int i=0;i<number;i++)
	{
	printf("[%d]",i);
	printf("%-25d",h64[i].sh_name);
	type(h64[i].sh_type);
	printf("%015lx", h64[i].sh_addr);
	printf("%08lx\n", h64[i].sh_offset);
	printf("%015lx", h64[i].sh_size);
        printf("%015lx\n", h64[i].sh_entsize);
        flags(h64[i].sh_flags);
	printf("%8d", h64[i].sh_link);
	printf("%5d", h64[i].sh_info);
	printf("%10lu\n", h64[i].sh_addralign);     
        
	
	
}
printf("Key to Flags:\n"); 
	printf(" W (write), A (alloc), X (execute), M (merge), S (strings), l (large)\n");
  	printf(" I (info), L (link order), G (group), T (TLS), E (exclude), x (unknown)\n");
  	printf(" O (extra OS processing required) o (OS specific), p (processor specific)\n");

}
