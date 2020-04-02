#include<stdio.h>
#include<stdint.h>
#define EI_NIDENT       16
typedef unsigned short Elf64_Half;
typedef unsigned long Elf64_Addr;
typedef unsigned long Elf64_Off;
typedef unsigned int Elf64_Word;
typedef unsigned long Elf64_Xword;

typedef struct {
        unsigned char   e_ident[EI_NIDENT]; 
        Elf64_Half      e_type;
        Elf64_Half      e_machine;
        Elf64_Word      e_version;
        Elf64_Addr      e_entry;
        Elf64_Off       e_phoff;
        Elf64_Off       e_shoff;
        Elf64_Word      e_flags;
        Elf64_Half      e_ehsize;
        Elf64_Half      e_phentsize;
        Elf64_Half      e_phnum;
        Elf64_Half      e_shentsize;
        Elf64_Half      e_shnum;
        Elf64_Half      e_shstrndx;
} Elf64_Ehdr;

typedef struct {
        Elf64_Word      p_type;
        Elf64_Word      p_flags;
        Elf64_Off       p_offset;
        Elf64_Addr      p_vaddr;
        Elf64_Addr      p_paddr;
        Elf64_Xword     p_filesz;
        Elf64_Xword     p_memsz;
        Elf64_Xword     p_align;
} Elf64_Phdr;


int main(int argc, char* argv[])
{
    
    if(argc!=2)
    {printf("Invalid nuber of arguments!");
        return 0;
    }
    
    char* file_name=argv[1];
    FILE* file;
    file=fopen(file_name,"r");
    Elf64_Ehdr e_header;
    
    fread(&e_header,sizeof(Elf64_Ehdr),1,file);
    Elf64_Phdr p_header[e_header.e_phnum];
    printf("There are %d program headers starting at offset %d\n",e_header.e_phnum,e_header.e_phoff);
    
    printf("Program Headers:\n");
printf("Type            Offset              VirtAddr            PhysAddr\n");
printf("                Filesiz             Memsiz              Flags    Align\n");
fseek(file,e_header.e_phoff,SEEK_SET);
fread(&p_header,sizeof(Elf64_Phdr),e_header.e_phnum,file);
for(int i=0;i<e_header.e_phnum;i++)
{
    if(p_header[i].p_type==0x0)
        printf("NULL        ");
    else if(p_header[i].p_type==0x1)
        printf("LOAD        ");
    else if(p_header[i].p_type==0x2)
        printf("DYNAMIC     ");
    else if(p_header[i].p_type==0x3)
        printf("INTERP      ");
    else if(p_header[i].p_type==0x4)
        printf("NOTE        ");
    else if(p_header[i].p_type==0x5)
        printf("SHLIB       ");
    else if(p_header[i].p_type==0x6)
        printf("PHDR        ");
    else if(p_header[i].p_type==0x7)
        printf("TLS         ");
    else if(p_header[i].p_type==0x60000000)
        printf("LOOS        ");
    else if(p_header[i].p_type>0x60000000 && p_header[i].p_type<0x6FFFFFFF)
        printf("LOOS-HIOS   ");
    else if(p_header[i].p_type==0x6FFFFFFF)
        printf("HIOS        ");
    else if(p_header[i].p_type==0x70000000)
        printf("LOPROC      ");
    else if(p_header[i].p_type>0x70000000 && p_header[i].p_type<0x7FFFFFFF)
        printf("LPROC-HPROC ");
    else if(p_header[i].p_type==0x7FFFFFFF)
        printf("HIPROC      ");
    printf("%#016x  ",p_header[i].p_offset);
    printf("%#016x  ",p_header[i].p_vaddr);
    printf("%#016x \n",p_header[i].p_paddr);
    printf("            ");
    printf("%#016x  ",p_header[i].p_filesz);
    printf("%#016x  ",p_header[i].p_memsz);
    if(p_header[i].p_flags==0)
        printf("       ");
    else if(p_header[i].p_flags==1)
        printf("  E    ");
    else if(p_header[i].p_flags==2)
        printf(" W     ");
    else if(p_header[i].p_flags==3)
        printf(" WE    ");
    else if(p_header[i].p_flags==4)
        printf("R      ");
    else if(p_header[i].p_flags==5)
        printf("R E    ");
    else if(p_header[i].p_flags==6)
        printf("RW     ");
    else if(p_header[i].p_flags==7)
        printf("RWE    ");
    else printf("Unspec");
    
    printf("%d \n",p_header[i].p_align);
    
}
    printf("End Of Program Headers");
    return 0;
    
    
    
}






















