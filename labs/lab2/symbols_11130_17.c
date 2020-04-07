#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libelf.h>
#include <stdint.h>
#include <gelf.h>
#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
/*
For solving my homework (readelf --symbols) i relied on gelf library. 
Library gelf gives simple API for manipulation with ELF object files and data.
We will only use functions for reading, from gelf.h library. 
In our case we dont think about our aplication being designed for 32b or 64b. 
Because <gelf.h> internally decides what structures it will use (elf32_* or elf64_*). 


Credits: 
http://www.skyfree.org/linux/references/ELF_Format.pdf
https://docs.oracle.com/cd/E19683-01/817-0679/6mgfb8784/index.html
https://refspecs.linuxbase.org/elf/gabi4+/ch4.symtab.html
 
Nedo Todoric
01.04.2020. 


sudo apt-get install libelf-dev
gcc symbols_11130_17.c -l elf -o exe_name
*/

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif 

static Elf *elf;
static GElf_Shdr symtab_shdr;
 
static _Bool print_symbols(Elf *elf, Elf_Scn *scn, GElf_Shdr *shdr, unsigned count)
{
	Elf_Data *data;
	char *name;
    char *size;
	char *stringName;
	data = 0;
	int number = 0;
    int type,bind;
    char type_s[15], bind_s[15], vis_s[15], ndx_s[15]; 
	if ((data = elf_getdata(scn, data)) == 0 || data->d_size == 0)
	{
		fprintf(stderr, "Section had no data!\n");
			exit(-1);
	}
	printf("    Num:    Value         Size  Type     Bind   Vis      Ndx Name  \n");            //formating
	for (unsigned i = 0; i < count; ++i)
	{
		GElf_Sym esym;
		GElf_Sym *ret = gelf_getsym(data, i, &esym);
		if (ret == 0) { fprintf(stderr, "Error!\n"); return FALSE; }
		name = elf_strptr(elf, shdr->sh_link, (size_t)esym.st_name);
		if(!name)
		{
			fprintf(stderr,"%s\n",elf_errmsg(elf_errno()));
			exit(-1);
		}
        if (esym.st_info>15) 
                {
                    type=(unsigned)(esym.st_info)&0xf; 
                    bind=(unsigned)(esym.st_info)>>4; 
                }
        else    
                {
                    type=esym.st_info;
                    bind=esym.st_info; 
                }
        if (type==0) strcpy(type_s,"NOTYPE");
        else if (type==1) strcpy(type_s,"OBJECT");
        else if (type==2) strcpy(type_s,"FUNC");
        else if (type==3) strcpy(type_s,"SECTION");              
        else if (type==4) strcpy(type_s,"FILE");
        else if (type==13) strcpy(type_s,"LOPROC");
        else if (type==15) strcpy(type_s,"HIPROC");
        if (bind==0) strcpy(bind_s," LOCAL");                          
        else if (bind==1) strcpy(bind_s," GLOBAL");
        else if (bind==2) strcpy(bind_s," WEAK");
        else if (bind==13) strcpy(bind_s," LOPROC");
        else if (bind==15) strcpy(bind_s," HIPROC");
        if(esym.st_other==0) strcpy(vis_s," DEFAULT");
        else if(esym.st_other==1) strcpy(vis_s," INTERNAL");               
        else if(esym.st_other==2) strcpy(vis_s," HIDDEN");
        else if(esym.st_other==3) strcpy(vis_s," PROTECTED");
        if(esym.st_shndx==0) strcpy(ndx_s," UND");
        else if(esym.st_shndx==65521) strcpy(ndx_s," ABS"); 
        else  sprintf(ndx_s,"%d",esym.st_shndx);            // formating the output exactly like readelf --sym does!
		printf("   %3d:  %016X %4d %-8s%-7s%-9s%+5s %s\n", number++,esym.st_value,esym.st_size,type_s,bind_s,vis_s,ndx_s,name);
	}
    printf("\n");
	return TRUE;
}
 

static void check_symbol_table()
{
	Elf_Scn *section = 0;
    int n=0;
	while ((section = elf_nextscn(elf, section)) != 0) {
		if (gelf_getshdr (section, &symtab_shdr) != 0) {

		if (symtab_shdr.sh_type == SHT_SYMTAB) 
            {   n++;
                unsigned count = symtab_shdr.sh_size / symtab_shdr.sh_entsize;
                printf("Symbol table '.symtab' contains %u entries\n",count); 
                print_symbols(elf, section, &symtab_shdr,count);
            }
        else if (symtab_shdr.sh_type == SHT_DYNSYM) 
            {    n++;
                 unsigned count = symtab_shdr.sh_size / symtab_shdr.sh_entsize;
                 printf("Symbol table '.dynsym' contains %u entries\n",count); 
                 print_symbols(elf, section, &symtab_shdr,count);
             }
		}
		}
	if (n==0) printf("No symbol table was found!\n");
}

int main(int argc, char **argv)
{
	assert(argc > 1); 
	
	char *filename = argv[1];
	
	if (elf_version(EV_CURRENT) == EV_NONE ) {
		/* library out of date */
		fprintf(stderr, "Elf library out of date!n");
		exit(-1);
	}

	int fd = open(argv[1], O_RDONLY);
	if ((elf = elf_begin(fd, ELF_C_READ, NULL)) == NULL){
    printf("error! file doesnt exist!\n"); 
    exit(-1); 
	}  

	check_symbol_table(); 
    return 0;
}
 

