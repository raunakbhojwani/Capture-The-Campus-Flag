/* 	info.h module for creating a team struct

	Project name: Project Incomputable
	Component name: Lib/info.h

	This file contains ...
	
	Primary Author:	
	Date Created:	
	
======================================================================*/
// do not remove any of these sections, even if they are empty

// ---------------- Constants

// ---------------- Structures/Types

typedef struct info info_t;

// ---------------- Public Variables

// ---------------- Prototypes/Macros

info_t* new_info(char** msgArr, int arrayLen, hashtable_t* teamhash, hashtable_t* codehash, hashtable_t* agentHash, int commsock, struct sockaddr_in themp, char* currentGameID, time_t lastContact, list_t* captureList, double remainingTime, double elapsedTime);
void del_infostore(info_t* infostore);
char** get_msg_arr(info_t* infostore);
int get_arr_len(info_t* infostore);

hashtable_t* get_agent_hash(info_t* infostore);
hashtable_t* get_team_hash(info_t* infostore);
hashtable_t* get_code_hash(info_t* infostore);
list_t* get_capture_list(info_t* infostore);

int get_commsock(info_t* infostore);
struct sockaddr_in get_themp(info_t* infostore);
char* get_gameid(info_t* infostore);
time_t get_curr_time(info_t* infostore);
time_t get_rem_time(info_t* infostore);
time_t get_elapsed_time(info_t* infostore);

/** Update the message array stored in the infostore **/

void update_msgArr(info_t* infostore, char** msgArr);

/** Updates the array length of the message array stored in the infostore */

void update_arrayLen(info_t* infostore, int arrayLen);

/** Update the current sock address of the sender to the server */

void update_infostore_themp(info_t* infostore, struct sockaddr_in themp);

/** Updates the last contact time of the sender */

void update_lastCont(info_t* infostore, time_t lastContact);

/** Updates the remaining time left in the game */

void update_remainingTime(info_t* infostore, double remainingTime);

/** Updates the elapsed time left in the game */

void update_elapsedTime(info_t* infostore, double elapsedTime);



