/*	gameserver.c A program which runs on the CS50 Unix Server, communicating between the Field Agents and the Guide Agents as well as storing information about the Game State.


	Project name: Project Incomputable (Jade)
	Component name: Game Server

	Primary Authors: Max Zhuang, Raunak Bhojwani, Samuel Ching
	Date Created: 5/22/16	

	Special considerations:  
		(e.g., special compilation options, platform limitations, etc.) 
	
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
#include <ctype.h>

// ---------------- Local includes  e.g., "file.h"

#include "../lib/memory.h"
#include "../lib/message.h"
#include "../lib/hashtable.h"
#include "../lib/agent.h"
#include "../lib/team.h"
#include "../lib/codedrop.h"
#include "../lib/latlongdist.h"
#include "../lib/list.h"
#include "info.h"
#include "log.h"
#include "../lib/randomHexGen.h"

// ---------------- Constant definitions 

const int HASHSLOTS = 1000;
const int FA_LOC_LEN = 8;
const int FA_NEU_LEN = 8;
const int FA_CAP_LEN = 6;
const int GA_STAT_LEN = 6;
const int GA_HINT_LEN = 7;
const char DELIM[] = "|";
const int MAX_MSG_LEN = 1000;
const int PEBBLE_LEN = 32; 
int MAXSTR = 25000;


// codeID response codes
char codeActive[] = "Active";
char codeNeutralized[] = "Neutralized";


/** OPCodes**/

//GS response codes
char gameStatus[] = "GAME_STATUS";
char gsCaptureID[] = "GS_CAPTURE_ID";
char gameOverOpCode[] = "GAME_OVER";


//FA Status 
char FACaptured[] = "captured";
char FACapturing[] = "capturing";
char FAActive[] = "active";
char mbCaptured[] = "maybe-captured";

//GA Status
char GAActive[] = "active";

//Game Mode Log Purposes
char newPlayerJoined[] = "NEW_FA_JOINED";
char playerCaptured[] = "FA_CAPTURED";
char codeDropNeut[] = "CODE_DROP_NEUT";


// Server Response Codes
char gsResponse[] = "GS_RESPONSE";
char invalidOpCode[] = "MI_ERROR_INVALID_OPCODE";
char invalidGameID[] = "MI_ERROR_INVALID_GAME_ID";
char neutralizedOpCode[] = "MI_NEUTRALIZED";
char captureSuccessCode[] = "MI_CAPTURE_SUCCESS";
char captureCode[] = "MI_CAPTURED";
char invalidTeamName[] = "MI_ERROR_INVALID_TEAMNAME";
char invalidGenericID[] = "MI_ERROR_INVALID_ID";

// ---------------- Macro definitions


// ---------------- Private variables

//logMode - determines which log function to call
int logMode = 0; // if raw, then logMode == 1. Else, the default logMode == 0

// ---------------- Private prototypes 

// Input validation
static int val_input(info_t* infostore);
static int read_in(char* codeDropFile, hashtable_t* codehashtable);

// Handling data structures
static int tokenize_input(char* message, char** msgArr);

// OPCode Handling
static bool validate_gameID(char* message); // Helper function 
static bool validate_guideID(char* message);
static bool validate_teamName(char** messageArr, char* message, hashtable_t* teamHash);
static bool validate_playerName(char* gameID, char* pebbleID, char* agentName, hashtable_t* agentHash);

static int update_fa_location(info_t* infostore);
static int neutralize_code(info_t* infostore);
static int capture_player(info_t* infostore);
static int update_GA(info_t* infostore);
static int send_hint(info_t* infostore);

// Helper functions for OPCode Handling
static bool check_guideID(info_t* infostore, char* guideID);
static void guideIDiter(void* arg, char* key, void* data);

// Capturing
static struct listOperatives find_foe_ops(info_t* infostore, char* teamName, agent_t* agent, list_t* captureList);
static void find_foe_ops_helper(void* arg, char* key, void* data);
static void captureListDelete(void* data);
static void gs_capture_notif(info_t* infostore, struct sockaddr_in themp, char* captureID, char* gameID);


// Sending Hints
static void send_all_friendlyOp(info_t* infostore, char* message, char* teamName);
static void send_hint_iter(void* arg, char* key, void* data);

//Game Update Msgs
static int send_game_status_fa(info_t* infostore, char* teamName);
static int find_rem_code(info_t* infostore);
static void find_rem_code_helper(void* arg, char* key, void* data);
static int send_game_status_ga(info_t* infostore);
static void agentStatIter(void* arg, char* key, void* data);
static void hashStatIter(void* arg, char* key, void* data);

static int find_num_friendly_operatives(info_t* infostore, char* teamName);
static void find_num_friend_helper(void* arg, char* key, void* data);

static int find_num_foe_operatives(info_t* infostore, char* teamName);
static void find_num_foe_helper(void* arg, char* key, void* data);

// Sends gameOver message to all 
static void game_over(info_t* infostore);
static void teamRecordIter(void* arg, char* key, void* data);
static void faSendIter(void* arg, char* key, void* data);
static void gaSendIter(void* arg, char* key, void* data);

// Track Game Stats
struct gameStats* trackGameStats(info_t* infostore, time_t startTime);

static void teamIter(void* arg, char* key, void* data);
static void activeOpsIter(void* arg, char* key, void* data);
static void codeNeutIter(void* arg, char* key, void* data);
static void yetNeutIter(void* arg, char* key, void* data);
static void counterListDel(void* data);
static void game_stats_delete(struct gameStats* stats);
static void print_team_neut(void* arg, char* key, void* data);


// Presenting Text Based Interface
static void print_all_code(void* arg, char* key, void* data);
static void print_all_agents(void* arg, char* key, void* data);

// Server Response Codes
static void invalid_opcode_response(info_t* infostore);
static void invalid_gameid(info_t* infostore);
static void code_neutralized(info_t* infostore);
static void capture_success(agent_t* capturingAgent, info_t* infostore);
static void server_response(char* gameID, char* respCode, char* message, struct sockaddr_in themp, int comm_sock, info_t* infostore);
static void captured(agent_t* capturedAgent, info_t* infostore);
static void invalid_genericID(info_t* infostore);
static void invalid_teamname(info_t* infostore);



// ---------------- Structures/Types

// Helper Struct for number of Operatives
struct numOperatives{
	char* teamName;
	int numOperatives;
};

// Helper Struct for List of nearby operatives

struct listOperatives{
	char* teamName;
	info_t* infostore;
	agent_t* capturingAgent;
	list_t* captureList; 
};


struct captureStruct{
	char* captureID;
	agent_t* capturingAgent;
	agent_t* capturedAgent;
	time_t captureTimeStart;
};

// Helper struct for guideID check

struct guideIDCheck{
	char* guideID;
	int guideIDPresent; 
};


// Helper struct for sending Hint from GA to FAs
struct sendHint{
	info_t* infostore;
	char* message;
	char* teamName;
};

// Helper struct for sending GAME_STATUS updates to GAs

struct sendGAUpdate{
	info_t* infostore;
	char* message;
	int beginFlag; 
};

// Helper Struct for the Game Over Message

struct gameOverInfo{
	info_t* infostore;
	char* message;
	int beginFlag;
};

// Helper Struct for storing the stats for the Game

struct gameStats{
	time_t elapsedTime;
	int numActiveAgents;
	int numActiveTeams;
	list_t* teamNeutCodes;
	int numCodeDropNotNeut;
};


static void captureListFree(struct captureStruct* captureS);

static const struct {
  const char *opcode;
  int (*func)(info_t* infostore);
} use[] = {
  { "FA_LOCATION", update_fa_location},
  { "FA_NEUTRALIZE", neutralize_code },
  {"FA_CAPTURE", capture_player},
  { "GA_STATUS", update_GA},
  { "GA_HINT", send_hint},
  { NULL, NULL }
};

/*====================================================================*/

