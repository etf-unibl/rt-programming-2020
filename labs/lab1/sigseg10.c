/*
 * sigseg10.c
 * 
 * Copyright 2020 popic <popic@RTRKN524>
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
#include <stdlib.h>
#include <string.h>

char* funkcija()
{
    char *c= (char*)calloc(1,sizeof(char));
    *c = 'x';
    return c;
}

int main(int argc, char **argv)
{
	char * pok = funkcija();
	char niz[9];
    memset(niz, 'y', sizeof(niz));
    niz[0] = *pok;
    printf("%s\n",niz);
    return 0;
}

