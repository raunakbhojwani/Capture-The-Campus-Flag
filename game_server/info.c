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
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <time.h>

// ---------------- Local includes  e.g., "file.h"
#include "../lib/memory.h"
#include "../lib/hashtable.h"
#include "../lib/list.h"


// ---------------- Constant definitions

// ---------------- Macro definitions

// ---------------- Structures/Types 

typedef struct info{
	char** msgArr;
	int arrayLen;
	hashtable_t* teamhash;
	hashtable_t* codehash;
	hashtable_t* agenthash;
	list_t* captureList;
	int commsock;
	struct sockaddr_in themp;
	char* currentGameID;
	time_t lastContact; 
	double remainingTime;
	double elapsedTime;
} info_t;

// ---------------- Private variables 

// ---------------- Private prototypes 

/************************************************/


info_t* new_info(char** msgArr, int arrayLen, hashtable_t* teamhash, hashtable_t* codehash, hashtable_t* agentHash, int commsock, struct sockaddr_in themp, char* currentGameID, time_t lastContact, list_t* captureList, double remainingTime, double elapsedTime)
{
	info_t* infostore = assertp(count_calloc(1, sizeof(info_t)), "Memory allocation for new info store failed.\n");
	infostore->msgArr = msgArr;
	infostore->arrayLen = arrayLen;
	infostore->teamhash= teamhash;
	infostore->codehash = codehash;
	infostore->agenthash = agentHash;
	infostore->captureList = captureList;
	infostore->commsock = commsock; 
	infostore->themp = themp;
	infostore->currentGameID = currentGameID;
	infostore->lastContact = lastContact;
	infostore->remainingTime = remainingTime;
	infostore->elapsedTime = elapsedTime;

	return infostore;
}


void update_msgArr(info_t* infostore, char** msgArr)
{
	if (infostore != NULL)
		infostore->msgArr = msgArr;
}


void update_arrayLen(info_t* infostore, int arrayLen)
{
	if (infostore != NULL)
		infostore->arrayLen = arrayLen;
}

void update_infostore_themp(info_t* infostore, struct sockaddr_in themp)
{
	if (infostore != NULL)
		infostore->themp = themp;
}

void update_lastCont(info_t* infostore, time_t lastContact)
{
	if (infostore != NULL)
		infostore->lastContact = lastContact;
}

void update_remainingTime(info_t* infostore, double remainingTime)
{
	if (infostore != NULL)
		infostore->remainingTime = remainingTime;
}

void update_elapsedTime(info_t* infostore, double elapsedTime)
{
	if (infostore != NULL)
		infostore->elapsedTime = elapsedTime;
}


void del_infostore(info_t* infostore)
{
	if (infostore == NULL){
		fprintf(stderr, "Infostore does not exist\n");
		return NULL;
	}

	count_free(infostore);
}

/*** Getter methods **/

char** get_msg_arr(info_t* infostore)
{
	if (infostore == NULL){
		fprintf(stderr, "Infostore does not exist\n");
		return NULL;
	}

	return infostore->msgArr;
}


int get_arr_len(info_t* infostore)
{
	if (infostore == NULL){
		fprintf(stderr, "Infostore does not exist\n");
		return NULL;
	}

	return infostore->arrayLen;
}


hashtable_t* get_team_hash(info_t* infostore)
{
	if (infostore == NULL){
		fprintf(stderr, "Infostore does not exist\n");
		return NULL;
	}

	return infostore->teamhash;
}


hashtable_t* get_code_hash(info_t* infostore)
{
	if (infostore == NULL){
		fprintf(stderr, "Infostore does not exist\n");
		return NULL;
	}

	return infostore->codehash;
}


int get_commsock(info_t* infostore)
{
	if (infostore == NULL){
		fprintf(stderr, "Infostore does not exist\n");
		return NULL;
	}

	return infostore->commsock;
}

struct sockaddr_in get_themp(info_t* infostore)
{
	if (infostore == NULL){
		fprintf(stderr, "Infostore does not exist\n");
	}

	return infostore->themp;
}

char* get_gameid(info_t* infostore)
{
	if (infostore == NULL){
		fprintf(stderr, "Infostore does not exist\n");
		return NULL;
	}

	return infostore->currentGameID;	
}


time_t get_curr_time(info_t* infostore)
{
	if (infostore == NULL){
		fprintf(stderr, "Infostore does not exist\n");
		return NULL;
	}

	return infostore->lastContact;
}


hashtable_t* get_agent_hash(info_t* infostore)
{
	if (infostore == NULL){
		fprintf(stderr, "Infostore does not exist\n");
		return NULL;
	}

	return infostore->agenthash;
}


list_t* get_capture_list(info_t* infostore)
{
	if (infostore == NULL){
		fprintf(stderr, "Infostore does not exist\n");
		return NULL;
	}

	return infostore->captureList;
}

time_t get_rem_time(info_t* infostore)
{
	return infostore->remainingTime;
}


time_t get_elapsed_time(info_t* infostore)
{
	return infostore->elapsedTime;
}