int 
main (const int argc, char* argv[])
{
	// logging mode 
	int logArg = 0;

	// Check commandline arguments

	if (argc < 3){
		fprintf(stderr, "usage: %s [-log=raw] codeDropPath GSport\n", argv[0]);
		char* commandLineFail = "usage: %s [-log=raw] codeDropPath GSport\n";
		error_logging(commandLineFail);
		exit(1);
	}

	for (int i = 1; i < argc; i++){
		// Check optional arguments - including logging mode and game level
		if (strcmp(argv[i], "-log=raw") == 0){
			logMode = 1;
			logArg = i;
		}
	}

	// Ensuring that the args are parsed correctly (due to optional flags triggered)
	int codeDropArg = 0;
	int GSportArg = 0;

	for (int i = 1; i < argc; i++){
		if (i == logArg){
			;
		}
		else{
			if (codeDropArg == 0){
				codeDropArg = i;
			}
			if (codeDropArg != i && GSportArg == 0){
				GSportArg = i;
			}

		}
	}

	// if GSPortArg != 0 && codeDropArg != 0 
		// if logArg != 0, check the 3rd Arg
		// else check the 4th Arg

	//Check if codedropFile is a valid path by checking if it can be opened

	FILE* fp = fopen(argv[codeDropArg], "r");

	if ( fp == NULL){
		fprintf(stderr, "Please ensure that the codeDropPath exists\n");
		char* codedropPath = "Please ensure that the codeDropPath exists\n";
		error_logging(codedropPath);
		exit(1);
	}
	fclose(fp);


	float distanceTest = distance(43.7050831248106, -72.29492692556658, 43.7050831248106, -72.29492692556658);
	printf("Testing the distance formula: %f\n", distanceTest);

	// Start the game time

	time_t gameTimeStart = time(NULL);
	double totalGameTime = 100000000;
	double elapsedGameTime = 0;

	time_t curtime; // initialize system time
	time(&curtime);
	srand(time(NULL)); // seed the random number generator using time
	
	// Initialize the game ID
	char* currGameID = gen_random_hexcode(8);
	printf("currGameID is: %s\n", currGameID);

	// Initialize the data structures

	hashtable_t* teamHash = hashtable_new(HASHSLOTS, team_delete);
	hashtable_t* codeHash = hashtable_new(HASHSLOTS, code_delete);
	hashtable_t* agentHash = hashtable_new(HASHSLOTS, agent_delete);
	list_t* captureList = list_new(captureListDelete);

	// Read the list of code drops in from the file
	read_in(argv[codeDropArg],codeHash);


	// Setup socket
	//printf("GSPort arg %d\n", GSportArg);

	char* serverIP = assertp(count_calloc(MAXSTR, sizeof(char)), "Memory allocation for server host name failed.\n");
	int GSport = atoi(argv[GSportArg]);
	int comm_sock = socket_setup(GSport);
	struct sockaddr_in them = {0, 0, {0}};

	// Write Log Header
	log_header(curtime,currGameID, GSport, serverIP);

	// Initialize infostore struct with all the information currently available
	info_t* infostore = new_info(NULL, NULL, teamHash, codeHash, agentHash, comm_sock, them ,currGameID, NULL, captureList, 0, 0);


	int numCodesRemaining = 0;
	hashtable_iterate(codeHash, yetNeutIter, &numCodesRemaining); 

	// Continuous while-loop waiting for incoming datagrams

	// if the game time has elapsed, or if there are no codedrops remaining, then quit.
	while (elapsedGameTime < totalGameTime && numCodesRemaining > 0) {	


		// Clear Screen
		printf("\033[2J\033[1;1H"); 

		// Handles the incoming datagrams and parses them into messages
		char* message = handle_socket(comm_sock, &them);
		int maxwords = strlen(message);

		// Find the elapsed game time and the time remaining
		time_t currTime = time(NULL);
		elapsedGameTime = difftime(currTime, gameTimeStart); 
		double timeRemaining = totalGameTime - elapsedGameTime;

		// Log all incoming datagrams if its raw
		if (logMode == 1)
			raw_mode_logging_inbound(message, currTime, elapsedGameTime);

		// If the incoming message is not empty
		if (maxwords != 0){
			char** msgArray = assertp(count_calloc(maxwords, sizeof(char*)), "Memory allocation for message array failed.\n");
			int arraylen = tokenize_input(message,msgArray);
			
			// Check if tokenizeInput did not return NULL, proceed.
			if (arraylen != 0){

				// Update infostore struct with the latest info for later use in the handling of the opcodes
				update_msgArr(infostore, msgArray);
				update_arrayLen(infostore, arraylen);
				update_infostore_themp(infostore, them);
				update_lastCont(infostore, currTime);
				update_remainingTime(infostore, timeRemaining);
				update_elapsedTime(infostore, elapsedGameTime);

				val_input(infostore); // validate
			}

			// Free the message array 
			for (int i = 0; i < arraylen; i++){
				if (msgArray[i] != NULL)
					count_free(msgArray[i]);
			}
			count_free(msgArray);

		}

		// free the message created by handle_socket()
		if (message != NULL) 
			count_free(message);

		// Produce game stats
		struct gameStats* gameStats = trackGameStats(infostore, gameTimeStart);
		int numActiveAgents = gameStats->numActiveAgents;
		int numActiveTeams = gameStats->numActiveTeams;
		int codedropsNtNeut = gameStats->numCodeDropNotNeut;
		list_t* listTeams = gameStats->teamNeutCodes;


		// Print Summary of Game to Terminal
		printf("************************************ GAME_SUMMARY *********************************\n");
		printf("Elapsed Game Time: %f seconds\n", elapsedGameTime);
		printf("Time Remaining in the Game: %f seconds\n", timeRemaining);
		printf("Number Active Agents: %d\n", numActiveAgents);
		printf("Number Active Teams: %d\n", numActiveTeams);
		printf("Code Drops Yet To Be Neutralized: %d\n", codedropsNtNeut);
		printf("Status of Teams/Codes Neutralized\n");
		printf("-----------------------------------------------------------------------------------\n");
		list_iterate(listTeams, print_team_neut, stdout);
		printf("-----------------------------------------------------------------------------------\n");
		printf("CodeID\tLatitude\tLongtitude\tStatus\tNeutralizing Team\n");
		printf("-----------------------------------------------------------------------------------\n");
		hashtable_iterate(codeHash, print_all_code, stdout);
		printf("\nField Agent\tLatitude\tLongtitude\tStatus\n");
		printf("-----------------------------------------------------------------------------------\n");
		hashtable_iterate(agentHash,print_all_agents, stdout);


		// if the time is up or all the codes have been neutralized, then send game_over message to all agents
		numCodesRemaining = 0;
		hashtable_iterate(codeHash, yetNeutIter, &numCodesRemaining);

		if (timeRemaining <= 0 || numCodesRemaining < 0) {
			game_over(infostore);			
		}

		game_stats_delete(gameStats);
	}

	// Clean up data structures
	del_infostore(infostore);
	close(comm_sock);
	hash_delete(teamHash);
	hash_delete(codeHash);
	hash_delete(agentHash);
	
	if (serverIP != NULL)
		count_free(serverIP);

	if (currGameID != NULL)
		count_free(currGameID);

	list_delete(captureList);

	count_report(stdout, "The calloc/free counts for gameserver.c are as follows:\n");

	return 0;
}

/********************* tokenizeInput ***********************/
/* Takes in a string and character array, tokenizes the string into the character array
 * It requires the length of the message to be greater than 0
 * If there is an error, it will return 0.
 * Else, it will return the length of the character array.
 */

static int tokenize_input(char* message, char** msgArr)
{

	// Check params
	if (message == NULL){
		fprintf(stderr, "Message is NULL\n");
		char* tokenizeErr = "tokenizeErr: Message is NULL";
		error_logging(tokenizeErr);
		return 0;
	}

	if (msgArr == NULL){
		fprintf(stderr, "Message is NULL\n");
		char* tokenizeErr = "tokenizeErr: Message is NULL";
		error_logging(tokenizeErr);
		return 0;
	}

	int wordcount = 0; 
	char* word = NULL;

	for (word = strtok(message,DELIM); word != NULL; word = strtok(NULL, DELIM)){
		// save in list of words
		char* newWord = assertp(count_calloc(MAXSTR, sizeof(char)), "Memory allocation for word failed.\n");
		strcpy(newWord, word);
		msgArr[wordcount++] = newWord; // assign the new word to a position in the character array
	}
	return wordcount;
}

/********************** read_in() ************************/
/* Parses the codedrop file and populates a hashtable with these files
*  Reads in the codedrop file line by line 
*  Creates a new codedrop struct and passes that into the codedrop hashtable
*  If there are errors, then the function will return 1. Else, it will return 0.
*/

static int read_in(char* codeDropFile, hashtable_t* codehashtable)
{
	// error checking
	if (codeDropFile == NULL){
		fprintf(stderr, "Code Drop File does not exist, please check to ensure that it is present\n");
		char* readInErr = "readInErr: Message is NULL";
		error_logging(readInErr);
		return 1;
	}
	
	// check if the codeDrop file can be opened
	FILE* fp = fopen(codeDropFile, "r");

	if ( fp == NULL){
		fprintf(stderr, "Please ensure that the codeDropPath exists\n");
		char* readInErr = "readInErr: Message is NULL";
		error_logging(readInErr);
		return 1;
	}

	float codelat; 
	float codelong;
	char* codeID = assertp(count_calloc(MAXSTR, sizeof(char)), "Memory allocation for message failed.\n"); 

	// parses the code drop file according to the code latitude, longtitude and codeID
	// creates a new codedrop struct and inserts it into the hashtable

	while (!feof(fp) && fscanf(fp, "%f%*c %f%*c %s", &codelat, &codelong, codeID) == 3) { 

		codedrop_t* codedrop = codedrop_new(codelat,codelong,codeID,codeActive);
		hashtable_insert(codehashtable, codeID, codedrop);
		
	}
	count_free(codeID);
	fclose(fp);
	return 0;
}

/*********** print_team_neut **************/
/* Helper list iterator to print out the list of teams and the number of codes that they have neutralized
* Prints out to a File Path
*/

static void print_team_neut(void* arg, char* key, void* data)
{	
	FILE* fp = arg; 
	if (fp != NULL && data != NULL){
		int* counter = data;
		fprintf(fp, "Team: %s\t|\tNum Code Drops Success: %d\n", key, *counter);
	}
}

/*********** print_all_code **************/
/* Helper hashtable iterator to print out the list of codes - their location and status
* Prints out to a File Path
*/
static void print_all_code(void* arg, char* key, void* data)
{
	FILE* fp = arg; 
	if (fp != NULL && data != NULL){
		codedrop_t* codedrop = data;
		char* codeStatus = get_code_status(codedrop);
		float codeLat = get_code_lat(codedrop);
		float codeLong = get_code_long(codedrop);
		char* codeID = key;
		char* neutralizingTeam = get_neuteam(codedrop);


		if (neutralizingTeam == NULL)
			fprintf(fp, "%s\t%f\t%f\t%s\t\tNone\n",codeID, codeLat, codeLong, codeStatus);
		else
			fprintf(fp, "%s\t%f\t%f\t%s\t%s\n",codeID, codeLat, codeLong, codeStatus, neutralizingTeam);
	}
}

/*********** print_all_agents **************/
/* Helper hashtable iterator to print out the list of agents - their location and status
* Prints out to a File Path
*/

static void print_all_agents(void* arg, char* key, void* data)
{
	FILE* fp = arg;
	if (fp != NULL && data != NULL){
		agent_t* currAgent = data;
		float agentLat = get_agent_lat(currAgent);
		float agentLong = get_agent_long(currAgent);
		char* agentStatus = get_player_status(currAgent);
		char* agentName = get_player_name(currAgent);
		fprintf(fp, "%s\t\t%f\t%f\t%s\n", agentName, agentLat, agentLong, agentStatus);
	}

}


/*********** captureListDelete **************/
/* Helper Function for List Delete
* Used in list_delete to delete and free the captureList struct in the captureList
*/

static void captureListDelete(void* data)
{
	if (data != NULL){
		struct captureStruct* captureS = data;
		captureListFree(captureS);
	}
}
/*********** captureListFree **************/
/* Helper function to free the elements of captureStruct
* Takes in a captureStruct type
*/

static void captureListFree(struct captureStruct *captureS)
{
	if (captureS != NULL){
		count_free(captureS->captureID);
		agent_del(captureS->capturedAgent);
		agent_del(captureS->capturingAgent);
		count_free(captureS);
	}
}

/*********** val_input **************/
/** Takes in a character array from the infostore struct
* Checks against the OPCODEs to determine which function to call
* Uses the function table struct as defined in the header
*/

static int val_input(info_t* infostore)
{
	char** messageArr = get_msg_arr(infostore);

	if (messageArr == NULL){
		fprintf(stderr, "The message array does not exist \n");
		char* validateInputMsg = "Val_Input: Please ensure that the message array exists\n";
		error_logging(validateInputMsg);
		return 1; // if messageArray does not, return 1
	}

	char* opcode = messageArr[0]; // check the first element of the message array, assign it to be the OPCODE

	int fn;
	for (fn = 0; use[fn].opcode != NULL; fn++) { // if opcode matches with the set of opcodes, then call the function corresponding to the opcode
      if (strcmp(opcode, use[fn].opcode) == 0) {
		 (*use[fn].func)(infostore);
		 break;		 
	      }
	    }

	//If the opcode is unknown, send back an invalid response message to offending agent

    if (use[fn].opcode == NULL){ 
      printf("Unknown opcode: '%s'\n", opcode);
      invalid_opcode_response(infostore); // send back an invalid opcode response message to offending agent
      return 2; // if invalid opcode, return 2
    }
    return 0;
}


/**************************************************** Game Updates *******************************************************/

// Field Agents


/*********** send_game_status_fa **************/

/* Sends a game status update to the field agent who requested the status update
*  Takes in an infostore struct as well as a team name
* If there is an error, return 1. Else return 0.
* Pulls the relevant information from the infostore and calls helper functions to get the relevant information
* Then, copy it to a string and send the string out to the FA.
*/

