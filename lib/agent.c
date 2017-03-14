/*	agent.c - a module for initializing, accessing and interacting with the agent struct

	Project name: Project Incomputable 
	Component name: Lib/agent.c

	This file contains ...
	
	Primary Author:	Max Zhuang, Raunak Bhojwani and Samuel Ching
	Date Created: 5/23/16	

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

#include "memory.h"

// ---------------- Constant definitions 

// ---------------- Macro definitions

// ---------------- Structures/Types 
typedef struct agent {
  char* gameID;
  char* pebbleID;
  char* teamName;
  char* playerName;
  float agentLat;
  float agentLong;
  char* playerStatus;
  struct sockaddr_in agentSocInfo;
  time_t lastCont;
  char* captureID; 
  time_t captureTimeStart;
 } agent_t;

// ---------------- Private variables 

// ---------------- Private prototypes 


/*====================================================================*/

/**Agent_New()**/

agent_t* agent_new(char* gameID, char* pebbleID, char* teamName, char* playerName, float agentLat, float agentLong, char* playerStatus, struct sockaddr_in agentSoc)
{
	agent_t* newAgent =  assertp(count_calloc(1, sizeof(agent_t)), "Memory allocation for new Agent failed.\n");
	char* teamNameCpy = assertp(count_calloc(strlen(teamName)*2+100, sizeof(char)), "Memory allocation for team name failed.\n");
	char* playerNameCpy = assertp(count_calloc(strlen(playerName)*2+100, sizeof(char)), "Memory allocation for player name failed.\n");
	char* playerStatusCpy = assertp(count_calloc(strlen(playerStatus)*2+100, sizeof(char)), "Memory allocation for player status failed.\n");
	char* pebbleIDCpy = assertp(count_calloc(strlen(pebbleID)*2+100, sizeof(char)), "Memory allocation for pebbleID failed.\n");
	char* gameIDCpy = assertp(count_calloc(strlen(gameID)*2+100, sizeof(char)), "Memory allocation for gameID failed.\n");
	char* captureID = assertp(count_calloc(20, sizeof(int)), "Memory allocation for gameID failed.\n");

	strcpy(gameIDCpy,gameID);
	strcpy(teamNameCpy,teamName);
	strcpy(playerNameCpy,playerName);
	strcpy(playerStatusCpy,playerStatus);
	strcpy(pebbleIDCpy,pebbleID);

	newAgent->gameID = gameIDCpy;
	newAgent->pebbleID = pebbleIDCpy;
	newAgent->teamName = teamNameCpy;
	newAgent->playerName = playerNameCpy;
	newAgent->agentLat = agentLat;
	newAgent->agentLong = agentLong;
	newAgent->playerStatus = playerStatusCpy;
	newAgent->lastCont = 0;
	newAgent->captureID = captureID; 
	newAgent->agentSocInfo = agentSoc;
	newAgent->captureTimeStart = 0; 

	//memcpy(newAgent->agentSocInfo, (struct sockaddr*) agentSoc, sizeof(struct sockaddr_in));

	return newAgent;
}


/** agent_del **/

void agent_del(agent_t* agent)
{
	if (agent != NULL){
		count_free(agent->teamName);
		count_free(agent->playerName);
		// do we need to close the agentSoc or will the agent close the socket on their end?
		count_free(agent->playerStatus);
		count_free(agent->pebbleID);
		count_free(agent->gameID);
		count_free(agent->captureID);
		//count_free(agent->agentSocInfo);
		count_free(agent);
	}
	else
		fprintf(stderr, "No Such Agent exists \n");
}

/** update_last_cont **/
void update_last_cont(agent_t* agent, time_t lastContTime)
{
	if (agent != NULL)
		agent->lastCont = lastContTime;
	else
		fprintf(stderr, "No Such Agent exists \n");
}


/** update_loc **/
void update_loc(agent_t* agent, float agentLat, float agentLong)
{
	if (agent != NULL){
		agent->agentLat = agentLat;
		agent->agentLong = agentLong;
	}
	else
		fprintf(stderr, "No Such Agent exists \n");
}

