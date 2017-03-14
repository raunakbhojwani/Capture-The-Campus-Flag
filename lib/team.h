/* 	team.h module for creating a team struct

	Project name: Project Incomputable
	Component name: Lib/team.h

	This file contains ...
	
	Primary Author:	
	Date Created:	
	
======================================================================*/
// do not remove any of these sections, even if they are empty


// ---------------- Prerequisites e.g., Requires "math.h"

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

// ---------------- Constants

// ---------------- Structures/Types

typedef struct team team_t;

// ---------------- Public Variables

// ---------------- Prototypes/Macros

team_t* team_new(int guideID, char* gameID, char* teamName, char* guideStatus, struct sockaddr_in SocInfo, time_t guideLastSeen);
void team_del(team_t* team);
//void insert_agent(team_t* team, agent_t* agent);
//agent_t* find_agent(team_t* team, int pebbleID);

void update_guide_status(team_t* team, char* guideStatus);

void update_GASoc(team_t* team, struct sockaddr_in socInfo);


/** If given a valid team, return the guideID associated with that team */
int get_guide_ID(team_t* team);

/** return team name **/
char* get_team_name(team_t* team);

/** update the GA's last seen time **/

void update_guide_ctime(team_t* team, time_t timeUpdate);

/** get the GA's last seen time **/
time_t get_guide_ctime(team_t* team);

/** increment the number of Active Agents by 1 **/
void add_num_FA_Ops_Active(team_t* team);

/** decrease the number of active agents by 1 **/
void minus_num_FA_Ops_Active(team_t* team);

/** increment the number of code drops which the team neutralized by 1*/
void add_codedrops_neut(team_t* team);

/** get the number of active FAs remaining in this team */

int get_num_active_fa(team_t* team);

/** get the number of active GAs in the team */
int get_num_active_ga(team_t* team);

/** get the number of code neutralizations done by the team */
int get_num_code_neut(team_t* team);


/** get the total number of agents which were ever active in the team */
int get_total_num_agents(team_t* team);

/** update the total number of foe agents captured by this team **/ /** update the total number of foe agents captured by this team **/ 

int get_num_foe_captured(team_t* team);


/** update the total number of foe agents captured by this team **/ 

void update_num_foe_captured(team_t* team);

/** Gets the team guide's socket info **/

struct sockaddr_in get_guide_soc(team_t* team);