static int send_game_status_fa(info_t* infostore, char* teamName)
{
	if (infostore == NULL){
		fprintf(stderr, "Info Store does not exist. Please try again.\n");
		char* errorMsg = "Infostore does not exist in send_game_status_fa.";
		error_logging(errorMsg);
		return 1;
	}


	char* gameID = get_gameid(infostore);
	hashtable_t* teamHash = get_team_hash(infostore);

	team_t* currTeam = hashtable_find(teamHash,teamName);

	// Get the guide ID of the FA's Team
	char* guideID = get_guide_ID(currTeam);

	// Find the number of remaining codedrops

	int numRemainingCD = find_rem_code(infostore);

	// Find the number of all friendly operatives 
	int numFriendlyOps = find_num_friendly_operatives(infostore, teamName);

	// Find the number of all the foe operatives outside of this team
	int numFoeOps = find_num_foe_operatives(infostore, teamName);

	char* gameStatusMsg = assertp(count_calloc(MAXSTR, sizeof(char)), "Memory allocation for game status failed.\n");

	// Find the remaining amount of time left in the game
	time_t currTime = get_curr_time(infostore);
	double remainingTime = get_rem_time(infostore);
	double remTimeMins = remainingTime/60;

	sprintf(gameStatusMsg, "%s|%s|%s|%d|%d|%d|%f", gameStatus, gameID, guideID, numRemainingCD, numFriendlyOps, numFoeOps, remTimeMins);

	// Get the address of the FA requesting the information
	struct sockaddr_in themp = get_themp(infostore);
	int comm_sock = get_commsock(infostore);


	// send datagram to requesting field agent
	int datagram_check;
	if ((datagram_check = send_datagram(comm_sock, &themp, gameStatusMsg)) != 0){
		fprintf(stderr, "Datagram failed to send\n");
	}

	// If the logmode is a raw log mode, then log the outgoing message
	if (logMode == 1){
		time_t elapsedTime = get_elapsed_time(infostore);
		raw_mode_logging_outbound(gameStatusMsg, currTime, elapsedTime);
	}

	if (gameStatusMsg != NULL)
		count_free(gameStatusMsg);

	return 0; 
}


/*********** find_rem_code **************/
/* Helper Function to find the number of remaining code drops in the game
* Takes in a infostore struct containing all the information 
* If there is an error, return 999. Else, return 0
*/ 

static int find_rem_code(info_t* infostore)
{
	if (infostore == NULL){
		fprintf(stderr, "Info Store does not exist. Please try again.\n");
		char* errorMsg = "Infostore does not exist in find_rem_code.";
		error_logging(errorMsg);
		return 999;
	}

	int numRemainingCD = 0;
	hashtable_t* codeHash = get_code_hash(infostore);

	// iterate over all codedrops to find the number of codes which have been neutralized
	hashtable_iterate(codeHash,find_rem_code_helper,&numRemainingCD);
	return numRemainingCD;
}


/*********** find_rem_code **************/
/* Helper Function to find the number of remaining code drops in the game. 
* Iterator helper function - looping through all the codedrops in the codehashtable.
*/ 
static void find_rem_code_helper(void* arg, char* key, void* data)
{
	int* numRemainingCD = arg;

	if (numRemainingCD != NULL && data != NULL){
		codedrop_t* codedrop = data;
		char* codeStatus = get_code_status(codedrop);

		if (strcmp(codeStatus,codeActive) == 0){
			(*numRemainingCD)++; // if the codedrop is active, increment the number of remaining codedrops
		}
	}
}

/*********** find_rem_code **************/
/* Helper function to find the number of Friendly operatives
* Takes in the infostore struct and the team name.
* If there is an error, return 999. Else, return 0.
*/ 

static int find_num_friendly_operatives(info_t* infostore, char* teamName)
{
	
	if (infostore == NULL){
		fprintf(stderr, "Info Store does not exist. Please try again.\n");
		char* errorMsg = "Infostore does not exist in find_num_friendly_operatives.";
		error_logging(errorMsg);
		return 999;
	}

	struct numOperatives numFriendlyOps = {teamName, 0};
	hashtable_t* agentHash = get_agent_hash(infostore);
	
	// iterate over all the agents in the agent hashtable
	hashtable_iterate(agentHash, find_num_friend_helper, &numFriendlyOps);
	return numFriendlyOps.numOperatives;
}

/*********** find_rem_code **************/
/* Helper function to iterate through the agent hashtable
*/

static void find_num_friend_helper(void* arg, char* key, void* data)
{

	struct numOperatives *numOps = arg;

	if (numOps != NULL && data != NULL){
		agent_t* agent = data;
		char* agentTeamName = get_team_name_agent(agent);
		char* agentStatus = get_player_status(agent);

		if (strcmp(agentTeamName, numOps->teamName) == 0){
			if (strcmp(agentStatus, FAActive) == 0){
				numOps->numOperatives++;
			}
		}		
	}
}

/*********** find_num_foe_operatives **************/
/* Helper function to find the number of foe operatives
* Takes in the infostore struct and the current agent's teamName.
* If there is an error, return 999. Else, return 0.
*/ 


static int find_num_foe_operatives(info_t* infostore, char* teamName)
{
	
	if (infostore == NULL){
		fprintf(stderr, "Info Store does not exist. Please try again.\n");
		char* errorMsg = "Infostore does not exist in find_num_foe_operatives.";
		error_logging(errorMsg);
		return 999;
	}

	struct numOperatives numFoeOps = {teamName, 0}; // store the teamName and the number of foes in a struct

	hashtable_t* agentHash = get_agent_hash(infostore);
	hashtable_iterate(agentHash, find_num_foe_helper, &numFoeOps); //iterate over all the agents in the hashtable

	return numFoeOps.numOperatives;
}

/*********** find_num_foe_helper **************/
/* Helper function to iterate over the agents in the agent hashtable
*/ 

static void find_num_foe_helper(void* arg, char* key, void* data)
{
	struct numOperatives *numOps = arg;

	if (numOps != NULL && data != NULL){
		agent_t* agent = data;
		char* agentTeamName = get_team_name_agent(agent);
		char* agentStatus = get_player_status(agent);

		// if the current agent is not in the same team as the agent in the hashtable
		// and if the agent in the hashtable is active
		// then increment the number of operatives

		if (strcmp(agentTeamName, numOps->teamName) != 0){
			if (strcmp(agentStatus, FAActive) == 0){
				numOps->numOperatives++;
			}
		}
	}
}


/*********** send_game_status_ga **************/
/** Game Status Update to Guide Agent
* Takes in the infostore struct with the right information and sends an update to the guide agent
* If there are errors, then return 1, else return 0
*/

static int send_game_status_ga(info_t* infostore)
{
	if (infostore == NULL){
		fprintf(stderr, "Info Store does not exist. Please try again.\n");
		char* errorMsg = "Infostore does not exist in send_game_status_ga.";
		error_logging(errorMsg);
		return 1;
	}

	// Get Game Status and GameID

	char* gameID = get_gameid(infostore);
	hashtable_t* agentHash = get_agent_hash(infostore);
	char* GAUpdate = assertp(count_calloc(MAXSTR+10000, sizeof(char)), "Memory allocation for GAUpdate failed.\n");
	sprintf(GAUpdate, "%s|%s|", gameStatus, gameID);

	struct sendGAUpdate GAUpdateStruct = {infostore, GAUpdate, 0}; // initialize struct 
	hashtable_iterate(agentHash, agentStatIter, &GAUpdateStruct); // iterate over all agents in the agent hashtable

	strcat(GAUpdate, DELIM);
	GAUpdateStruct.beginFlag = 0; // reset the beginFlag (for colon delimiter)
	hashtable_t* codeHash = get_code_hash(infostore); // iterate over all codes 
	hashtable_iterate(codeHash, hashStatIter, &GAUpdateStruct); 

	// get the socket address of the guide agent
	int commSock = get_commsock(infostore);
	struct sockaddr_in themp = get_themp(infostore);

	// send the message to the guide agent
	int datagram_check;
	if ((datagram_check = send_datagram(commSock, &themp, GAUpdate)) != 0) // send message to guide agent
		fprintf(stderr, "Datagram failed to send\n");

	// initialize time for logging
	time_t currTime = get_curr_time(infostore);
	double elapsedTime = get_elapsed_time(infostore);

	if (logMode == 1)
		raw_mode_logging_outbound(GAUpdate, currTime, elapsedTime);

	if (GAUpdate != NULL)
		count_free(GAUpdate);

	return 0;
}


/*********** agentStatIter **************/
/** Helper function for game status update function
* Iterates through each agent and extracts the relevant information 
* The information is then added to the main string
*/

static void agentStatIter(void* arg, char* key, void* data)
{
	struct sendGAUpdate* GAUpdateStruct = arg;

	if (GAUpdateStruct != NULL && data != NULL){
		int beginFlag = GAUpdateStruct->beginFlag;

		// get the message and the infostore
		info_t* infostore = GAUpdateStruct->infostore;
		char* GAUpdate = GAUpdateStruct->message;
		char colon[] = ":";

		if (beginFlag == 1)
			strcat(GAUpdate, colon);

		agent_t* agent = data;

		// store information relating to the time
		time_t currTime = get_curr_time(infostore);
		time_t lastContTime = get_agent_lastc(agent);
		double secondsSinceLastCont = difftime(currTime, lastContTime);

		// store all the relevant information for each agent in a string
		char* tempStr = assertp(count_calloc(MAXSTR, sizeof(char)), "Memory allocation for GAUpdate failed.\n");
		sprintf(tempStr, "%s%s,%s,%s,%s,%f,%f,%f" , GAUpdate, get_pebble_ID(agent), get_team_name_agent(agent), get_player_name(agent), get_player_status(agent), get_agent_long(agent),get_agent_lat(agent),secondsSinceLastCont);
		strcpy(GAUpdate, tempStr);
		count_free(tempStr);

		GAUpdateStruct->beginFlag = 1;
	}
}

/*********** hashStatIter **************/
/** Helper function for game status update function
* Iterates through each codedrop and extracts the relevant information 
* The information is then added to the main string
*/

static void hashStatIter(void* arg, char* key, void* data)
{
	struct sendGAUpdate* GAUpdateStruct = arg;

	if (GAUpdateStruct != NULL && data != NULL){
		char* GAUpdate = GAUpdateStruct->message;
		codedrop_t* codedrop = data;

		if (codedrop != NULL){

			// check if its the beginning of the entire string containing codedrop info
			int beginFlag = GAUpdateStruct->beginFlag;
			if (beginFlag == 1){
				char colon[] = ":";
				strcat(GAUpdate, colon);
			}

			// store all the relevant information for each agent in a string
			char* tempStr = assertp(count_calloc(MAXSTR, sizeof(char)), "Memory allocation for GAUpdate failed.\n");
			sprintf(tempStr,"%s%s,%f,%f,",GAUpdate, key, get_code_lat(codedrop), get_code_long(codedrop)); // concatenate the latitude and longtitude 
			strcpy(GAUpdate,tempStr);
			count_free(tempStr);


			// if there is no neutralizing team, then append None to the string
			if (get_neuteam(codedrop) == NULL)
			{	
				char none[] = "None";
				strcat(GAUpdate,none);
				GAUpdateStruct->beginFlag = 1;
			}
			// else, append the team's name to the string
			else{
				strcat(GAUpdate, get_neuteam(codedrop));
				GAUpdateStruct->beginFlag = 1;
			}
		}
	}
}




