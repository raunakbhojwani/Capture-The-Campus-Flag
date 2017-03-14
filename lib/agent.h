/* 	agent.h module for creating an agent struct

	Project name: Project Incomputable
	Component name:

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
#include <time.h>

// ---------------- Local includes  e.g., "file.h"

#include "memory.h"

// ---------------- Constants

// ---------------- Structures/Types

typedef struct agent agent_t;

// ---------------- Public Variables

// ---------------- Prototypes/Macros

/** agent_new()
* Creates and returns a new agent based on the parameters passed in */
agent_t* agent_new(char* gameID, char* pebbleID, char* teamName, char* playerName, float agentLat, 
	float agentLong, char* playerStatus, struct sockaddr_in agentSoc);



/** agent_del()
* Deletes the agent struct as well as its contents */

void agent_del(agent_t* agent);


/** update_last_cont()
* Updates the last contact time of the guide agent */

void update_last_cont(agent_t* agent, time_t seconds);

/** update_loc()
* Based on the new agent latitude and longitude, update the player's location */

void update_loc(agent_t* agent, float agentLat, float agentLong);

/** update_player_status
* If the player has been captured, update the playerStatus to reflect that */

void update_player_status(agent_t* agent, char* status);


/** get_player_status
* returns the player's current status*/
char* get_player_status(agent_t* agent);


/** Update_player_soc
* If the player's IP Address and Port Address changes, then update it accordingly */

void update_player_soc(agent_t* agent, struct sockaddr_in agentSoc);

/** Get Agent's SocInfo **/
struct sockaddr_in get_agent_soc(agent_t* agent);

/** Get Pebble ID
* Getter method to get the pebble ID of an agent */

char* get_pebble_ID(agent_t* agent);

/** Get the Agent's TeamName
* Getter method to get the team of an agent*/

char* get_team_name_agent(agent_t* agent);

/** Get the Agent's Lat */
float get_agent_lat(agent_t* agent);

/* Get the Agent's Long */
float get_agent_long(agent_t* agent);

/** Get the Agent's Capture ID*/
char* get_capture_ID(agent_t* agent);

/* Update's the agent's capture ID*/
void update_capture_ID(agent_t* agent, char* captureID);

/* Returns a copy of the current Agent*/
agent_t* copy_agent(agent_t* agent);

/** Returns the GameID of current agent */
char* get_game_id(agent_t* agent);

/** get last contact from the FA **/
time_t get_agent_lastc(agent_t* agent);

/** Get agent's Player Name */
char* get_player_name(agent_t* agent);

/** Get the Start Time of the capturing process **/
time_t get_capture_time_start(agent_t* agent);

/** Update the Start Time of the capturing process of the agent **/
void update_capture_time_start(agent_t* agent, time_t captureTimeStart);

