/*
 * test.c
 * 
 * Copyright 2020 Kali Live user <kali@kali>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */


#include <stdio.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>

typedef struct
{
  uint8_t e_ident[16];		/* Magic number and other info */
  uint16_t e_type;		/* Object file type */
  uint16_t e_machine;		/* Architecture */
  uint32_t e_version;		/* Object file version */
  uint64_t e_entry;		/* Entry point virtual address */
  uint64_t e_phoff;		/* Program header table file offset */
  uint64_t e_shoff;		/* Section header table file offset */
  uint32_t e_flags;		/* Processor-specific flags */
  uint16_t e_ehsize;		/* ELF header size in bytes */
  uint16_t e_phentsize;		/* Program header table entry size */
  uint16_t e_phnum;		/* Program header table entry count */
  uint16_t e_shentsize;		/* Section header table entry size */
  uint16_t e_shnum;		/* Section header table entry count */
  uint16_t e_shstrndx;		/* Section header string table index */
} Elf64Hdr;
Elf64Hdr header;

void
P_e_type ()
{
  if (header.e_type == 0)
    printf ("No file type\n");
  else if (header.e_type == 1)
    printf ("Relocatable file\n");
  else if (header.e_type == 2)
    printf ("Executable file\n");
  else if (header.e_type == 3)
    printf ("Shared object file\n");
  else if (header.e_type == 4)
    printf ("Core file\n");
  else if (header.e_type == 0xfe00)
    printf ("Operating system-specific\n");
  else if (header.e_type == 0xfeff)
    printf ("Operating system-specific\n");
  else if (header.e_type == 0xff00)
    printf ("Processor-specific\n");
  else if (header.e_type == 0xffff)
    printf ("Processor-specific\n");
}