/*********** gs_capture_notif **************/
/** Helper function which sends an update to the agent who is about to be captured
* Takes in the infostore struct, the address of the captured agent, the capture ID and the current gameID
* Sends the message to the agent.
*/

static void gs_capture_notif(info_t* infostore, struct sockaddr_in themp, char* captureID, char* gameID)
{
	int commSock = get_commsock(infostore);
	time_t currTime = get_curr_time(infostore);
	double elapsedTime = get_elapsed_time(infostore);

	// create a string to hold the information
	char* captureNotif = assertp(count_calloc(MAXSTR, sizeof(char)), "Memory allocation for captureNotif failed.\n");
	sprintf(captureNotif, "%s|%s|%s", gsCaptureID, gameID, captureID);

	// send string to agent who is about to be captured
	int datagram_check;
	if ((datagram_check = send_datagram(commSock, &themp, captureNotif)) != 0)
			fprintf(stderr, "Datagram failed to send\n");

	if (logMode == 1)
		raw_mode_logging_outbound(captureNotif, currTime, elapsedTime);

	if (captureNotif != NULL)
		count_free(captureNotif);
	
}

/************************* Validation of Reponse Codes ***************************/

/*********** validate_gameID **************/
/** Helper function which validates the gameID 
* If the gameID is valid - if it's 8 or less hex digits - then return true.
* Else, return false
*/

static bool validate_gameID(char* message)
{
	unsigned int gameID;

	if (message != NULL){
		// checks if the length of the gameID is longer than 8, if so, return false
		if (strlen(message) > 8)
			return false;

		// checks if the gameID is a hexadecimal 
		if ((sscanf(message,"%X",&gameID)) != 1){
			fprintf(stderr, "gameID is invalid. Please check the gameID\n");
			char* valgameIDErr = "validate_gameID: Please check that the gameID is valid";
			error_logging(valgameIDErr);
			return false;
		}
		return true;
	}
	else
		return false;
}

/*********** validate_guideID **************/
/** Helper function which validates the guideID 
* If the guideID is valid - if it's not 8 hex digits - then return true.
* Else, return false.
*/

static bool validate_guideID(char* message)
{
	unsigned int guideID;

	if (message != NULL){
		// checks if the length of the guideID is not 8, if so, return false
		if (strlen(message) != 8)
			return false;

		// checks if the guideID is a hexadecimal 
		if ((sscanf(message,"%X",&guideID)) != 1){
			fprintf(stderr, "guideID is invalid. Please check the guideID\n");
			char* valguideIDerr = "validate_guideID: Please check that the guideID is valid";
			error_logging(valguideIDerr);
			return false;
		}
		return true;
	}
	else 
		return false;
}

/*********** validate_teamName **************/
/** Helper function which validates the teamName 
* If the gameID is 0, then the game is just starting, therefore any teamName will be sound, return True
* If the gameID is not 0, then there should be team in the hashtable,
* If the team is not found using the supplied teamName, then return false
* Else, return true.
*/

static bool validate_teamName(char** messageArr, char* message, hashtable_t* teamHash)
{
	if (messageArr != NULL && message != NULL){
		if (strcmp(messageArr[1], "0") != 0){
			team_t* team = hashtable_find(teamHash, message);
			if (team == NULL)
				return false;
			else
				return true;
		}
		 return true;
	}
	else
		return false; 
}

/*********** validate_playerName **************/
/** Helper function which validates the FA's player name 
* If the gameID is 0, then the game is just starting, therefore any player name will be sound, return True
* If the gameID is not 0, then there should be team in the hashtable,
* If the team is not found using the supplied teamName, then return false
* Else, return true.
*/

static bool validate_playerName(char* gameID, char* pebbleID, char* agentName, hashtable_t* agentHash)
{
	if (gameID != NULL && agentName != NULL){
		if (strcmp(gameID, "0") != 0){
			agent_t* agent = hashtable_find(agentHash, pebbleID);
			char* playerName = get_player_name(agent);
			if (strcmp(playerName, agentName) != 0){
				return false;
			}
			else
				return true;
		}
		 return true;
	}
	
	return false; 
}


/**************************************************************** OPCODE HANDLING ************************************************************/

/*********** update_fa_location **************/
/* Handles OPCODE - FA_LOCATION 
* Takes in the message from the FA to the GS which updates the location and the status of the FA
* Also allows the FA to receive an update from the GS regarding the status and state of the game
* Takes in the infostore struct as a parameter
* If there is an error, throw an error and return a non-zero return code
* Else, return 0.
*/

static int update_fa_location(info_t* infostore)
{
	//Check if the array length matches the number of elements for FA_LOC opcode
	if (get_arr_len(infostore) != FA_LOC_LEN){
		fprintf(stderr, "Invalid message length\n");
		char* updateLocErr = "update_fa_location: Please ensure that the message length is valid\n";
		error_logging(updateLocErr);
		return 1; 
	}	

	char** message = get_msg_arr(infostore);
	
	// validate GameID
	char* msgGameID = message[1];
	if (!validate_gameID(msgGameID)){
		invalid_gameid(infostore);
		return 2;
	}

	if (strcmp(msgGameID,get_gameid(infostore)) != 0 && strcmp(msgGameID, "0") != 0) {
		invalid_gameid(infostore);	
		return 2;
	}

	// validate pebbleID
	char* pebbleID = message[2];
	if (strlen(pebbleID) != PEBBLE_LEN){
		printf(stderr, "pebbleID is invalid. Please check the pebbleID. \n");
		char* updateLocErr = "update_fa_location: Please ensure that the pebbleID is valid\n";
		error_logging(updateLocErr);
		invalid_genericID(infostore);
		return 2;
	}

	//validate teamName
	char* teamName = assertp(count_calloc(MAXSTR, sizeof(char)), "Memory allocation for serverResponse failed.\n");
	if ((sscanf(message[3],"%s",teamName)) != 1){
		count_free(teamName);
		invalid_teamname(infostore);
		char* updateLocErr = "update_fa_location: Please ensure that the teamName is valid\n";
		error_logging(updateLocErr);
		fprintf(stderr, "teamName is invalid. Please check the teamName\n");
		return 2;
	}

	// check if the teamName is in the hashtable, else throw an error
	hashtable_t* teamHash = get_team_hash(infostore);
	team_t* team = hashtable_find(teamHash,teamName);

	if (! validate_teamName(message, teamName, teamHash)){
		invalid_teamname(infostore);
		count_free(teamName);
		char* updateLocErr = "update_fa_location: Please ensure that the teamName is valid\n";
		error_logging(updateLocErr);
		fprintf(stderr, "teamName is invalid. Please check the teamName\n");
		return 2;
	}

	// validate PlayerName
	hashtable_t* agentHash = get_agent_hash(infostore);
	agent_t* currAgent = hashtable_find(agentHash, message[2]);
	char* playerName = assertp(count_calloc(MAXSTR, sizeof(char)), "Memory allocation for serverResponse failed.\n");
	
	if ((sscanf(message[4],"%s",playerName)) != 1){
		count_free(teamName);
		count_free(playerName);
		invalid_genericID(infostore);
		char* updateLocErr = "update_fa_location: Please ensure that the playerName is valid\n";
		error_logging(updateLocErr);
		fprintf(stderr, "playerName is invalid. Please check the playerName\n");
		return 2;		
	}

	if (! validate_playerName(msgGameID, pebbleID, playerName, agentHash)){
		count_free(teamName);
		count_free(playerName);
		invalid_genericID(infostore);
		char* updateLocErr = "update_fa_location: Please ensure that the playerName is valid\n";
		error_logging(updateLocErr);
		fprintf(stderr, "playerName is invalid. Please check the playerName\n");
		return 2;	
	}

	float faLat;
	float faLong;
	int statusReq;

	sscanf(message[5],"%f", &faLat);
	sscanf(message[6], "%f", &faLong);
	sscanf(message[7], "%d", &statusReq);

	
	time_t currTime = get_curr_time(infostore);
	struct sockaddr_in themp = get_themp(infostore);



	// If GameID is 0 (if the field agent just joined)
	if (strcmp(msgGameID, "0") == 0){

		if ( currAgent == NULL){ // if current agent has not yet been made
			char* currGameID = get_gameid(infostore);
			agent_t* newAgent = agent_new(currGameID,pebbleID,teamName,playerName, faLat, faLong, FAActive, themp);
			if (team != NULL)
				add_num_FA_Ops_Active(team); // increment the number of active FAs in this team 
			
			// insert new field agent into the hashtable
			hashtable_insert(agentHash,message[2],newAgent); 


			// If the logmode is default (game-mode, then, log it)
			if (logMode == 0){
				char* playerJoinedMessage = assertp(count_calloc(MAXSTR, sizeof(char)), "Memory allocation for player joined message failed.\n");
				sprintf(playerJoinedMessage, "Another Champ Joins The Arena. Ding! Ding! Ding! \nVital Stats: \nPebbleID: %s \nPlayer Name:%s \nTeam Name:%s\n", pebbleID, playerName, teamName);
				double elapsedTime = get_elapsed_time(infostore);
				game_mode_logging(newPlayerJoined, playerJoinedMessage, elapsedTime);

			}
		}

		// send game status message back to fa on request 

		if (statusReq ==1)
			send_game_status_fa(infostore, teamName);
	}
	else{

		if (currAgent != NULL){
			if (strcmp(FACaptured, get_player_status(currAgent)) == 0){
				if (statusReq == 1){
				// send Game Status message back to FA
					send_game_status_fa(infostore, teamName);
				}
			}
			else{
				//update the location, time and sockaddr_in of these players
				update_loc(currAgent, faLat, faLong);
				update_last_cont(currAgent, currTime);
				update_player_soc(currAgent, themp);

				// check the status of the player
				time_t captureStartTime = get_capture_time_start(currAgent);
				double captureTimeDiff = difftime(currTime, captureStartTime);

				// if it has been more than 60s since the capture request, then reset the player status to Active
				// from 'capturing' or 'maybe captured'

				if (captureTimeDiff > 60){
					update_player_status(currAgent, FAActive);
				}

				if (statusReq == 1){
				// send Game Status message back to FA
					send_game_status_fa(infostore, teamName);
				}
			}
		}
	}

	count_free(playerName);
	count_free(teamName);


	return 0;

}

/*********** neutralize_code **************/
/** Takes in a message from the FA to GS to update if the FA has managed to neutralize a code
* Takes in the infostore struct as a parameter
* If there is an error, throw an error and return a non-zero return code
* Else, return 0.
* If the agent is successful, respond with a message. Else, ignore the FA.
*/

