/*
 * sigseg16.c
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

int add_numbers(int n1, int n2)
{
     int sum=n1+n2;
     return sum;
}

int suma;

int main()
{
	 static char *poruka = "Suma brojeva 1 i 2 je";
     int n1=1;
     int n2;
     
     suma=add_numbers(n1,n2);
     printf("%s %d\n", poruka, suma);

     return 0;
}
