/* 
* Dejan MIlojica 1150/16
* Section Header
*/

#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>

# define EI_NIDENT 16

/* Dodjeljivanje novih imena tipovima!*/
typedef uint64_t Elf64_Addr;  
typedef uint64_t Elf64_Off;
typedef uint32_t Elf64_Word;
typedef uint16_t Elf64_Half;
typedef uint64_t Elf64_Xword;

/* ELF File Header */
/* Neophodan za odredjivanje offseta SH, velicine, kao i pristup String tabeli! */
typedef struct
{
  unsigned char e_ident[EI_NIDENT];     /* Magic number and other info */
  Elf64_Half    e_type;                 /* Object file type */
  Elf64_Half    e_machine;              /* Architecture */
  Elf64_Word    e_version;              /* Object file version */
  Elf64_Addr    e_entry;                /* Entry point virtual address */
  Elf64_Off     e_phoff;                /* Program header table file offset */
  Elf64_Off     e_shoff;                /* Section header table file offset */
  Elf64_Word    e_flags;                /* Processor-specific flags */
  Elf64_Half    e_ehsize;               /* ELF header size in bytes */
  Elf64_Half    e_phentsize;            /* Program header table entry size */
  Elf64_Half    e_phnum;                /* Program header table entry count */
  Elf64_Half    e_shentsize;            /* Section header table entry size */
  Elf64_Half    e_shnum;                /* Section header table entry count */
  Elf64_Half    e_shstrndx;             /* Section header string table index */
} Elf64_Ehdr;

/* Section header.  */
typedef struct
{
  Elf64_Word     sh_name;                /* Section name (string tbl index) */
  Elf64_Word     sh_type;                /* Section type */
  Elf64_Xword    sh_flags;               /* Section flags */
  Elf64_Addr     sh_addr;                /* Section virtual addr at execution */
  Elf64_Off      sh_offset;              /* Section file offset */
  Elf64_Xword    sh_size;                /* Section size in bytes */
  Elf64_Word     sh_link;                /* Link to another section */
  Elf64_Word     sh_info;                /* Additional section information */
  Elf64_Xword    sh_addralign;           /* Section alignment */
  Elf64_Xword    sh_entsize;             /* Entry size if section holds table */
} Elf64_Shdr;

/* String table structure. */
typedef struct{
	unsigned int index;
	char name[15];
}String_Table;


/* Deklaracija f-ja */
void ispis_preuzetih_podataka(Elf64_Shdr sh, char* naziv);
Elf64_Ehdr citanje_file_headera(FILE *ptr);
void obrada_section_headera(FILE *ptr, String_Table *string_table, int i);
void ispis_neophodnog_zaglavlja(int shnum,unsigned long shoff);
String_Table* string_table_analiza(Elf64_Ehdr file_header, FILE *ptr);
void ispis_flag_objasnjenja();

/* Staticka pr. definise broj sekcija */
static int N;

int main(){
	int i;
	/* Izbor izvrsne datoteke za analizu */
	char izvrsna_datoteka[15];
	printf("Unesite naziv izvrsne datoteke: ");
	scanf("%s",izvrsna_datoteka);
	//gets(izvrsna_datoteka);
	printf("I.D.: %s\n",izvrsna_datoteka);

	FILE *ptr = NULL;
	ptr = fopen(izvrsna_datoteka,"rb");

	if(ptr!=NULL){
		Elf64_Ehdr file_header;

		//Citanje file headera, neophodnog za section header!
		file_header = citanje_file_headera(ptr); 
		fseek(ptr,file_header.e_shoff,SEEK_SET); //Pozicioniranje na pocetak Section Header-a!
		//Ispis zaglavlja:
		ispis_neophodnog_zaglavlja(file_header.e_shnum, file_header.e_shoff);
		
		/* STRING TABLE ANALIZA */
		String_Table* string_table;
		string_table = string_table_analiza(file_header,ptr);

		/* Obrada section H.*/
		for(int i=0;i<file_header.e_shnum;i++)
			obrada_section_headera(ptr,string_table,i);		
		
		//Ispis neophodnih podataka, koji definisu flag skracenice.
		ispis_flag_objasnjenja();

		fclose(ptr);
	}else
		printf("Nepostojeca izvrsna datoteka!\n");

return 0;
}

/* Ucitavanje sekcije, te njena analiza (formatiranje) */
void obrada_section_headera(FILE *ptr,String_Table *string_table,int i){
	Elf64_Shdr section_header;

	/* Citanje section headera */
	fread(&section_header,sizeof(section_header),1,ptr); 
	printf("  [ %d]",i);

	/* Prolazak kroz string tabelu, pretraga naziva na osnovu indexa, te formatirani ispis sekcije */
	for(int i=0;i<N;i++)
		if(string_table[i].index == section_header.sh_name){
			ispis_preuzetih_podataka(section_header,string_table[i].name);
			break;		
		}
		else if(i==N-1)
			ispis_preuzetih_podataka(section_header,"no_name");
}