static int neutralize_code(info_t* infostore)
{
	// validate gameID, pebbleID, teamName and playerName
	//Check if the array length matches the number of elements for FA_NEU opcode

	if (get_arr_len(infostore) != FA_NEU_LEN){
		char* neutCodeErr = "neutralize_code: Please ensure that msg length is valid.\n";
		error_logging(neutCodeErr);
		fprintf(stderr, "Invalid message length\n");
		return 1; 
	}	

	char** message = get_msg_arr(infostore);

	// validate GameID
	char* msgGameID = message[1];

	if (! validate_gameID(msgGameID)){
		invalid_gameid(infostore);
		return 2;
	}

	if (strcmp(msgGameID,get_gameid(infostore)) != 0 && strcmp(msgGameID, "0") != 0) {
		invalid_gameid(infostore);	
		return 2;
	}

	// validate pebbleID
	char* pebbleID = message[2];
	if (strlen(pebbleID) != PEBBLE_LEN){
		char* neutCodeErr = "neutralize_code: Please ensure that pebbleID is valid.\n";
		error_logging(neutCodeErr);
		printf(stderr, "pebbleID is invalid. Please check the pebbleID. \n");
		invalid_genericID(infostore);
		return 2;
	}

	// check if the FA exists in Hashtable

	hashtable_t* agentHash = get_agent_hash(infostore);
	agent_t* currAgent = hashtable_find(agentHash, pebbleID);

	if (currAgent == NULL){
		printf(stderr, "pebbleID is invalid. Please check the pebbleID. \n");
		char* neutCodeErr = "neutralize_code: Please ensure that pebbleID is valid.\n";
		error_logging(neutCodeErr);
		invalid_genericID(infostore);
		return 2;
	}

	//validate teamName
	char* teamName = assertp(count_calloc(MAXSTR, sizeof(char)), "Memory allocation for team name failed.\n");
	
	if ((sscanf(message[3],"%s",teamName)) != 1){
		invalid_teamname(infostore);
		count_free(teamName);
		char* neutCodeErr = "neutralize_code: Please ensure that teamName is valid.\n";
		error_logging(neutCodeErr);
		fprintf(stderr, "teamName is invalid. Please check the teamName\n");
		return 2;
	}

	// check if the teamName is in the hashtable, else throw an error
	hashtable_t* teamHash = get_team_hash(infostore);
	team_t* team = hashtable_find(teamHash,teamName);
	
	if (!validate_teamName(message, teamName, teamHash)){
		invalid_teamname(infostore);
		count_free(teamName);
		char* neutCodeErr = "neutralize_code: Please ensure that teamName is valid.\n";
		error_logging(neutCodeErr);
		fprintf(stderr, "teamName is invalid. Please check the teamName\n");
		return 2;
	}

	// validate PlayerName
	char* playerName = assertp(count_calloc(MAXSTR, sizeof(char)), "Memory allocation for player name failed.\n");
	
	if ((sscanf(message[4],"%s",playerName)) != 1){
		invalid_genericID(infostore);
		count_free(playerName);
		count_free(teamName);
		char* neutCodeErr = "neutralize_code: Please ensure that playerName is valid.\n";
		error_logging(neutCodeErr);
		fprintf(stderr, "playerName is invalid. Please check the playerName\n");
		return 2;		
	}

	if (! validate_playerName(msgGameID, pebbleID, playerName, agentHash)){
		invalid_genericID(infostore);
		count_free(playerName);
		count_free(teamName);
		char* neutCodeErr = "neutralize_code: Please ensure that playerName is valid.\n";
		error_logging(neutCodeErr);
		fprintf(stderr, "playerName is invalid. Please check the playerName\n");
		return 2;		
	}

	// Confirm that the codeID matches a known codeID
	hashtable_t* codeHash = get_code_hash(infostore);
	char* currCodeID = message[7];

	// make the codeID upperCase
	for( int x = 0; currCodeID[x] != '\0'; x++){
		if (isdigit(currCodeID[x]) == 0)
			currCodeID[x] = toupper(currCodeID[x]);
	}

	codedrop_t* codeDrop;
	if ((codeDrop = hashtable_find(codeHash,currCodeID)) != NULL){
		char* codeStatus = get_code_status(codeDrop);
		if (strcmp(codeStatus,codeNeutralized)!= 0){

			// get code lat and code long for FA and the codedrop
			float codeLat = get_code_lat(codeDrop);
			float codeLong = get_code_long(codeDrop);
			float FALat = get_agent_lat(currAgent);
			float FALong = get_agent_long(currAgent);

			// ensure that the field agent is within 10 meters of the codedrop
			if (distance(codeLat, codeLong, FALat, FALong) <= 10){ 
				update_code_status(codeDrop, teamName, codeNeutralized);
				code_neutralized(infostore); // send a MI neutralized message to the field agent
				add_codedrops_neut(team);

				if (logMode == 0){
					char* codeNeutralized = assertp(count_calloc(MAXSTR, sizeof(char)), "Memory allocation for player joined message failed.\n");
					sprintf(codeNeutralized, "Code Neutralized. A Team Inches Closer to Victory! \n CodeDrop ID: %s, Neutralizing Team: %s, CodeDrop Lat: %f, CodeDrop Long: %f\n", currCodeID, teamName, codeLat, codeLong);
					double elapsedTime = get_elapsed_time(infostore);
					game_mode_logging(codeDropNeut, codeNeutralized, elapsedTime);

				}
			}
			else{
				count_free(playerName);
				count_free(teamName);
				return 3;
			}
		}
		else{
			// ignore the message
			count_free(playerName);
			count_free(teamName);
			return 3;
		}
	}
	else{
		//ignore the message
		count_free(playerName);
		count_free(teamName);
		return 3;
	}

	count_free(playerName);
	count_free(teamName);

	return 0;
}


/** FA_CAPTURE **/

/*********** capture_player **************/
/** Takes in a message from the FA to GS to update if:
* 1. the Player wants to capture another player
* 2. the Player is being captured
* 
* Takes in the infostore struct as a parameter
* If there is an error, throw an error and return a non-zero return code
* Else, return 0.
* If the agent is requesting capture, respond with a capture message to surrounding foe agents within 10m. 
* If the agent is being captured, check to see if the capture code matches with the one given within 60s ago by the capturing agent
* If these two capture codes match, the capturing agent is successful and the target agent is captured.
*/


static int capture_player(info_t* infostore)
{
	// validate gameID, pebbleID, teamName and playerName

	//Check if the array length matches the number of elements for FA_NEU opcode
	if (get_arr_len(infostore) != FA_CAP_LEN){
		char* captPlayerErr = "capture_player: Please ensure that msg length is valid.\n";
		error_logging(captPlayerErr);
		fprintf(stderr, "Invalid message length\n");
		return 1; 
	}	

	char** message = get_msg_arr(infostore);

	// validate GameID
	char* msgGameID = message[1];

	if (! validate_gameID(msgGameID))
		return 2;
	
	if (strcmp(msgGameID,get_gameid(infostore)) != 0 && strcmp(msgGameID, "0") != 0) {
		invalid_gameid(infostore);	
		return 2;
	}

	// validate pebbleID
	char* pebbleID = message[2];
	if (strlen(pebbleID) != PEBBLE_LEN){
		invalid_genericID(infostore);
		char* captPlayerErr = "capture_player: Please ensure that pebbleID is valid.\n";
		error_logging(captPlayerErr);
		printf(stderr, "pebbleID is invalid. Please check the pebbleID. \n");
		return 2;
	}

	// check if the FA exists in Hashtable
	hashtable_t* agentHash = get_agent_hash(infostore);
	agent_t* currAgent = hashtable_find(agentHash, pebbleID);

	if (currAgent == NULL){
		invalid_genericID(infostore);
		char* captPlayerErr = "capture_player: Please ensure that pebbleID is valid.\n";
		error_logging(captPlayerErr);
		printf(stderr, "pebbleID is invalid. Please check the pebbleID. \n");
		return 2;
	}
	//validate teamName
	char* teamName = assertp(count_calloc(MAXSTR, sizeof(char)), "Memory allocation for team name failed.\n");
	if ((sscanf(message[3],"%s",teamName)) != 1){
		invalid_teamname(infostore);
		count_free(teamName);
		char* captPlayerErr = "capture_player: Please ensure that teamName is valid.\n";
		error_logging(captPlayerErr);
		fprintf(stderr, "teamName is invalid. Please check the teamName\n");
		return 2;
	}

	// check if the teamName is in the hashtable, else throw an error
	hashtable_t* teamHash = get_team_hash(infostore);
	
	if (!validate_teamName(message, teamName, teamHash)){
		invalid_teamname(infostore);
		count_free(teamName);
		char* captPlayerErr = "capture_player: Please ensure that teamName is valid.\n";
		error_logging(captPlayerErr);
		fprintf(stderr, "teamName is invalid. Please check the teamName\n");
		return 2;
	}

	// validate PlayerName
	char* playerName = assertp(count_calloc(MAXSTR, sizeof(char)), "Memory allocation for player name failed.\n");
	if ((sscanf(message[4],"%s",playerName)) != 1){
		count_free(teamName);
		count_free(playerName);
		invalid_genericID(infostore);
		char* captPlayerErr = "capture_player: Please ensure that playerName is valid.\n";
		error_logging(captPlayerErr);
		fprintf(stderr, "playerName is invalid. Please check the playerName\n");
		return 2;		
	}

	if (! validate_playerName(msgGameID, pebbleID, playerName,agentHash)){
		count_free(teamName);
		count_free(playerName);
		invalid_genericID(infostore);
		char* captPlayerErr = "capture_player: Please ensure that playerName is valid.\n";
		error_logging(captPlayerErr);
		fprintf(stderr, "playerName is invalid. Please check the playerName\n");
		return 2;		
	}

	list_t* captureList = get_capture_list(infostore);

	// Deal with CaptureID
	char* msgCaptureID = assertp(count_calloc(8*10, sizeof(int)), "Memory allocation for msgCaptureID failed.\n");
	if ((sscanf(message[5],"%s", msgCaptureID)) != 1){
		count_free(teamName);
		count_free(playerName);
		char* captPlayerErr = "capture_player: Please ensure that captureID is valid.\n";
		error_logging(captPlayerErr);
		fprintf(stderr, "msgCaptureID is invalid. Please check the msgCaptureID\n");
		return 2;	
	}


	// make the CaptureID upperCase
	for( int x = 0; msgCaptureID[x] != '\0'; x++){
		if (isdigit(msgCaptureID[x]) == 0)
			msgCaptureID[x] = toupper(msgCaptureID[x]);
	}


	int msgCaptureIDint = atoi(msgCaptureID);
	// If captureID is 0,

	if (msgCaptureIDint == 0){

		find_foe_ops(infostore, teamName, currAgent, captureList);
		update_player_status(currAgent, FACapturing);
		time_t captureStartTime = get_curr_time(infostore);
		update_capture_time_start(currAgent, captureStartTime);
		count_free(msgCaptureID);
	}
	// If CaptureID is nonZero

	if (msgCaptureIDint != 0){
		time_t captureTimeEnd = get_curr_time(infostore);
		struct captureStruct* currentCapture = list_find(captureList, msgCaptureID);
		
		if (currentCapture != NULL){
			time_t captureTimeStart = currentCapture->captureTimeStart;

			if ((captureTimeEnd - captureTimeStart) <= 60){ // if its within 60s
				
				capture_success(currAgent,infostore); // send capture success message to the capturing agent
				update_player_status(currAgent, FAActive);

				agent_t* capturedAgent = currentCapture->capturedAgent; 
				char* capturedAgentName = get_player_name(capturedAgent);
				char* captAgentPebbleID = get_pebble_ID(capturedAgent);
				agent_t* capturedAgentActual = hashtable_find(agentHash, captAgentPebbleID);
				captured(capturedAgent, infostore);
				update_player_status(capturedAgentActual, FACaptured); //update the captured agent's status in the team
			
				
				if (logMode == 0){
					char* playerCapt = assertp(count_calloc(MAXSTR, sizeof(char)), "Memory allocation for player joined message failed.\n");
					char* capturedAgentTeamName = get_team_name_agent(capturedAgent);
					sprintf(playerCapt, "Tango Down! Another Player Captured! A Team Inches Closer to Victory!\n Captured Player: %s, Captured Player's Team: %s, Capturing Player: %s, Capturing Player Team: %s\n", capturedAgentName, capturedAgentTeamName, playerName, teamName);
					double elapsedTime = get_elapsed_time(infostore);
					game_mode_logging(playerCaptured, playerCapt, elapsedTime);

				}

			}
		}
		// if there is no matching captureID, ignore the message
		count_free(msgCaptureID);
	}

	count_free(playerName);
	count_free(teamName);

	return 0; 
}

