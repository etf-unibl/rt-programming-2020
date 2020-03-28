/*
 * sigseg15.c
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
#include <malloc.h> 
int main(void) 
{ 
    int* p = (int*)malloc(8); 
    *p = 100; 
    *(p+1) = 110; 
    free(p);
	p = NULL; 
    return 0; 
} 
