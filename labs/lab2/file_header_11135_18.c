#include <elf.h>
#include <stdio.h>
#include <stdlib.h>

///kao argument sa komandne linije mu se salje ime programa
int main(int argc, char *argv[]){
	
	if(argc != 2){
		printf("Wrong number of arguments suplied!");
		return -1;
	}
	
	Elf64_Ehdr header;
	
	FILE* file = fopen(argv[1],"rb");
	if(!file){
		printf("Error,while opening file!");
		return -1;
	}
	
	fread(&header, 1, sizeof(header),file);
	
	if(header.e_ident[EI_MAG0] == 0x7f &&
		header.e_ident[EI_MAG1] == 'E' &&
		header.e_ident[EI_MAG2] == 'L' &&
		header.e_ident[EI_MAG3] == 'F'){
		printf("ELF Header:\n");
		printf("  Magic:   ");
		int i;
		for(i=0;i<16;i++){
			printf("%02X ",header.e_ident[i]);
		}
		printf("\n");

		if(header.e_ident[EI_CLASS] == 0){
			printf("  Class:			     invalid class\n");
		}else if(header.e_ident[EI_CLASS] == 1){
			printf("  Class:			     ELF32\n");
		}else if(header.e_ident[EI_CLASS] == 2){
			printf("  Class:			     ELF64\n");
		}else{
			return -1;
		}
		
		if(header.e_ident[EI_DATA] == 0){
			printf("  Data:				     invalid iata encoding\n");
		}else if(header.e_ident[EI_DATA] == 1){
			printf("  Data:				     litle endian\n");
		}else if(header.e_ident[EI_DATA] == 2){
			printf("  Data:				     big endian\n");
		}else{
			return -1;
		}
		
		if(header.e_ident[EI_VERSION] == 1){
			printf("  Version:			     1 (corrent)\n");
		}else{
			return -1;
		}
		printf("  OS/ABI:			     UNIX - System V\n");
		printf("  ABI Version:			     0\n");
		
		switch(header.e_type){
			case 0:printf("  Type:				     No fle type\n");break;
			case 1:printf("  Type:				     Relocatable file\n");break;
			case 2:printf("  Type:				     Executable file\n");break;
			case 3:printf("  Type:				     Shared object file\n");break;
			case 4:printf("  Type:				     Core file\n");break;
			default:break;
		}
		
		///nisam znao kako da poredim sve moguce masine pa sam samo unjeo slucaj za moju
		if(header.e_machine == 62){
			printf("  Machine			     Advanced Micro Devices X86-64\n");
		}else{
			printf("  Machine:			     %d\n",header.e_machine);
		}
		
		printf("  Version:			     0x%02X\n",header.e_version);
		
		printf("  Entry point address:		     0x%lX\n",header.e_entry);
		
		printf("  Start of program headers:	     %ld (bytes into file)\n",header.e_phoff);
		
		printf("  Start of section headers:	     %ld (bytes into file)\n",header.e_shoff);
		
		printf("  Flags:			     0x%X\n",header.e_flags);
		
		printf("  Size of this headers:		     %hd (bytes)\n",header.e_ehsize);
		
		printf("  Size of program headers:	     %hd (bytes)\n",header.e_phentsize);
		
		printf("  Number of program headers:	     %hd\n",header.e_phnum);
		
		printf("  Size of section headers:	     %hd (bytes)\n",header.e_shentsize);
		
		printf("  Number of section headers:	     %hd\n",header.e_shnum);
		
		printf("  Section header string table index: %hd\n",header.e_shstrndx);
	}
	return 0;
}
