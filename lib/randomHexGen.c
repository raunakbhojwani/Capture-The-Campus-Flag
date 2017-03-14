/*	randomHexGen.c a function to generate random hexcodes (with a specified input length)

	Project name: Project Incomputable
	Component name: randomHexGen.c

	Primary Author:	Max Zhuang, Raunak Bhojwani, Samuel Ching
	Date Created: 5/30/16

	Special considerations:  
		(e.g., special compilation options, platform limitations, etc.) 
	
======================================================================*/

/** Global Include **/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

/** Local Includes **/
#include "../lib/memory.h"


/****** gen_random_hexcode(num) ******/

char* gen_random_hexcode(int num)
{
	char* randomHex = assertp(count_calloc(num+100, sizeof(char)), "Memory allocation for randomHex failed.\n");
	char* hexArray = "0123456789ABCDEF";
	
	for (int i = 0; i < num; i++){
		int randomNum = rand()%16;
		char randomVal = hexArray[randomNum];
		sprintf(randomHex, "%s%c", randomHex,randomVal);
	}

	return randomHex;

}