void
P_e_machine ()
{
  switch (header.e_machine)
    {
    case 0:
      printf ("No machine\n");
      break;
    case 1:
      printf ("AT&T WE 32100\n");
      break;
    case 2:
      printf ("SPARC\n");
      break;
    case 3:
      printf ("Intel 80386\n");
      break;
    case 4:
      printf ("Motorola 68000\n");
      break;
    case 5:
      printf ("Motorola 88000\n");
      break;
    case 6:
      printf ("Reserved for future use (was EM_486)\n");
      break;
    case 7:
      printf ("Intel 80860\n");
      break;
    case 8:
      printf ("MIPS I Architecture\n");
      break;
    case 9:
      printf ("IBM System/370 Processor\n");
      break;
    case 10:
      printf ("MIPS RS3000 Little-endian\n");
      break;
    case 11:
      printf ("Reserved for future use\n");
      break;
    case 12:
      printf ("Reserved for future use\n");
      break;
    case 13:
      printf ("Reserved for future use\n");
      break;
    case 14:
      printf ("Reserved for future use\n");
      break;
    case 15:
      printf ("Hewlett-Packard PA-RISC\n");
      break;
    case 16:
      printf ("Reserved for future use\n");
      break;
    case 17:
      printf ("Fujitsu VPP500\n");
      break;
    case 18:
      printf ("Enhanced instruction set SPARC\n");
      break;
    case 19:
      printf ("Intel 80960\n");
      break;
    case 20:
      printf ("PowerPC\n");
      break;
    case 21:
      printf ("64-bit PowerPC\n");
      break;
    case 22:
      printf ("IBM System/390 Processor\n");
      break;
    case 23:
      printf ("Reserved for future use\n");
      break;
    case 24:
      printf ("Reserved for future use\n");
      break;
    case 25:
      printf ("Reserved for future use\n");
      break;
    case 26:
      printf ("Reserved for future use\n");
      break;
    case 27:
      printf ("Reserved for future use\n");
      break;
    case 28:
      printf ("Reserved for future use\n");
      break;
    case 29:
      printf ("Reserved for future use\n");
      break;
    case 30:
      printf ("Reserved for future use\n");
      break;
    case 31:
      printf ("Reserved for future use\n");
      break;
    case 32:
      printf ("Reserved for future use\n");
      break;
    case 33:
      printf ("Reserved for future use\n");
      break;
    case 34:
      printf ("Reserved for future use\n");
      break;
    case 35:
      printf ("Reserved for future use\n");
      break;
    case 36:
      printf ("NEC V800\n");
      break;
    case 37:
      printf ("Fujitsu FR20\n");
      break;
    case 38:
      printf ("TRW RH-32\n");
      break;
    case 39:
      printf ("Motorola RCE\n");
      break;
    case 40:
      printf ("Advanced RISC Machines ARM\n");
      break;
    case 41:
      printf ("Digital Alpha\n");
      break;
    case 42:
      printf ("Hitachi SH\n");
      break;
    case 43:
      printf ("SPARC Version 9\n");
      break;
    case 44:
      printf ("Siemens TriCore embedded processor\n");
      break;
    case 45:
      printf ("Argonaut RISC Core, Argonaut Technologies Inc.\n");
      break;
    case 46:
      printf ("Hitachi H8/300\n");
      break;
    case 47:
      printf ("Hitachi H8/300H\n");
      break;
    case 48:
      printf ("Hitachi H8S\n");
      break;
    case 49:
      printf ("Hitachi H8/500\n");
      break;
    case 50:
      printf ("Intel IA-64 processor architecture\n");
      break;
    case 51:
      printf ("Stanford MIPS-X\n");
      break;
    case 52:
      printf ("Motorola ColdFire\n");
      break;
    case 53:
      printf ("Motorola M68HC12\n");
      break;
    case 54:
      printf ("Fujitsu MMA Multimedia Accelerator\n");
      break;
    case 55:
      printf ("Siemens PCP\n");
      break;
    case 56:
      printf ("Sony nCPU embedded RISC processor\n");
      break;
    case 57:
      printf ("Denso NDR1 microprocessor\n");
      break;
    case 58:
      printf ("Motorola Star*Core processor\n");
      break;
    case 59:
      printf ("Toyota ME16 processor\n");
      break;
    case 60:
      printf ("STMicroelectronics ST100 processor\n");
      break;
    case 61:
      printf ("Advanced Logic Corp. TinyJ embedded processor family\n");
      break;
    case 62:
      printf ("AMD x86-64 architecture\n");
      break;
    case 63:
      printf ("Sony DSP Processor\n");
      break;
    case 64:
      printf ("Digital Equipment Corp. PDP-10\n");
      break;
    case 65:
      printf ("Digital Equipment Corp. PDP-11\n");
      break;
    case 66:
      printf ("Siemens FX66 microcontroller\n");
      break;
    case 67:
      printf ("STMicroelectronics ST9+ 8/16 bit microcontroller\n");
      break;
    case 68:
      printf ("STMicroelectronics ST7 8-bit microcontroller\n");
      break;
    case 69:
      printf ("Motorola MC68HC16 Microcontroller\n");
      break;
    case 70:
      printf ("Motorola MC68HC11 Microcontroller\n");
      break;
    case 71:
      printf ("Motorola MC68HC08 Microcontroller\n");
      break;
    case 72:
      printf ("Motorola MC68HC05 Microcontroller\n");
      break;
    case 73:
      printf ("Silicon Graphics SVx\n");
      break;
    case 74:
      printf ("STMicroelectronics ST19 8-bit microcontroller\n");
      break;
    case 75:
      printf ("Digital VAX\n");
      break;
    case 76:
      printf ("Axis Communications 32-bit embedded processor\n");
      break;
    case 77:
      printf ("Infineon Technologies 32-bit embedded processor\n");
      break;
    case 78:
      printf ("Element 14 64-bit DSP Processor\n");
      break;
    case 79:
      printf ("LSI Logic 16-bit DSP Processor\n");
      break;
    case 80:
      printf ("Donald Knuth's educational 64-bit processor\n");
      break;
    case 81:
      printf ("Harvard University machine-independent object files\n");
      break;
    case 82:
      printf ("SiTera Prism\n");
      break;
    case 83:
      printf ("Atmel AVR 8-bit microcontroller\n");
      break;
    case 84:
      printf ("Fujitsu FR30\n");
      break;
    case 85:
      printf ("Mitsubishi D10V\n");
      break;
    case 86:
      printf ("Mitsubishi D30V\n");
      break;
    case 87:
      printf ("NEC v850\n");
      break;
    case 88:
      printf ("Mitsubishi M32R\n");
      break;
    case 89:
      printf ("Matsushita MN10300\n");
      break;
    case 90:
      printf ("Matsushita MN10200\n");
      break;
    case 91:
      printf ("picoJava\n");
      break;
    case 92:
      printf ("OpenRISC 32-bit embedded processor\n");
      break;
    case 93:
      printf ("ARC Cores Tangent-A5\n");
      break;
    case 94:
      printf ("Tensilica Xtensa Architecture\n");
      break;
    case 95:
      printf ("Alphamosaic VideoCore processor\n");
      break;
    case 96:
      printf ("Thompson Multimedia General Purpose Processor\n");
      break;
    case 97:
      printf ("National Semiconductor 32000 series\n");
      break;
    case 98:
      printf ("Tenor Network TPC processor\n");
      break;
    case 99:
      printf ("Trebia SNP 1000 processor\n");
      break;
    case 100:
      printf ("STMicroelectronics (www.st.com) ST200 microcontroller\n");
      break;
    }
}

void
P_e_version ()
{
  if (header.e_version == 0)
    printf ("0 Invalid version\n");
  else if (header.e_version == 1)
    printf ("1 Current version\n");
}

void
P_e_entry ()
{
  printf ("Ox" "%" PRIx64 "\n", header.e_entry);
}

void
P_e_phoff ()
{
  printf ("%" PRId64 " (bytes into file)\n", header.e_phoff);
}

void
P_e_shoff ()
{
  printf ("%" PRId64 " (bytes into file)\n", header.e_shoff);
}

void
P_e_flags ()
{
  printf ("Ox" "%" PRIx32 "\n", header.e_flags);
}

void
P_e_ehsize ()
{
  printf ("%" PRId16 " (bytes)\n", header.e_ehsize);
}