/*********** find_foe_ops **************/
/** Helper Function to find all Foe Field Agents within 10m of current field agent
* Sends message to all eligible foe field agents using an iterator.
*/

static struct listOperatives find_foe_ops(info_t* infostore, char* teamName, agent_t* agent, list_t* captureList)
{
	struct listOperatives listFoeOps = {teamName, infostore, agent, captureList}; // store pertinent information in a struct
	hashtable_t* agentHash = get_agent_hash(infostore);
	hashtable_iterate(agentHash, find_foe_ops_helper, &listFoeOps); // iterate over agent hashtable with the helper function
	
	return listFoeOps;
}

/*********** find_foe_ops_helper **************/
/** Helper Function to find all Foe Field Agents within 10m of current field agent
* Sends message to all eligible foe field agents using an iterator.
*/

static void find_foe_ops_helper(void* arg, char* key, void* data)
{
	struct listOperatives *listOps = arg;

	if (listOps != NULL && data != NULL){
		agent_t* agent = data;
		list_t* captureList = listOps->captureList;
		

		// Foe's TeamName
		char* agentTeamName = get_team_name_agent(agent);
		char* agentStatus = get_player_status(agent);
		agent_t* capturingAgent = listOps->capturingAgent;

		//Get capturing Player's Lat and Long
		float captAgentLat = get_agent_lat(capturingAgent);
		float captAgentLong = get_agent_long(capturingAgent);

		if (strcmp(agentTeamName, listOps->teamName) != 0){

			if (strcmp(agentStatus, FAActive) == 0){
				//Get Current Foe Agent's lat and long
				float foeAgentLat = get_agent_lat(agent);
				float foeAgentLong = get_agent_long(agent);

				// if the agents are within 10m of each other
				if (distance(captAgentLat, captAgentLong, foeAgentLat, foeAgentLong) <= 10){

					struct captureStruct* newCapture = assertp(count_calloc(1, sizeof(struct captureStruct)), "Memory allocation for capture struct failed.\n");
					char* captureID = gen_random_hexcode(4); // generate captureCode
					char* captureIDCopy = assertp(count_calloc(strlen(captureID)*2+10, sizeof(struct captureStruct)), "Memory allocation for capture struct failed.\n");

					// create a new agent to store in the captureList
					agent_t* cloneAgentCaptureList = copy_agent(agent);
					agent_t* capturingAgentCopy = copy_agent(capturingAgent);
					strcpy(captureIDCopy,captureID);

					// update the player status
					info_t* infostore = listOps->infostore;
					struct sockaddr_in themp = get_agent_soc(agent);
					char* gameID = get_game_id(agent);
					gs_capture_notif(infostore, themp, captureID, gameID);
					update_player_status(agent,mbCaptured); // update Foe Agent status to be almost captured

					time_t captureTimeStart = get_curr_time(infostore);

					// update the capture start time for the player who is almost captured
					update_capture_time_start(agent, captureTimeStart);

					// put it into the struct
					newCapture->capturingAgent = capturingAgentCopy; 
					newCapture->capturedAgent = cloneAgentCaptureList;
					newCapture->captureTimeStart = captureTimeStart;
					newCapture->captureID = captureIDCopy;

					list_insert (captureList, captureID, newCapture);

					if (captureID != NULL)
						count_free(captureID);	
				}
			}
		}
	}
}




/************** GA_STATUS *********************/


/*********** update_GA **************/
/** Function which is called when GA_STATUS opcode is triggered
* Takes in the infostore struct containing the pertinent information for the game
* If the Guide is announcing its presence to the game for the first time, then create a new team 
* If the Guide requests a status update, send status update to guide
* Updates information about the guide as well
*/

static int update_GA(info_t* infostore)
{
	// validate gameID, guideID, teamName and playerName

	//Check if the array length matches the number of elements for GA stat opcode
	if (get_arr_len(infostore) != GA_STAT_LEN){
		char* updateGAErr = "update_GA: Please ensure that msg length is valid.\n";
		error_logging(updateGAErr);
		fprintf(stderr, "Invalid message length\n");
		return 1; 
	}	

	char** message = get_msg_arr(infostore);

	// validate GameID
	char* msgGameID = message[1];
	int msgGameIDInt = atoi(msgGameID);

	if (! validate_gameID(msgGameID))
		return 2;
	
	if (strcmp(msgGameID,get_gameid(infostore)) != 0 && strcmp(msgGameID, "0") != 0) {
		invalid_gameid(infostore);	
		return 2;
	}

	// validate guideID
	char* guideID = message[2];

	if (! validate_guideID(message[2])){
		char* updateGAErr = "update_GA: Please ensure that guideID is valid.\n";
		error_logging(updateGAErr);
		printf(stderr, "guideID is invalid. Please check the guideID. \n");
		invalid_genericID(infostore);
		return 2;
	}

	//validate teamName
	char* teamName = assertp(count_calloc(MAXSTR, sizeof(char)), "Memory allocation for team name failed.\n");
	if ((sscanf(message[3],"%s",teamName)) != 1){
		invalid_teamname(infostore);
		count_free(teamName);
		char* updateGAErr = "update_GA: Please ensure that teamName is valid.\n";
		error_logging(updateGAErr);
		fprintf(stderr, "teamName is invalid. Please check the teamName\n");
		return 2;
	}

	// check if the teamName is in the hashtable, else throw an error
	hashtable_t* teamHash = get_team_hash(infostore);
	
	if (!validate_teamName(message, teamName, teamHash)){
		invalid_teamname(infostore);
		count_free(teamName);
		char* updateGAErr = "update_GA: Please ensure that teamName is valid.\n";
		error_logging(updateGAErr);
		fprintf(stderr, "teamName is invalid. Please check the teamName\n");
		return 2;
	}

	// validate PlayerName
	char* playerName = assertp(count_calloc(MAXSTR, sizeof(char)), "Memory allocation for player name failed.\n");
	if ((sscanf(message[4],"%s",playerName)) != 1){
		count_free(teamName);
		count_free(playerName);
		char* updateGAErr = "update_GA: Please ensure that playerName is valid.\n";
		error_logging(updateGAErr);
		fprintf(stderr, "playerName is invalid. Please check the playerName\n");
		return 2;		
	}

	// Get StatusReq

	int statusReq;
	sscanf(message[5], "%d", &statusReq);

	// Check if the GuideID
	if (msgGameIDInt == 0){
		
		hashtable_t* teamHash = get_team_hash(infostore);
		team_t* team = hashtable_find(teamHash, teamName); 

		// if guideID is not yet known
		if (!check_guideID(infostore, guideID)){

			if (team == NULL){
				// add new Team + Guide to Team Hashtable

				struct sockaddr_in gaSocInfo = get_themp(infostore);
				time_t currTime = get_curr_time(infostore);
				team_t* newTeam = team_new(guideID, msgGameID, teamName, GAActive, gaSocInfo, currTime);
				hashtable_insert(teamHash, teamName, newTeam);
			}
		}
		else{ // if guideID is known but the team is not found, then send an invalid teamname message to the Guide Agent
			if (team == NULL){
				count_free(teamName);
				count_free(playerName);
				invalid_teamname(infostore);
				fprintf(stderr, "Team Is NULL\n");
				return 3;
			}

			// checks if the teamName matches up with known team name, if not, send a message to the offending agent
			if (strcmp(get_team_name(team),teamName) != 0){ 
				count_free(teamName);
				count_free(playerName);
				invalid_teamname(infostore);
				char* updateGAErr = "update_GA: Given teamName does not match guideID's teamName.\n";
				error_logging(updateGAErr);
				fprintf(stderr, "Given teamName does not match guideID's teamName\n");
				return 3;
			}
		}
	}
	team_t* team = hashtable_find(teamHash, teamName);

	time_t currTime = get_curr_time(infostore);
	struct sockaddr_in gaSocInfo = get_themp(infostore);

	// Update time + socket information of GA

	update_guide_ctime(team,currTime);
	update_GASoc(team,gaSocInfo);

	if (statusReq == 1)
		send_game_status_ga(infostore);
	

	count_free(playerName);
	count_free(teamName);

	return 0;
}

/*********** check_guideID **************/
/** Helper Function to check if guide ID exists in the team hashtable or not
*
*/
static bool check_guideID(info_t* infostore, char* guideID)
{
	hashtable_t* teamHash = get_team_hash(infostore);

	struct guideIDCheck guideIDch = {guideID, 0};
	
	hashtable_iterate(teamHash, guideIDiter, &guideIDch);

	if (guideIDch.guideIDPresent == 0)
		return false;
	else
		return true;
}


/*********** guideIDiter **************/
/** Helper Function to check if guide ID exists in the team hashtable or not.
* Iterates over the teamHashtable to check if guideID exists.
*
*/
static void guideIDiter(void* arg, char* key, void* data)
{
	struct guideIDCheck* guideIDch = arg;

	if (guideIDch != NULL && data != NULL){
		team_t* team = data;
		char* guideID = guideIDch->guideID;

		if (team != NULL){
			if (strcmp(get_guide_ID(team), guideID) == 0) // check to see if guideID in the team hashtable matches with the current team 
				guideIDch->guideIDPresent = 1;
		}
	}
}



/********************* GA_HINT ****************************/