/* Preuzimanje string tabele, formiranje elemenata strukture */
String_Table* string_table_analiza(Elf64_Ehdr file_header, FILE *ptr){
	String_Table* string_tabela;
	unsigned long offset =file_header.e_shstrndx*file_header.e_shentsize+file_header.e_shoff;
	unsigned long pomjeraj = ftell(ptr);
	Elf64_Shdr section_header;


	fseek(ptr,offset,SEEK_SET); //Pozicioniranje na pocetak STRTBL!		
	fread(&section_header,sizeof(section_header),1,ptr); 

	fseek(ptr,section_header.sh_offset,SEEK_SET); //Pozicioniranje na dio za citanje naziva!
	/* Brojimo koliko je neophodno alocirati memorije. */
	char znak;
	for(int i=1;i<=section_header.sh_size;i++){
		fread(&znak,sizeof(znak),1,ptr);
		if(znak=='\0') N++;
	}
	string_tabela = (String_Table*)malloc(sizeof(String_Table)*N);
	int n=-1, k=0;
	
	fseek(ptr,section_header.sh_offset,SEEK_SET); //Pozicioniranje na dio za citanje naziva!
	for(int i=1;i<=section_header.sh_size;i++){
		fread(&znak,sizeof(znak),1,ptr);
		if(znak=='\0'){
			n++;k=0;
			string_tabela[n].index = i;
		}else
			string_tabela[n].name[k++] = znak;
	}
	fseek(ptr,pomjeraj,SEEK_SET); //Pozicioniranje na staru vrijednost!	
return string_tabela;	   			
}

/* Preuzimanje File Headera, neophodan za pristup SH-u */
Elf64_Ehdr citanje_file_headera(FILE *ptr){
	Elf64_Ehdr file_header;
	fread(&file_header,sizeof(file_header),1,ptr); 
	return file_header;
}

/* Ispis uvodnog zaglavlja */
void ispis_neophodnog_zaglavlja(int shnum,unsigned long shoff){
	printf("There are %d section headers, starting at offset 0x%lx:\n",shnum,shoff);
	printf("\n");
	printf("Section Headers:\n");
	printf("  [Nr] Name                     Type                Address             Offset\n");
    printf("       Size                     EntSize             Flags  Link  Info   Align\n");
}

/* Ispis zavrsnog zaglavlja */
void ispis_flag_objasnjenja(){
	printf("Key to Flags:\n"); 
	printf(" W (write), A (alloc), X (execute), M (merge), S (strings), l (large)\n");
  	printf(" I (info), L (link order), G (group), T (TLS), E (exclude), x (unknown)\n");
  	printf(" O (extra OS processing required) o (OS specific), p (processor specific)\n");
}

/* Formatirani ispis sekcije */
void ispis_preuzetih_podataka(Elf64_Shdr sh, char* naziv){
	/* Name */
	printf(" %-25s",naziv);

	/* Type */
	switch(sh.sh_type){
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
		case 14:
			printf("%-20s","INIT_ARRAY");
			break;
		case 15:
			printf("%-20s","FINI_ARRAY");
			break;
		case 0x6FFFFFFF:
			printf("%-20s","VERNEED");
			break;
		case 0x6FFFFFF6:
			printf("%-20s","GNU_HASH");
			break;
		case 0x6FFFFFFE:
			printf("%-20s","VERSYM");
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
			printf("%-20s","No_Type");
	}

	/* Adresa */
	printf("%016lx    ",sh.sh_addr);

	/* Offset */
	printf("%08lx\n",sh.sh_offset);

	/* Size */
	printf("\t%016lx",sh.sh_size);

	/* Ent. Size */
	printf("\t%016lx",sh.sh_entsize);
	
	/* Flags */
	switch(sh.sh_flags){
		case 1:
			printf("%6s","W");
			break;
		case 2:
			printf("%6s","A");
			break;
		case 4:
			printf("%6s","E");
			break;
		case 3:
			printf("%7s","WA");
			break;
		case 5:
			printf("%7s","WE");
			break;
		case 6:
			printf("%7s","AX");
			break;
		case 0xf0000000:
			printf("%6s","M");
			break;
		default:
			printf("%6s"," ");
	}

	/* Link */
	printf("%8d",sh.sh_link);

	/* Info */
	printf("%5d",sh.sh_info);

	/* Align */
	printf("%10lu\n",sh.sh_addralign);
}