void
P_e_phentsize ()
{
  printf ("%" PRId16 " (bytes)\n", header.e_phentsize);
}

void
P_e_phnum ()
{
  printf ("%" PRId16 "\n", header.e_phnum);
}

void
P_e_shentsize ()
{
  printf ("%" PRId16 " (bytes)\n", header.e_shentsize);
}

void
P_e_shnum ()
{
  printf ("%" PRId16 "\n", header.e_shnum);
}

void
P_e_shstrndx ()
{
  printf ("%" PRId16 "\n", header.e_shstrndx);
}

void
P_e_ident_MAGIC ()
{
  for (int i = 0; i < 15; i++)
    {
      printf ("%" PRIx8, header.e_ident[i]);
      printf (" ");
    }
  printf ("%" PRIx8 "\n", header.e_ident[15]);
}

void
P_e_ident_CLASS ()
{
  switch (header.e_ident[4])
    {
    case 0:
      printf ("Invalid class\n");
      break;
    case 1:
      printf ("32-bit objects\n");
      break;
    case 2:
      printf ("64-bit objects\n");
      break;
    }
}

void
P_e_ident_DATA ()
{
  switch (header.e_ident[5])
    {
    case 0:
      printf ("Invalid data encoding\n");
      break;
    case 1:
      printf ("ELFDATA2LSB 2's complement values little endian\n");
      break;
    case 2:
      printf ("ELFDATA2MSB 2's complement values big endian\n");
      break;
    }
}

void
P_e_ident_VERSION ()
{
  printf ("Ox" "%" PRId8 "\n", header.e_ident[6]);
}

void
P_e_ident_OSABI ()
{
  switch (header.e_ident[7])
    {
    case 0:
      printf ("UNIX - System V\n");
      break;
    case 1:
      printf ("Hewlett-Packard HP-UX\n");
      break;
    case 2:
      printf ("NetBSD\n");
      break;
    case 3:
      printf ("Linux\n");
      break;
    case 6:
      printf ("Sun Solaris\n");
      break;
    case 7:
      printf ("AIX\n");
      break;
    case 8:
      printf ("IRIX\n");
      break;
    case 9:
      printf ("FreeBSD\n");
      break;
    case 10:
      printf ("Compaq TRU64 UNIX\n");
      break;
    case 11:
      printf ("Novell Modesto\n");
      break;
    case 12:
      printf ("Open BSD\n");
      break;
    case 13:
      printf ("Open VMS\n");
      break;
    case 14:
      printf ("Hewlett-Packard Non-Stop Kernel\n");
      break;
    }
}

void
P_e_ident_ABIVERSION ()
{
  printf ("%" PRId8 "\n", header.e_ident[8]);
}

void
P_e_ident_PAD ()
{
  printf ("%" PRIx8 "\n", header.e_ident[9]);
}

void
P_e_ident_NIDENT ()
{
  printf ("%" PRId8 "\n", header.e_ident[16]);
}

void
read_elf_header (const char *Ifile)
{

  printf ("open file\n");
  FILE *file = fopen (Ifile, "rb");
  if (file)
    {
      fread (&header, 1, sizeof (header), file);
      if (header.e_ident[0] == 0x7f &&
	  header.e_ident[1] == 'E' &&
	  header.e_ident[2] == 'L' && header.e_ident[3] == 'F')
	{
	  printf ("MAGIC_NUM..................");
	  P_e_ident_MAGIC ();
	  printf ("CLASS......................");
	  P_e_ident_CLASS ();
	  printf ("DATA.......................");
	  P_e_ident_DATA ();
	  printf ("VERSION....................");
	  P_e_version ();
	  printf ("OS/ABI.....................");
	  P_e_ident_OSABI ();
	  printf ("ABI VERSION................");
	  P_e_ident_ABIVERSION ();
	  printf ("PAD........................");
	  P_e_ident_PAD ();
	  printf ("NIDENT.....................");
	  P_e_ident_NIDENT ();
	  printf ("TYPE.......................");
	  P_e_type ();
	  printf ("MACHINE....................");
	  P_e_machine ();
	  printf ("VERSION....................");
	  P_e_ident_VERSION ();
	  printf ("ENTRY......................");
	  P_e_entry ();
	  printf ("PHOFF......................");
	  P_e_phoff ();
	  printf ("SHOFF......................");
	  P_e_shoff ();
	  printf ("FLAGS......................");
	  P_e_flags ();
	  printf ("EHSIZE.....................");
	  P_e_ehsize ();
	  printf ("PHENTSIZE..................");
	  P_e_phentsize ();
	  printf ("PHNUM......................");
	  P_e_phnum ();
	  printf ("SHENTSIZE..................");
	  P_e_shentsize ();
	  printf ("SHNUM......................");
	  P_e_shnum ();
	  printf ("SHSTRNDX...................");
	  P_e_shstrndx ();
	}
      else
	printf ("Given file is not an Elf file\n");
      fclose (file);
    }
}

int
main (int argc, char **argv)
{
  char file[50];
  printf ("Enter the file name or file path: ");
  scanf ("%s", file);
  read_elf_header (file);

  return 0;
}
