#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>


#define EI_NIDENT       16
#define SHT_SYMTAB      2
#define SHT_STRTAB		3
#define SHT_DYNSYM      11

#define ELF64_ST_BIND(info)          ((info) >> 4)
#define ELF64_ST_TYPE(info)          ((info) & 0xf)
#define ELF64_ST_INFO(bind, type)    (((bind)<<4)+((type)&0xf))


typedef unsigned long long Elf64_Addr;
typedef unsigned short Elf64_Half;
typedef signed short Elf64_SHalf;
typedef unsigned long long Elf64_Off;
typedef signed int Elf64_Sword;
typedef unsigned int Elf64_Word;
typedef unsigned long long Elf64_Xword;
typedef signed long long Elf64_Sxword;

typedef struct elf64_hdr {
	unsigned char e_ident[EI_NIDENT]; 
	Elf64_Half e_type;
	Elf64_Half e_machine;
	Elf64_Word e_version;
	Elf64_Addr e_entry;       
	Elf64_Off  e_phoff;        
	Elf64_Off  e_shoff;        
	Elf64_Word e_flags;
	Elf64_Half e_ehsize;
	Elf64_Half e_phentsize;
	Elf64_Half e_phnum;
	Elf64_Half e_shentsize;
	Elf64_Half e_shnum;
	Elf64_Half e_shstrndx;
} Elf64_Ehdr;

typedef struct elf64_shdr {
	Elf64_Word sh_name;       
	Elf64_Word sh_type;      
	Elf64_Xword sh_flags;     
	Elf64_Addr sh_addr;       
	Elf64_Off sh_offset;      
	Elf64_Xword sh_size;      
	Elf64_Word sh_link;       
	Elf64_Word sh_info;       
	Elf64_Xword sh_addralign; 
	Elf64_Xword sh_entsize;   
} Elf64_Shdr;

typedef struct {
	Elf64_Word      st_name;
	unsigned char   st_info;
	unsigned char   st_other;
	Elf64_Half      st_shndx;
	Elf64_Addr      st_value;
	Elf64_Xword     st_size;
} Elf64_Sym;


//funkcija koja ucitava symbol table section
void get_tab_section(int *num,int *link,Elf64_Sym **tab,Elf64_Shdr *sect_header,FILE *ElfFile )
{
    *num=sect_header->sh_size/sect_header->sh_entsize;
    *link=sect_header->sh_link;
	*tab=(Elf64_Sym *)malloc(sect_header->sh_size);
    fseek(ElfFile,sect_header->sh_offset,SEEK_SET);
	fread(*tab, 1,sect_header->sh_size, ElfFile);
}
//funkcija koja ucitava string tabelu u memoriju
void get_str_section(char **string_tab,Elf64_Shdr *sect_header,FILE *ElfFile )
{
    *string_tab=(char *) malloc(sect_header->sh_size);
	fseek(ElfFile,sect_header->sh_offset,SEEK_SET);
	fread(*string_tab,1,sect_header->sh_size, ElfFile);
}
//mapiranje int u string potrebno prilikom ispisa
 struct entry {
    char *str;
    int n;
};

struct entry type[] = {
    "NOTYPE",0,
    "OBJECT",1,
    "FUNC",2,
    "SECTION",3,
    "FILE",4,
    "COMMON",5,
    "TLS",6,
    "LOOS",10,
    "HIOS",12,
    "LOPROC",13,
    "HIPROC",15,
    NULL,-1
};

struct entry bind[]= 
{
    "LOCAL",0,
    "GLOBAL",1,
    "WEAK",2,
    "LOOS",10,
    "HIOS",12,
    "LOPROC",13,
    "HIPROC",15,
    NULL,-1
};

struct entry visibility[]= 
{
    "DEFAULT",0,
    "INTERNAL",1,
    "HIDDEN",2,
    "PROTECTED",3,
    "EXPORTED",4,
    "SINGLETON",5,
    "ELIMINATE",6,
    NULL,-1
};

