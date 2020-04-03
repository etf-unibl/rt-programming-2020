#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <elf.h>

#define MAX_LEN		60*1024*1024



u_char buffer [MAX_LEN];

const char * _flags = "Key to Flags:\n"
"  W (write), A (alloc), X (execute), M (merge), S (strings), I (info),\n"
"  L (link order), O (extra OS processing required), G (group), T (TLS),\n"
"  C (compressed), x (unknown), o (OS specific), E (exclude),\n"
"  l (large), p (processor specific)\n";



void show_program_header(const u_char *);
const char *shdr_get_type(Elf64_Word);
const char *shdr_get_flags(Elf64_Xword);


const char *shdr_get_type(Elf64_Word sh_type)
{
   switch(sh_type)
		{
			case SHT_NULL:
				return "NULL";
				break;
			case SHT_PROGBITS:
				return "PROGBITS";
				break;
			case SHT_SYMTAB:
				return "SYMTAB";
				break;
			case SHT_STRTAB:
			    return "STRTAB";
				break;
			case SHT_RELA:
				return "RELA";
				break;
			case SHT_HASH:
				return "HASH";
				break;
			case SHT_DYNAMIC:
			    return "DYNAMIC";
				break;
			case SHT_NOTE:
				return "NOTE";
				break;
			case SHT_NOBITS:
		        return "NOBITS";
				break;
			case SHT_REL:
				return "REL";
				break;
			case SHT_SHLIB:
				return "SHLIB";
				break;
			case SHT_DYNSYM:
				return "DYNSYM";
				break;
			case SHT_LOPROC:
				return "LOPROC";
				break;
			case SHT_HIPROC:
				return "HIPROC";
				break;
			case SHT_LOUSER:
				return "LOUSER";
				break;
			case SHT_HIUSER:
				return "HIUSER";
				break;
			default:
				return " ";
				break;
		}
}

const char *shdr_get_flags(Elf64_Xword sh_flags)
{
    switch(sh_flags)
		{
			case SHF_WRITE:
				return " W ";
				break;
			case SHF_ALLOC:
				return " A ";
				break;
			case SHF_EXECINSTR:
				return " E ";
				break;
			case SHF_MASKPROC:
				return " M ";
				break;
			case SHF_WRITE | SHF_ALLOC:
				return " WA ";
				break;
			case SHF_WRITE | SHF_EXECINSTR:
				return " WE ";
				break;
			case SHF_ALLOC | SHF_EXECINSTR:
				return " AE ";
				break;
			case SHF_WRITE | SHF_ALLOC | SHF_EXECINSTR:
				return " WAE";
				break;
			default:
				return " ";
				break;
		}

}




void show_section_header(const u_char *data)
{
	Elf64_Ehdr *header;
	Elf64_Shdr *sheader;
	int i, j;
	int num;
	int s_off;
	u_char *name;

	header = (Elf64_Ehdr*)data;
	num =  header->e_shnum;
	s_off = header->e_shoff;
    sheader = (Elf64_Shdr*)(data + s_off +  sizeof(*sheader) * (header->e_shstrndx));
	name = (u_char *)(data + sheader->sh_offset);
   
	printf("There are %d section headers, starting at offset 0x%x:\n\n", num, s_off);
    printf("Section Headers:\n");
    printf("  %-23s%-17s%-18s%s\n", "[Nr] Name", "Type", "Address", "Offset");
    printf("       %-18s%-17s%s\n", "Size", "EntSize", "Flags  Link  Info  Align");
    for (i = 0; i < num; i++) {
        sheader = (Elf64_Shdr*)(data + i * sizeof(*sheader) + s_off);

        printf("  [%*d] %-17s  %-16s %016lx  %08lx\n",
                2, i, name + sheader->sh_name, shdr_get_type(sheader->sh_type), sheader->sh_addr, sheader->sh_offset);
        printf("       %016lx  %016lx  %-9s%-6d%-6d%ld\n",
                sheader->sh_size, sheader->sh_entsize, shdr_get_flags(sheader->sh_flags), sheader->sh_link, sheader->sh_info, sheader->sh_addralign);
    }
	
	
  printf("%s",_flags);
}



int main(int argc,char *argv[]){

   
	int len = 0;
	FILE  *elf;


    printf("argc =  %d\n", argc);
    printf("file name %s\n",argv[1]);
    elf = fopen(argv[1], "rb+") ;
	if(elf == NULL)
    {
	  printf("Open error\n");
	  printf("File %s cannot be opened\n", argv[1]);
      exit(-1);
	}
			
	len = fread(buffer,1,MAX_LEN,elf);
	buffer[len] = '\0';
    show_section_header(buffer);
    fclose(elf);
    

}