/** update_player_status **/
void update_player_status(agent_t* agent, char* status)
{
	if (agent != NULL){
		count_free(agent->playerStatus);
		char* playerStatusCpy = assertp(count_calloc(strlen(status)*2+100, sizeof(char)), "Memory allocation for player status failed.\n");
		strcpy(playerStatusCpy,status);
		agent->playerStatus = playerStatusCpy;
	}
	else
		fprintf(stderr, "No Such Agent exists \n");
}


/**get_player_status**/

char* get_player_status(agent_t* agent)
{
	if (agent != NULL){
		return agent->playerStatus;
	}
	else{
		fprintf(stderr, "No Such Agent exists\n");
		return NULL;
	}
}

/** update_player_soc **/

void update_player_soc(agent_t* agent, struct sockaddr_in agentSoc)
{
	if (agent != NULL)
		agent->agentSocInfo = agentSoc;
	else
		fprintf(stderr, "No Such Agent exists \n");
}

/** Get Agent's SocInfo **/

struct sockaddr_in get_agent_soc(agent_t* agent)
{
	return agent->agentSocInfo;
}

/** get_pebble_ID **/

char* get_pebble_ID(agent_t* agent)
{
	if (agent != NULL)
		return agent->pebbleID;
	else
		fprintf(stderr, "No Such Agent exists \n");
		return NULL;
}

char* get_team_name_agent(agent_t* agent)
{
	if (agent != NULL)
		return agent->teamName;
	else
		fprintf(stderr, "Agent does not exist \n");
		return NULL;
}


float get_agent_lat(agent_t* agent)
{
	return agent->agentLat;
}

float get_agent_long(agent_t* agent)
{
	return agent->agentLong;
}


char* get_capture_ID(agent_t* agent)
{
	return agent->captureID;
}

void update_capture_ID(agent_t* agent, char* captureID)
{
	strcpy(agent->captureID,captureID);
}

agent_t* copy_agent(agent_t* agent)
{
	agent_t* newAgent =  assertp(count_calloc(1, sizeof(agent_t)), "Memory allocation for new Agent failed.\n");
	char* teamNameCpy = assertp(count_calloc(strlen(agent->teamName)*2, sizeof(char)), "Memory allocation for team name failed.\n");
	char* playerNameCpy = assertp(count_calloc(strlen(agent->playerName)*2, sizeof(char)), "Memory allocation for player name failed.\n");
	char* playerStatusCpy = assertp(count_calloc(strlen(agent->playerStatus)*2, sizeof(char)), "Memory allocation for player status failed.\n");
	char* pebbleIDCpy = assertp(count_calloc(strlen(agent->pebbleID)*2, sizeof(char)), "Memory allocation for pebbleID failed.\n");
	char* gameIDCpy = assertp(count_calloc(strlen(agent->gameID)*2, sizeof(char)), "Memory allocation for gameID failed.\n");
	char* captureID = assertp(count_calloc(20, sizeof(int)), "Memory allocation for gameID failed.\n");

	strcpy(gameIDCpy,agent->gameID);
	strcpy(teamNameCpy,agent->teamName);
	strcpy(playerNameCpy,agent->playerName);
	strcpy(playerStatusCpy,agent->playerStatus);
	strcpy(pebbleIDCpy,agent->pebbleID);
	strcpy(captureID, agent->captureID);

	newAgent->gameID = gameIDCpy;
	newAgent->pebbleID = pebbleIDCpy;
	newAgent->teamName = teamNameCpy;
	newAgent->playerName = playerNameCpy;
	newAgent->agentLat = agent->agentLat;
	newAgent->agentLong = agent->agentLong;
	newAgent->playerStatus = playerStatusCpy;
	newAgent->agentSocInfo = agent->agentSocInfo;
	newAgent->lastCont = agent->lastCont;
	newAgent->captureID = captureID;

	return newAgent;
}

char* get_game_id(agent_t* agent)
{
	return agent->gameID;
}

time_t get_agent_lastc(agent_t* agent)
{
	return agent->lastCont;
}

char* get_player_name(agent_t* agent)
{
	if (agent != NULL)
		return agent->playerName;
	else
		fprintf(stderr, "Agent does not exist \n");
		return NULL;
}

time_t get_capture_time_start(agent_t* agent)
{
	return agent->captureTimeStart;
}

void update_capture_time_start(agent_t* agent, time_t captureTimeStart)
{
	if (agent != NULL){
		agent->captureTimeStart = captureTimeStart;
	}
}
