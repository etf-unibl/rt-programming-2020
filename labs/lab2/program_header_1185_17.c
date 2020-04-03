#include <stdio.h>
#include <elf.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#define ELF_FILE "mmap"

#define e_phoff_offset  32  /* Pomeraj e_phoff u ELF headeru. e_phoff sadrzi informaciju o poziciji program headera u Elf fajlu. */
#define e_phentsize_offset 54
#define e_phnum_offset 56

#define e_shoff_offset  40
#define e_shentsize_offset  58
#define e_shnum_offset  60

#define e_shstrndx_offset 62

char *typeOfHeader(int64_t n)
{
    char *name=(char*)malloc(15);
    switch(n)
    {
        case 0: name="NULL";
            break;
        case 1: name="LOAD";
            break;
        case 2: name="DYNAMIC";
            break;
        case 3: name="INTERP";
            break;
        case 4: name="NOTE";
            break;
        case 5: name="SHLIB";
            break;
        case 6: name="PHDR";
            break;
        case 7: name="TLS";
            break;
        case 0x70000000: name="LOPROC";
            break;
        case 0x7fffffff: name="HIPROC";
            break;
        case 0x60000000: name="LOOS";
            break;
        case 0x6fffffff: name="HIOS";
            break;
        case 1685382480: name="GNU_EH_FRAME";
            break;       
        case 1685382481: name="GNU_STACK";
            break;        
        case 1685382482: name="GNU_RELRO";
            break;
        default: name=NULL;
    }
    return name;
}

char *flags(int n)
{
    char *str = (char*) malloc(3);
    switch(n)
    {
        case 0: str="   ";
            break;
        case 1: str="  E";
            break;
        case 2: str=" W ";
            break;
        case 3: str=" WE";
            break;
        case 4: str="R  ";
            break;
        case 5: str="R E";
            break;
        case 6: str="RW ";
            break;
        case 7: str="RWE";
            break;
        default: str=NULL;
    }
    return str;
}


int main()
{
    int64_t e_phoff;        /*  Program  header  offset  */
    short e_phentsize;      /*  Size  of  program  header  entry  */
    short e_phnum;          /*  Number  of  program  header  entries  */

    int64_t e_shoff;        /*  Section  header  offset  */
    short e_shentsize;      /*  Size  of  section  header  entry  */
    short e_shnum;          /*  Number  of  section  header  entries  */

    short e_shstrndx;       /*  Section name string table index.  */
    FILE *fp;
    if((fp=fopen(ELF_FILE,"r"))==NULL)
    {
        printf("Datoteka nije otvorena.\n");
    }else
    {

        fseek(fp,e_phoff_offset,0);
        fread(&e_phoff,8,1,fp);
        fseek(fp,e_phentsize_offset,0);
        fread(&e_phentsize,2,1,fp);
        fseek(fp,e_phnum_offset,0);
        fread(&e_phnum,2,1,fp);
        
        fseek(fp,e_shoff_offset,0);
        fread(&e_shoff,8,1,fp);
        fseek(fp,e_shentsize_offset,0);
        fread(&e_shentsize,2,1,fp);
        fseek(fp,e_shnum_offset,0);
        fread(&e_shnum,2,1,fp);

        fseek(fp,e_shstrndx_offset,0);
        fread(&e_shstrndx,2,1,fp);

        Elf64_Phdr *arr=(Elf64_Phdr*) malloc(e_phnum*e_phentsize);
        Elf64_Shdr *sarr=(Elf64_Shdr*) malloc(e_shnum*e_shentsize);

        fseek(fp,e_phoff,0);    
        fread(arr,e_phentsize,e_phnum,fp);

        fseek(fp,e_shoff,0);
        fread(sarr,e_shentsize,e_shnum,fp);

   
      //  fclose(fp);
    
    /*Ispis*/
    printf("There are %d program headers, starting at offset %ld \n",e_phnum,e_phoff);
    printf("Program Headers:\n    %-*s",16,"Type");
    printf("%-*s",20,"Offset");
    printf("%-*s",20,"VirtAddr");
    printf("%-*s\n",16,"PhysAddr");
    printf("%-20s%s","","FileSiz");
    printf("%*s",19,"MemSiz");
    printf("%*s     %s\n",20," Flags", "Align");
    for(int i=0; i<e_phnum; i++)
    {
        printf("    %-*s",16,typeOfHeader(arr[i].p_type));
        printf("0x%016lx  ",arr[i].p_offset);
        printf("0x%016lx  ",arr[i].p_vaddr);
        printf("0x%016lx\n",arr[i].p_paddr);
        printf("%-20s0x%016lx  ","",arr[i].p_filesz);
        printf("0x%016lx  ",arr[i].p_memsz);
        printf("%-8s",flags(arr[i].p_flags));
        printf("0x%lx\n",arr[i].p_align);
        if(arr[i].p_type==3)
            printf("        [Requesting program interpreter: /lib64/ld-linux-x86-64.so.2]\n");
    }
    char k;
    int64_t offsetStringSekcije=sarr[e_shstrndx].sh_offset;

    printf("    Section to Segment mapping:\n    Segment Sections...\n");
    for(int j=0; j<e_phnum; j++)
    {
        printf("    %d  ",j);
        for(int i=0; i<e_shnum; i++)
        {
            if(sarr[i].sh_offset>=arr[j].p_offset && (sarr[i].sh_offset+sarr[i].sh_size)<=(arr[j].p_offset+arr[j].p_filesz))
            {   
                fseek(fp,offsetStringSekcije+sarr[i].sh_name,0);
                while ((k = fgetc(fp)) != 0) 
                printf("%c", k);
                printf("  ");
            }            
           
        }
        printf("\n");
    }
    fclose(fp);
    }
    return 0;
}
