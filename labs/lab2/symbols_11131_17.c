#include <stdio.h>
#include <stdlib.h>
#include <elf.h>
#include <string.h>
#include <libelf.h>


int counter2=0;
int counter11=0;
long position_table;
int first2=1;
int first11=1;

FILE* file;

void read_elfh_32(FILE*);
void read_symbols_64(FILE*);
void print_sym(int,Elf64_Sym);

Elf64_Sym sym;
Elf64_Ehdr header;
Elf64_Shdr section_header;
Elf64_Shdr section_header_temp;


int main(int argc, char* argv[])
{

    if (argc < 2) {
        printf("Nije dobar broj argumentata komandle linije\n");
        return -1;
    }
    file = fopen(argv[1], "rb");
    char architecture[5];

    if(file != NULL)
    {
        fread(&architecture, 1, 5, file);
        fseek(file, 0, 0);

        if(architecture[4] == 1)
            printf(" arhitektrura je 32bitna!\n");
        else if(architecture[4] == 2)
        {
            printf(" arhitektura je 64bit\n");
            read_symbols_64(file);
        }
        fclose(file);
    }
    else
        printf("Nije moguce otvoriti fajl\n");
    return 0;
}
void print_sym_header11()
{
    printf("   Symbol table '.dynsym'\n");
    printf("Num        Value            Size       Type      Bind       Vis     Ndx        Name   \n");
}
void print_sym_header2()
{
    printf("   Symbol table '.symtab'\n");
    printf("Num        Value            Size       Type      Bind       Vis     Ndx        Name   \n");
}
void print_sym(int counter,Elf64_Sym sym)
{
    char type[20];

    char temp=sym.st_info;
    temp=temp&0b00001111;
    switch(temp) {
    case 0:
        strcpy(type,"LOCAL");
        break;
    case 1:
        strcpy(type,"GLOBAL");
        break;
    case 2:
        strcpy(type,"WEAK");
        break;
    case 10:
        strcpy(type,"LOODS");
        break;
    case 12:
        strcpy(type,"HIOS");
        break;
    case 13:
        strcpy(type,"LOPROC");
        break;
    case 15:
        strcpy(type,"HIPROC");
        break;
    }
    char bind[20];
    temp=sym.st_info;
    temp=temp&0b11110000;
    temp=temp>>4;
    switch(temp) {
    case 0:
        strcpy(bind,"NOTYPE");
        break;
    case 1:
        strcpy(bind,"OBJECT");
        break;
    case 2:
        strcpy(bind,"FUNC");
        break;
    case 3:
        strcpy(bind,"SECTION");
        break;
    }
    char vis[20];
    temp=sym.st_other;
    switch(temp) {
    case 0:
        strcpy(vis,"DEFAULT");
        break;
    case 1:
        strcpy(vis,"INTERNAL");
        break;
    case 2:
        strcpy(vis,"HIDDEN");
        break;
    case 3:
        strcpy(vis,"PROTECTED");
        break;
    }
    char name[50],c;
    int j=0;
    long temp_position=ftell(file);
    fseek(file,header.e_shoff,0);//prvi section_header
    fseek(file,position_table*sizeof(Elf64_Shdr),1);
    fread(&section_header_temp,1,sizeof(Elf64_Shdr),file);
    fseek(file,section_header_temp.sh_offset+sym.st_name,0);

    do {      //citanje imena iz string tabele
        fread(&c,1,sizeof(char),file);
        name[j++]=c;
        if(j>50)
            break;
    } while(c!=0);
    name[j]=0;
    fseek(file,temp_position,0);
    if(sym.st_shndx!=0)
        printf(" %3d %016lx %10ld %10s %10s %10s %5d %s\n",counter, sym.st_value, sym.st_size, bind, type, vis, sym.st_shndx, name);
    else
        printf(" %3d %016lx %10ld %10s %10s %10s   UND %s\n",counter, sym.st_value, sym.st_size, bind, type, vis, name);
}

void read_symbols_64(FILE* file)
{

    fread(&header,sizeof(header),1,file);
    Elf64_Off  section_header_offset =header.e_shoff;
    fseek(file,section_header_offset,0);  // pozicioniram se na pocetak secion headera
    fread(&section_header,sizeof(section_header),1,file); // procitam prvi section_header i pozicionira se na kraj prvog section_headera
    long position=ftell(file);
    for( int i=1; i<header.e_shnum; i++)
    {
        Elf64_Word type=section_header.sh_type;
        if(type ==2||type==11) // symtab ili dynsym
        {
            if(type==2&&first2)
            {
                print_sym_header2();
                first2=0;
            }
            if(type==11&&first11)
            {
                print_sym_header11();
                first11=0;
            }
            double size=section_header.sh_size/sizeof(Elf64_Sym);
            position_table=section_header.sh_link;// pozicija zbog linka
            fseek(file,section_header.sh_offset,0);

            for(int i=0; i<size; i++) {
                fread(&sym,sizeof(sym),1,file);

                if(type==2)
                    print_sym(counter2++,sym);
                else
                    print_sym(counter11++,sym);   // ispis symbols
            }
            fseek(file,position,0); // na prethodni kraj section_header-a
            fread(&section_header,sizeof(section_header),1,file);
        }
        else {
            fread(&section_header,sizeof(section_header),1,file);
            position=ftell(file);
        }
    }
}