/*********** send_hint **************/
/** Function which is called when GA_HINT opcode is triggered
* Takes in the infostore struct containing the pertinent information for the game
* It can send a code to an individual FA or to all FAs on the same team. 
* Takes a message from the guide agent for the team and sends it to the FA(s).
* If there are errors, return a non-zero value. Else, return 0.
*/


static int send_hint(info_t* infostore)
{
	// validate gameID, guideID, teamName and playerName

	//Check if the array length matches the number of elements for FA_NEU opcode
	if (get_arr_len(infostore) != GA_HINT_LEN){
		char* sendHintErr = "send_hint: Invalid Msg Length.\n";
		error_logging(sendHintErr);
		fprintf(stderr, "Invalid message length\n");
		return 1; 
	}	

	char** message = get_msg_arr(infostore);

	// validate GameID
	char* msgGameID = message[1];

	if (! validate_gameID(msgGameID))
		return 2;
	
	if (strcmp(msgGameID,get_gameid(infostore)) != 0 || strcmp(msgGameID, "0") == 0) {
		invalid_gameid(infostore);	
		return 2;
	}

	// validate guideID
	char* guideID = message[2];

	if (! validate_guideID(message[2])){
		printf(stderr, "guideID is invalid. Please check the guideID. \n");
		char* sendHintErr = "send_hint: Invalid pebbleID\n";
		error_logging(sendHintErr);
		invalid_genericID(infostore);
		return 2;
	}

	//validate teamName
	char* teamName = assertp(count_calloc(MAXSTR, sizeof(char)), "Memory allocation for team name failed.\n");
	if ((sscanf(message[3],"%s",teamName)) != 1){
		invalid_teamname(infostore);
		count_free(teamName);
		char* sendHintErr = "send_hint: Invalid teamName\n";
		error_logging(sendHintErr);
		fprintf(stderr, "teamName is invalid. Please check the teamName\n");
		return 2;
	}

	hashtable_t* teamHash = get_team_hash(infostore);
	hashtable_t* agentHash = get_agent_hash(infostore);

	if (!validate_teamName(message, teamName, teamHash)){
		invalid_teamname(infostore);
		count_free(teamName);
		char* sendHintErr = "send_hint: Invalid teamName\n";
		error_logging(sendHintErr);
		fprintf(stderr, "teamName is invalid. Please check the teamName\n");
		return 2;
	}

	// validate PlayerName
	char* playerName = assertp(count_calloc(MAXSTR, sizeof(char)), "Memory allocation for player name failed.\n");
	if ((sscanf(message[4],"%s",playerName)) != 1){
		invalid_genericID(infostore);
		count_free(teamName);
		count_free(playerName);
		char* sendHintErr = "send_hint: Invalid playerName\n";
		error_logging(sendHintErr);
		fprintf(stderr, "playerName is invalid. Please check the playerName\n");
		return 2;		
	}

	// Validate pebbleID
	char* pebbleID = message[5];
	agent_t* currAgent = hashtable_find(agentHash,pebbleID);

	if (currAgent == NULL && strcmp(pebbleID,"*") != 0){
		invalid_genericID(infostore);
		count_free(teamName);
		count_free(playerName);
		return 3;
	}

	char* GAHINT = "GA_HINT";
	char* hint = assertp(count_calloc(MAXSTR, sizeof(char)), "Memory allocation for hint failed.\n");
	char* messageHint = message[6];
	sprintf(hint, "%s|%s|%s|%s|%s|%s|%s", GAHINT, msgGameID, guideID, teamName, playerName, pebbleID, messageHint);

	// if the receipient is designated to be *, then send a message to all the friendly operatives in the team
	if (strcmp(pebbleID,"*") == 0){
		send_all_friendlyOp(infostore, hint, teamName);
	}

	else{
		
		// send it to the specific agent who requested it 

		int commSock = get_commsock(infostore);
		time_t currTime = get_curr_time(infostore);
		double elapsedTime = get_elapsed_time(infostore);

		struct sockaddr_in agentSoc = get_agent_soc(currAgent);
		int datagram_check;

		/*** Game Over HARDCODE ****/
		struct sockaddr_in them = get_themp(infostore);
		char* gameover_hardcode = "GAME_OVER|86CC0663|23|Hackers,3,1,1,0:Artists,3,0,0,1";
		if ((datagram_check = send_datagram(commSock, &them, gameover_hardcode)) != 0){
			fprintf(stderr, "Datagram failed to send\n");
		}
		/********************/

		
		if ((datagram_check = send_datagram(commSock, &agentSoc, hint)) != 0)
			fprintf(stderr, "Datagram failed to send\n");

		if (logMode == 1)
			raw_mode_logging_outbound(hint, currTime, elapsedTime);
	}
	// update the last contact time of the guide agent

	team_t* currTeam = hashtable_find(teamHash, teamName);
	time_t currTime = get_curr_time(infostore);
	update_guide_ctime(currTeam, currTime);

	count_free(teamName);
	count_free(playerName);

	if (hint != NULL)
		count_free(hint);

	return 0;
}

/*********** send_all_friendlyOp **************/
/** Helper function which sends the messages to all the friendly FAs
*/

static void send_all_friendlyOp(info_t* infostore, char* message, char* teamName)
{
	hashtable_t* agentHash = get_agent_hash(infostore);
	struct sendHint sendHintHelper = {infostore, message, teamName};
	hashtable_iterate(agentHash, send_hint_iter, &sendHintHelper);
}

/*********** send_hint_iter **************/
/** Helper function which iterates through the agent hashtable,
* Checks if the agents are friendly, if so, send the hint to them 
*/

static void send_hint_iter(void* arg, char* key, void* data)
{
	struct sendHint *sendHintHelper = arg;

	if (sendHintHelper != NULL && data != NULL){

		// get the pertinent information

		agent_t* currAgent = data;
		info_t* infostore = sendHintHelper->infostore;
		char* teamName = sendHintHelper->teamName;
		char* currTeamName = get_team_name_agent(currAgent);
		char* message = sendHintHelper->message;
		time_t currTime = get_curr_time(infostore);
		double elapsedTime = get_elapsed_time(infostore);

		
		// if agents are on the same team as GA, send hint out

		if (strcmp(currTeamName, teamName) == 0){
			int commSock = get_commsock(infostore);
			struct sockaddr_in currAgentSoc = get_agent_soc(currAgent);
			int datagram_check;

			if ((datagram_check = send_datagram(commSock, &currAgentSoc, message)) != 0)
					fprintf(stderr, "Datagram failed to send\n");
			
			if (logMode == 1)
				raw_mode_logging_outbound(message, currTime, elapsedTime);
		}
	}
}


/**************************************** GAME_OVER **********************************/

/*********** game_over **************/
/** Function which is called when GAME_OVER code is triggered
* Sends all agents (both guide and field) a message, summarizing the game stats
*/

static void game_over(info_t* infostore)
{	
	char* gameOverMsg = assertp(count_calloc(MAXSTR, sizeof(char)), "Memory allocation for gameOverMsg failed.\n");
	char* gameID = get_gameid(infostore);

	// find the number of codedrops yet to be neutralized
	hashtable_t* codeHash = get_code_hash(infostore);
	int yetNeutCount = 0;
	hashtable_iterate(codeHash, yetNeutIter, &yetNeutCount);
	
	sprintf(gameOverMsg, "%s|%s|%d|", gameOverOpCode, gameID, yetNeutCount);

	// create a struct containing game over information as well as the game over message to be sent to all receipients

	struct gameOverInfo gameOvr = {infostore, gameOverMsg, 0};

	// Update Individual Team Records
	hashtable_t* teamHash = get_team_hash(infostore);
	hashtable_iterate(teamHash, teamRecordIter, &gameOvr);

	// Send to all guide and field agents
	hashtable_t* agentHash = get_agent_hash(infostore);
	hashtable_iterate(agentHash, faSendIter, &gameOvr);
	hashtable_iterate(teamHash, gaSendIter, &gameOvr);

	// print game-over message on game server

	printf(" ---------------------------------------------------------------- \n");
	printf("| 				        GAME OVER 						         |\n");
	printf(" ---------------------------------------------------------------- \n");

	count_free(gameOverMsg);
}

/*********** teamRecordIter **************/
/** Helper function which iterates over the all the teams to get the information
* Needed to summarize the game contents 
*/

static void teamRecordIter(void* arg, char* key, void* data)
{
	struct gameOverInfo* gameOvr = arg;

	if (gameOvr != NULL && data != NULL){
		char* gameOverMsg = gameOvr->message;
		team_t* currTeam = data;

		char colon[] = ":";

		if (gameOvr->beginFlag == 1)
			strcat(gameOverMsg, colon);

		if (currTeam != NULL){

			//teamName
			char* teamName = key;

			// numPlayers ever active on this team
			int totalActiveAgents = get_total_num_agents(currTeam);

			//numCaptures is the number of opposing players captured by any player on this team
			int numCaptures = get_num_foe_captured(currTeam);

			//numCaptured is the number of this team’s players who were captured during the game
			int currActiveAgents = get_num_active_fa(currTeam) + get_num_active_ga(currTeam);
			int numCaptured = totalActiveAgents - currActiveAgents;

			//numNeutralized is the number of code drops this team neutralized during the game
			int totalNeut = get_num_code_neut(currTeam);

			// put the information into a string
			char* tempStr = assertp(count_calloc(MAXSTR, sizeof(char)), "Memory allocation for GAUpdate failed.\n");
			sprintf(tempStr, "%s,%d,%d,%d,%d", teamName, totalActiveAgents, numCaptures, numCaptured, totalNeut);
			strcat(gameOverMsg, tempStr);
			count_free(tempStr);

			gameOvr->beginFlag = 1;
		}
	}

}

/*********** faSendIter **************/
/** Helper function which iterates over the all the field agents in the agent hashtable
*/

static void faSendIter(void* arg, char* key, void* data)
{
	struct gameOverInfo* gameOvr = arg;
	agent_t* currAgent = data;

	if (gameOvr != NULL && data != NULL){
		if (currAgent != NULL){
			info_t* infostore = gameOvr->infostore;
			char* gameOverMsg = gameOvr->message;
			int commSock = get_commsock(infostore);
			struct sockaddr_in themp = get_agent_soc(currAgent);

			int datagram_check;
			if ((datagram_check = send_datagram(commSock, &themp, gameOverMsg)) != 0){
				fprintf(stderr, "Datagram failed to send\n");
			}


			if (logMode == 1){
				time_t timeStamp = get_curr_time(infostore);
				double elapsedTime = get_elapsed_time(infostore);

				raw_mode_logging_outbound(gameOverMsg, timeStamp, elapsedTime);
			}	
		}

	}
}

/*********** gaSendIter **************/
/** Helper function which iterates over the all the guide agents in the team hashtable
* Sends the game_over message.
*/

