/*
 * sigseg1.c
 * 
 * Copyright 2019 popic <popic@RTRKN524>
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
 * Pronaci gresku pomocu gdb
 * 
 * 
 */
 #include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
 
int main(int argc, char* argv[]){
	if (argc>1)
	{
   char* name = malloc(strlen(argv[1])) ;
   name = strcpy(name,argv[1]);
   printf("%s \n", name);
	}
	else printf("Nije unijeto dovljno argumenata");
   return EXIT_SUCCESS;
 } 
