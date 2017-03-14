/*	team.c - a module for initializing, accessing and interacting with the team struct

	Project name: Project Incomputable 
	Component name: Lib/team.c

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

// ---------------- Local includes  e.g., "file.h"

#include "memory.h"
#include "list.h"
#include "agent.h"

// ---------------- Constant definitions 

// ---------------- Macro definitions

// ---------------- Structures/Types 
typedef struct team {
  int guideID;
  time_t guideLastSeen;
  char* guideStatus;
  int codeDropsNeut;
  int numFAOpsActive;
  int numGAOpsActive;
  char* teamName;
  char* gameID;
  struct sockaddr_in gaSocInfo;
  int totalNumAgents;
  int foeAgentsCaptured; 
  //struct sockaddr_in* currSocInfo;

  //list_t* agentList;
 } team_t;

// ---------------- Private variables 

// ---------------- Private prototypes 

/*====================================================================*/

/**Agent_New()**/

team_t* team_new(int guideID, char* gameID, char* teamName, char* guideStatus, struct sockaddr_in SocInfo, time_t guideLastSeen)
{	
	team_t* team = assertp(count_calloc(1, sizeof(team_t)), "Memory allocation for new Team failed.\n");
	char* guideStatusCpy = assertp(count_calloc(strlen(guideStatus)+100, sizeof(char)), "Memory allocation for guide status failed.\n");
	char* teamNameCpy = assertp(count_calloc(strlen(teamName)+100, sizeof(char)), "Memory allocation for team name failed.\n");
	char* gameIDCpy = assertp(count_calloc(strlen(gameID)+100, sizeof(char)), "Memory allocation for gameID failed.\n");

	strcpy(guideStatusCpy, guideStatus);
	strcpy(teamNameCpy, teamName);
	strcpy(gameIDCpy, gameID);

	//team->currSocInfo = SocInfo;
	team->guideID = guideID;
	team->gameID = gameIDCpy;
	team->guideStatus = guideStatusCpy;
	team->teamName = teamNameCpy;
	team->guideLastSeen = guideLastSeen;
	team->numFAOpsActive = 0;
	team->numGAOpsActive = 1;
	team->codeDropsNeut = 0;
	team->totalNumAgents = 1; // initialize the total number of agents as 1 (since the GA becomes active)

	// Allocate memory for copying the soc info for the guide agent
	//struct sockaddr_in* gaSocInfo = assertp(count_calloc(1, sizeof(struct sockaddr_in)), "Memory allocation for sockaddr_in failed.\n");
	
	//memcpy(gaSocInfo, (struct sockaddr*) SocInfo, sizeof(struct sockaddr_in));
	//printf("MEMTEST: [%s@%05d]: check\n\n\n", inet_ntoa((gaSocInfo->sin_addr)), ntohs(gaSocInfo->sin_port));
	team->gaSocInfo = SocInfo;

	return team;
}


/** agent_del **/

void team_del(team_t* team)
{
	if (team != NULL){

		if (team->guideStatus != NULL)
			count_free(team->guideStatus);
	
		if (team->teamName != NULL)
			count_free(team->teamName);
		
		if (team->gameID != NULL)
			count_free(team->gameID);
		//count_free(team->gaSocInfo);
		count_free(team);
	}
}



//Update guide status
void update_guide_status(team_t* team, char* guideStatus)
{
	if (team != NULL){
		team->guideStatus = guideStatus;
	}
	else{
		fprintf(stderr, "Team is NULL \n");
	}
}


//Update GA Soc Info
void update_GASoc(team_t* team, struct sockaddr_in socInfo)
{
	if (team != NULL){
		team->gaSocInfo = socInfo;
	}
	else{
		fprintf(stderr, "Team is NULL \n");
	}
}



int get_guide_ID(team_t* team)
{
	if (team != NULL){
		return team->guideID;
	}
	else{
		fprintf(stderr, "Team is NULL \n");
		return 9999;
	}
}


char* get_team_name(team_t* team)
{
	if (team != NULL){
		return team->teamName;
	}
	else{
		fprintf(stderr, "Team is NULL \n");
		return NULL;
	}
}

void update_guide_ctime(team_t* team, time_t timeUpdate)
{
	
	if (team != NULL){
		team->guideLastSeen = timeUpdate;
	}
}

time_t get_guide_ctime(team_t* team)
{
	return team->guideLastSeen;
}


void add_num_FA_Ops_Active(team_t* team)
{
	 team->numFAOpsActive++;
	 team->totalNumAgents++;
}

void minus_num_FA_Ops_Active(team_t* team)
{
	team->numFAOpsActive--;
}

void add_codedrops_neut(team_t* team)
{
	team->codeDropsNeut++;
}


int get_num_active_fa(team_t* team)
{
	return team->numFAOpsActive;
}

int get_num_active_ga(team_t* team)
{
	return team->numGAOpsActive;
}

int get_num_code_neut(team_t* team)
{
	return team->codeDropsNeut;
}

int get_total_num_agents(team_t* team)
{
	return team->totalNumAgents;
}

int get_num_foe_captured(team_t* team)
{
	return team->foeAgentsCaptured;
}


void update_num_foe_captured(team_t* team)
{
	team->foeAgentsCaptured++;	
}

struct sockaddr_in get_guide_soc(team_t* team)
{
	return team->gaSocInfo;
}

/**
agent_t* find_agent(team_t* team, int pebbleID)
{
	if (team != NULL){
		char pebbleIDchar = pebbleID + '0'; //convert the pebbleID from int to char
		agent_t* agent;
		if ((agent = list_find(team->agentList, &pebbleIDchar)) != NULL)
			return agent;
		else
			return NULL;
	}
	else{
		fprintf(stderr, "Team is NULL \n");
		return NULL;
	}
}
*/

/*void insert_agent(team_t* team, agent_t* agent)
{
	if (team != NULL && agent != NULL){
		char pebbleID = get_pebble_ID(agent) + '0'; // convert the pebbleID from int to char
		list_insert(team->agentList, &pebbleID,agent);
	}
	else{
		fprintf(stderr, "team or agent is NULL\n");
	}
}
*/