struct entry ndx[]= 
{
    "UND",0,
    "LOR",0xff00,
    "ABS",0xfff1,
    "COM",0xfff2,
    "HIR",0xffff,
     NULL,-1
};

char * int_to_string(int key, struct entry *dict)
{
    int i=0;
    int n = dict[i].n;
    while (n!=-1) {
        if (key==n)
            return dict[i].str;
        n = dict[++i].n;
    }
    return NULL;
}

void print( Elf64_Sym  *sym_tab,char *string_sym,int num_sym)
{
    int i;
    for(i=0;i<num_sym;i++)
   {
	printf("%6d: %016llx %5llu %-7s %-6s %-9s",i, sym_tab[i].st_value, sym_tab[i].st_size, int_to_string(ELF64_ST_TYPE(sym_tab[i].st_info),type),
    int_to_string(ELF64_ST_BIND(sym_tab[i].st_info),bind), int_to_string(sym_tab[i].st_other,visibility));

    int_to_string(sym_tab[i].st_shndx,ndx)==NULL?printf("%3d ",sym_tab[i].st_shndx):printf("%-4s",int_to_string(sym_tab[i].st_shndx,ndx));

    printf("%s\n",&string_sym[sym_tab[i].st_name]);   
    }
}

int main(int argc, char **argv) 
{
    FILE* ElfFile = NULL;
    Elf64_Ehdr elf_header;//
    Elf64_Shdr *sect_header=NULL;
    Elf64_Sym  *sym_tab=NULL,*dynsym_tab=NULL;
    char *string_sym,*string_dynsym;
    int num_sym,num_dynsym;
    int i,sym_link,dynsym_link;
    if(argc<2){
        printf("Prilikom pokretanja unesite argument\n");
        return 0;
    }
	if((ElfFile = fopen(argv[1], "r")) == NULL) 
    {
        printf("Error: ");
        return 0;
    }

    //Ucitavam elf_header
    fread(&elf_header, 1, sizeof(Elf64_Ehdr), ElfFile);
    
    //na osnovu broja sekcija(e_shnum) iz elf_header zauzimam potrebnu memoriju za section header
    sect_header=(Elf64_Shdr *)malloc(elf_header.e_shnum*sizeof(Elf64_Shdr));
    
    //pozicioniram se u section header  
    fseek(ElfFile,elf_header.e_shoff,SEEK_SET);
    
    //Ucitavam section header
    fread(sect_header,elf_header.e_shnum,sizeof(Elf64_Shdr),ElfFile);
   
     //trazim sym i dynsym sekcije kao i string tabele u kojima se cuvaju imena te ih ucitavam    
    for(int i=0;i<elf_header.e_shnum;i++)
    {
	    //ucitavanje sym tabele
        if(sect_header[i].sh_type==SHT_SYMTAB)
		{
			get_tab_section(&num_sym,&sym_link,&sym_tab,&sect_header[i],ElfFile);// ucitavanje sym tabele
			get_str_section(&string_sym,&sect_header[sym_link],ElfFile);// ucitavanje string tabele u kojoj se nalaze imena
			
		}
        //ucitavanje dynsym tabele
		if(sect_header[i].sh_type==SHT_DYNSYM)
		{
			get_tab_section(&num_dynsym,&dynsym_link,&dynsym_tab,&sect_header[i],ElfFile);// ucitavanje dynsym tabele
			get_str_section(&string_dynsym,&sect_header[dynsym_link],ElfFile);// ucitavanje string tabele u kojoj se nalaze imena za dynsym tabelu
			
		}
     }
    //ispisivanje simbola
    printf("Symbol table '.dynsym' contains %d entries:\n",num_dynsym);
    print(dynsym_tab,string_dynsym,num_dynsym);
    printf("Symbol table '.symtab' conatins %d entries:\n",num_sym);
    print(sym_tab,string_sym,num_sym);
    
	free(sym_tab);
    free(dynsym_tab);
    free(string_sym);
    free(string_dynsym);

    return 0;
  }
