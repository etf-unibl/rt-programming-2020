#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elf.h>


void program_headers(FILE *fp)
{
	int i, j;
    char flag[4] = {' ', ' ', ' ', ' '};    

    int64_t pheaders_offset;   
    short pheaders_size;      
    short number_of_pheaders; 
    
    fseek(fp, 56, 0);
    fread(&number_of_pheaders, 2, 1, fp);

    fseek(fp, 32, 0);
    fread(&pheaders_offset, 8, 1, fp);

    fseek(fp, 54, 0);
    fread(&pheaders_size, 2, 1, fp);
    
    Elf64_Phdr *pheader=(Elf64_Phdr*) malloc(number_of_pheaders*pheaders_size);
    fseek(fp, pheaders_offset, 0);    
    fread(pheader, pheaders_size, number_of_pheaders, fp);
    fclose(fp);

    printf("Program header:\n");
    printf("\tType\t\tOffset\t VAddr\t    PAddr\tFSize\t MemSize   Flag  Align\n");
    
	for (i = 0; i < number_of_pheaders; i++)
	{   
		printf("\t");
		switch (pheader[i].p_type)
		{
		case 0x00000000:
			printf("NULL\t");
			break;
		case 0x00000001:
			printf("LOAD \t");
			break;
		case 0x00000002:
			printf("DYNAMIC\t");
			break;
		case 0x00000003:
			printf("INTERP\t");
			break;
		case 0x00000004:
			printf("NOTE \t");
			break;
		case 0x00000005:
			printf("SHLIB\t");
			break;
		case 0x00000006:
			printf("PHDR\t");
			break;
		case 0x00000007:
			printf("TLS\t");
        case 0x70000000: 
            printf("LOPROC\t");
            break;
        case 0x7fffffff: 
            printf("HIPROC\t");
            break;
        case 0x60000000: 
            printf("LOOS\t");
            break;
        case 0x6fffffff: 
            printf("HIOS\t");
            break;
        case 1685382480: 
            printf("GNU_EH_FRAME");
            break;       
        case 1685382481: 
            printf("GNU_STACK");
            break;        
        case 1685382482: 
            printf("GNU_RELRO");
            break;
		default:
			printf("Unknown  ");
			break;
        }

        switch(pheader[i].p_flags)
		{
			case 0x01:
				flag[2]='E';
				break;
			case 0x02:
				flag[1]='W';
				break;
			case 0x03:
				flag[1]='W';
				flag[2]='E';
				break;
			case 0x04:
				flag[0]='R';
				break;
			case 0x05:
				flag[0]='R';
				flag[2]='E';
				break;
			case 0x06:
				flag[0]='R';
				flag[1]='W';
				break;
			case 0x07:
				flag[0]='R';
				flag[1]='W';
				flag[2]='E';
				break;
        }
        flag[3]='\0';
		printf("\t0x%06x 0x%08x 0x%08x  0x%06x 0x%06x", 
				pheader[i].p_offset, pheader[i].p_vaddr, 
                pheader[i].p_paddr, pheader[i].p_filesz, 
                pheader[i].p_memsz);
		printf("  %s",flag);
		printf("  0x%x\n",pheader[i].p_align);
	    if(pheader[i].p_type == 3) printf("\t \t [Requesting program interpreter: /lib64/ld-linux-x86-64.so.2]\n");
        for(j = 0; j < 4;++j) flag[j]=' ';
	}
}

//poziv programa:
//  ime prog.   cisto radi izgleda   koristice kao pokazivac na elf FILE
// ./readelf    --program-headers     <ime_programa>
int main(int argc, char *argv[])
{
	char *readelf = "readelf";
	char *phdr_s1 = "--program-headers";
	char *phdr_s2 = "-l";
	if (argc == 1)
	{
		printf("Try to enter   readelf --program-headers elf-file");
		printf("\n          or   readelf -l elf-file\n");
		return -1;
	}
	if (argc == 2)
	{
		if ((strcmp(argv[1], phdr_s1) == 0) || (strcmp(argv[1], phdr_s2) == 0))
		{
			printf("readelf: Warning: Nithing to do.");
			printf("\nUsage: readelf <option(s)> elf-file");
			printf("\nOptions are:");
			printf("\n     -l  --program-headers        Display the program headers\n");
		}
		printf("Incorrect command.\n");
		return -1;
	}
	if (argc == 3)
	{
		if ((strcmp(argv[1], phdr_s1) == 0) || (strcmp(argv[1], phdr_s2) == 0))
		{
			FILE *elf;
			printf("File name: %s\n", argv[2]);
			elf = fopen(argv[2], "rb+");
			if (elf == NULL)
			{
				printf("\nFile %s cannot be opened", argv[2]);
				return -1;
			}
			program_headers(elf);
			fclose(elf);
		}
	}

	return 0;
}
