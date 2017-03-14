/*	codedrop.c - a module for initializing, accessing and interacting with the codedrop struct

	Project name: Project Incomputable 
	Component name: Lib/codedrop.c
	
	Primary Author:	Max Zhuang, Raunak Bhojwani and Samuel Ching
	Date Created: 5/24/16	

======================================================================*/
// do not remove any of these sections, even if they are empty
//
// ---------------- Open Issues 

// ---------------- System includes e.g., <stdio.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h> 

// ---------------- Local includes  e.g., "file.h"
#include "memory.h"


// ---------------- Constant definitions

// ---------------- Macro definitions

// ---------------- Structures/Types 
typedef struct codedrop {
	float codeLat;
	float codeLong;
	char* neutralizingTeam;
	char* codeID;
	char* codeStatus;
 } codedrop_t;

// ---------------- Private variables 

// ---------------- Private prototypes 

/************************************************/


/**codedrop_new()**/

codedrop_t* codedrop_new(float codeLat, float codeLong, char* codeID, char* codeStatus)
{
	codedrop_t* newCodeDrop = assertp(count_calloc(1, sizeof(codedrop_t)), "Memory allocation for new codedrop failed.\n");
	char* codeStatusCpy = assertp(count_calloc(strlen(codeStatus)*2+100, sizeof(char)), "Memory allocation for codeStatus copy failed.\n");
	char* codeIDCpy = assertp(count_calloc(strlen(codeID)+100, sizeof(char)), "Memory allocation for codeID copy failed.\n");

	strcpy(codeStatusCpy,codeStatus);
	strcpy(codeIDCpy, codeID);

	newCodeDrop->codeLat = codeLat;
	newCodeDrop->codeLong = codeLong;
	newCodeDrop->neutralizingTeam = NULL;
	newCodeDrop->codeID = codeIDCpy;
	newCodeDrop->codeStatus = codeStatusCpy;

	printf("Code Lat:%f\tCode Long:%f\tCodeID:%s\tCode Status:%s\n\n",newCodeDrop->codeLat, newCodeDrop->codeLong, newCodeDrop->codeID, newCodeDrop->codeStatus);

	return newCodeDrop;
}

/**update_code_status()**/

int update_code_status(codedrop_t* codedrop, char* neutralizingTeam, char* codeStatus)
{
	if (codedrop == NULL){
		fprintf(stderr, "Codedrop does not exist_update\n");
		return 1;
	}

	char* newNeutralizingT = assertp(count_calloc(strlen(neutralizingTeam)+100, sizeof(char)), "Memory allocation for neutralizing team failed.\n");
	strcpy(newNeutralizingT,neutralizingTeam);

	if (codedrop->neutralizingTeam == NULL){
		codedrop->neutralizingTeam = newNeutralizingT;
	}

	if (codedrop->codeStatus != NULL)
		count_free(codedrop->codeStatus);

	char* codeStatusCpy = assertp(count_calloc(strlen(codeStatus)+100, sizeof(char)), "Memory allocation for neutralizing team failed.\n");
	strcpy(codeStatusCpy,codeStatus);

	codedrop->codeStatus = codeStatusCpy;
	return 0;
}


void codedrop_del(codedrop_t* codedrop)
{
	if (codedrop != NULL){

	if (codedrop->neutralizingTeam != NULL){
		count_free(codedrop->neutralizingTeam);
	}

	if (codedrop->codeID != NULL)
		count_free(codedrop->codeID);
	
	if (codedrop->codeStatus != NULL)
		count_free(codedrop->codeStatus);
	
	count_free(codedrop);

	}
}

char* get_code_status(codedrop_t* codedrop)
{
	if (codedrop == NULL){
		fprintf(stderr, "Codedrop does not exist_status\n");
		return NULL;
	}

	return codedrop->codeStatus;
}


float get_code_lat(codedrop_t* codedrop)
{
	if (codedrop == NULL){
		fprintf(stderr, "Codedrop does not exist_lat\n");
		return 1;
	}

	return codedrop->codeLat;	
}


float get_code_long(codedrop_t* codedrop)
{
	if (codedrop == NULL){
		fprintf(stderr, "Codedrop does not exist_long\n");
		return 1;
	}

	return codedrop->codeLong;
}


char* get_neuteam(codedrop_t* codedrop)
{
	if (codedrop == NULL){
		fprintf(stderr, "Codedrop does not exist_neut\n");
		return NULL;
	}

	return codedrop->neutralizingTeam;	
}