static void gaSendIter(void* arg, char* key, void* data)
{
	struct gameOverInfo* gameOvr = arg;

	if (gameOvr != NULL && data != NULL){
		team_t* currTeam = data;
		info_t* infostore = gameOvr->infostore;
		char* gameOverMsg = gameOvr->message;
		int commSock = get_commsock(infostore);

		if (currTeam != NULL){
			struct sockaddr_in themp = get_guide_soc(currTeam);

			int datagram_check;
			if ((datagram_check = send_datagram(commSock, &themp, gameOverMsg)) != 0){
				fprintf(stderr, "Datagram failed to send\n");
			}

			if (logMode == 1){
				time_t timeStamp = get_curr_time(infostore);
				double elapsedTime = get_elapsed_time(infostore);

				raw_mode_logging_outbound(gameOverMsg, timeStamp, elapsedTime);
			}	
		}
	}
}

/*********** trackGameStats **************/
/** Is triggered when the time of the game runs out or if all the codedrops have been neutralized
*  Iterates over the different hashtable - codehash/teamhash/agenthash to get the required information.
*/

struct gameStats* trackGameStats(info_t* infostore, time_t startTime)
{
	struct gameStats* currGameStats = assertp(count_calloc(1, sizeof(struct gameStats)), "Memory allocation for gameStats struct failed.\n");

	// Elapsed Time
	time_t currTime = get_curr_time(infostore);
	time_t elapsedTime = currTime - startTime;

	//find the number of active agents
	hashtable_t* teamHash = get_team_hash(infostore);
	int numActiveOps = 0;
	hashtable_iterate(teamHash, activeOpsIter, &numActiveOps);

	// find the number of active teams
	int numActiveTeams = 0;
	hashtable_iterate(teamHash, teamIter, &numActiveTeams);


	// find the number of codedrops neutralized by each team - list of team/num 
	hashtable_t* codeHash = get_code_hash(infostore);
	list_t* teamNeuts = list_new(counterListDel);
	hashtable_iterate(codeHash, codeNeutIter, teamNeuts);

	// find the number of codedrops yet to be neutralized
	int yetNeutCount = 0;
	hashtable_iterate(codeHash, yetNeutIter, &yetNeutCount);


	currGameStats->elapsedTime = elapsedTime;
	currGameStats->numActiveAgents = numActiveOps;
	currGameStats->numActiveTeams = numActiveTeams;
	currGameStats->teamNeutCodes = teamNeuts;
	currGameStats->numCodeDropNotNeut = yetNeutCount;

	//printf("Gets in here.  Track Stats5\n\n");


	return currGameStats;
}

/*********** teamIter **************/
/** Iterates over the teamhashtable to find the number of active teams remaining
*/

static void teamIter(void* arg, char* key, void* data)
{
	int* numActiveTeams = arg;
	if (numActiveTeams != NULL && data !=NULL){
		(*numActiveTeams)++;
	}
}

/*********** activeOpsIter **************/
/** Iterates over the agent hashtable to find the number of active agents remaining 
*/

static void activeOpsIter(void* arg, char* key, void* data)
{
	int* numActiveOps = arg;

	if (numActiveOps != NULL && data != NULL){

		team_t* currTeam = data;
		if (currTeam != NULL){
			int activeFA = get_num_active_fa(currTeam);
			int activeGA = get_num_active_ga(currTeam);
			*numActiveOps = *numActiveOps + activeGA + activeFA;
		}
	}
}

/*********** codeNeutIter **************/
/** Iterates over the code hashtable to produce a list of codes neutralized by each team
*/

static void codeNeutIter(void* arg, char* key, void* data)
{
	list_t* teamNeuts = arg;

	if (teamNeuts != NULL && data != NULL){
		codedrop_t* codedrop = data;

		if (codedrop != NULL){
			char* neutTeam = get_neuteam(codedrop);

			if (neutTeam != NULL){
				int* counter = list_find(teamNeuts, neutTeam);

				if (counter == NULL){
					int* newCounter = assertp(count_calloc(10, sizeof(int)), "Memory allocation for int failed.\n");
					*newCounter = 1; 
					list_insert(teamNeuts, neutTeam, newCounter);
				}
				else
				{
					(*counter)++; 
				}
			}
		}
	}
}


/*********** yetNeutIter **************/
/** Iterates over the code hashtable to produce the number of codes not yet neutralized
*/

static void yetNeutIter(void* arg, char* key, void* data)
{
	int* yetNeutCount = arg;

	if (yetNeutCount != NULL && data != NULL){
		codedrop_t* codedrop = data;

		if (codedrop != NULL){
			char* neutTeam = get_neuteam(codedrop);

			if (neutTeam == NULL)
				(*yetNeutCount)++;
		}
	}

}
/*********** counterListDel **************/
/** Frees the list of teams and code neutralized.
*/

static void counterListDel(void* data)
{
	if (data != NULL){
		count_free(data);
	}
}

/*********** game_stats_delete **************/
/** Frees the gameStats struct and its elements
*/

static void game_stats_delete(struct gameStats* stats)
{
	list_delete(stats->teamNeutCodes);
	count_free(stats);
}



/******************************************** Server Response Messages ******************************************************/

/*********** server_response **************/
/* Takes in the gameID, the response code, the message, the address of the target agent, the commSock of the current server and the infostore struct
* Puts it into the string and Sends the message to the target agent
*/

static void server_response(char* gameID, char* respCode, char* message, struct sockaddr_in themp, int comm_sock, info_t* infostore)
{
	char* serverResponse = assertp(count_calloc(MAXSTR, sizeof(char)), "Memory allocation for serverResponse failed.\n");
	sprintf(serverResponse, "%s|%s|%s|%s", gsResponse, gameID, respCode, message);
	int datagram_check;
	if ((datagram_check = send_datagram(comm_sock, &themp, serverResponse)) != 0){
		fprintf(stderr, "Datagram failed to send\n");
		char* datagramFailed = "ServerResponse Datagram failed to send\n";
		error_logging(datagramFailed);
	}

	if (logMode == 1){
		double elapsedTime = get_elapsed_time(infostore);
		time_t currTime = get_curr_time(infostore);
		raw_mode_logging_outbound(serverResponse, currTime, elapsedTime);
	}

	if (serverResponse != NULL)
		count_free(serverResponse);
}

/*********** invalid_opcode_response **************/
/** Helper function which sends an invalid OPCode response to the offending agent
* Takes in the infostore struct containing the required information for sending the response 
*/

static void invalid_opcode_response(info_t* infostore)
{
	if (infostore != NULL){
		char* currGameID = get_gameid(infostore);
		struct sockaddr_in themp = get_themp(infostore);
		int comm_sock = get_commsock(infostore);
		char* message = "You have sent an invalid opcode. Please try again.\n";
		server_response(currGameID, invalidOpCode, message ,themp, comm_sock, infostore);
	}
}

/*********** invalid_gameid **************/
/** Helper function which sends an invalid gameID response to the offending agent.
* Occurs when the gameID of the offending agent is invalid
* Takes in the infostore struct containing the required information for sending the response 
*/

static void invalid_gameid(info_t* infostore)
{
	if (infostore != NULL){
		char* currGameID = get_gameid(infostore);
		struct sockaddr_in themp = get_themp(infostore);
		int comm_sock = get_commsock(infostore);
		char* message = "You have sent an invalid gameID. Please double check your gameID.\n";
		server_response(currGameID, invalidGameID, message ,themp, comm_sock, infostore);
	}
}

/*********** invalid_genericID **************/
/** Helper function which sends an invalid genericID response to the offending agent.
* This occurs when either the pebbleID or the guideID or the PlayerName of the offending agent is invalid
* Takes in the infostore struct containing the required information for sending the response 
*/

static void invalid_genericID(info_t* infostore)
{
	if (infostore != NULL){
		char* currGameID = get_gameid(infostore);
		struct sockaddr_in themp = get_themp(infostore);
		int comm_sock = get_commsock(infostore);
		char* message = "You have sent an invalid guideID or PebbleID or PlayerName. Please double check that they are correct.\n";
		server_response(currGameID, invalidGenericID, message ,themp, comm_sock, infostore);
	}
}

/*********** invalid_teamname **************/
/** Helper function which sends an invalid teamname response to the offending agent.
* This occurs when the teamname of the offending agent is invalid.
* Takes in the infostore struct containing the required information for sending the response 
*/


static void invalid_teamname(info_t* infostore)
{
	if (infostore != NULL){
		char* currGameID = get_gameid(infostore);
		struct sockaddr_in themp = get_themp(infostore);
		int comm_sock = get_commsock(infostore);
		char* message = "You have sent an invalid teamname. Please double check that the TeamName is correct.\n";
		server_response(currGameID, invalidTeamName, message ,themp, comm_sock, infostore);
	}
}

/*********** code_neutralized **************/
/** Helper function which sends a code neutralized message.
* This occurs when an agent manages to neutralize a code successfully.
* Takes in the infostore struct containing the required information for sending the response 
*/


static void code_neutralized(info_t* infostore)
{
	if (infostore != NULL){
		char* currGameID = get_gameid(infostore);
		struct sockaddr_in themp = get_themp(infostore);
		int comm_sock = get_commsock(infostore);
		char* message = "You have successfully neutralized a codedrop!\n";
		server_response(currGameID, neutralizedOpCode, message ,themp, comm_sock, infostore);
	}
}

/*********** capture_success **************/
/** Helper function which sends a capture success message.
* This occurs when an agent manages to capture a foe agent successfully.
* Takes in the infostore struct containing the required information for sending the response.
* Also updates the number of foes captured by the capturing agent's team
*/

static void capture_success(agent_t* capturingAgent, info_t* infostore)
{
	if (infostore != NULL){
		char* currGameID = get_gameid(infostore);
		struct sockaddr_in themp = get_agent_soc(capturingAgent);
		int comm_sock = get_commsock(infostore);
		char* message = "You have captured an enemy agent! Tango Down!\n";

		//Update the number of foes which the team of the Capturing Player has captured
		hashtable_t* teamHash = get_team_hash(infostore);
		char* teamName = get_team_name_agent(capturingAgent);
		team_t* currTeam = hashtable_find(teamHash, teamName);
		update_num_foe_captured(currTeam);

		// Send the server response to the capturing agent
		server_response(currGameID, captureSuccessCode, message , themp, comm_sock, infostore);
	}
}

/*********** captured **************/
/** Helper function which sends a captured message to the captured agent.
* This occurs when an agent has been captured successfully.
* Takes in the infostore struct containing the required information for sending the response.
* Also, updates the captured agent status in his team
*/

static void captured(agent_t* capturedAgent, info_t* infostore)
{
	if (infostore != NULL){
		char* currGameID = get_gameid(infostore);
		struct sockaddr_in themp = get_agent_soc(capturedAgent);
		int comm_sock = get_commsock(infostore);
		char* message = "You have been captured! Sorry better luck next time \n";
		server_response(currGameID, captureCode, message, themp, comm_sock, infostore);

		hashtable_t* teamHash = get_team_hash(infostore);
		char* capturedAgentTeamName = get_team_name_agent(capturedAgent);
		team_t* capturedAgentTeam = hashtable_find(teamHash, capturedAgentTeamName);

		minus_num_FA_Ops_Active(capturedAgentTeam); // decrement the number of active FA in the team
		
	}
}
