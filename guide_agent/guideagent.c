/*guideagent.c A program which runs on the CS50 Unix Server, accept and validate four command-line arguments, communicates with game server to update the game, present a suitable interface, and logs all its activity to a logfile

  Project name: Project Incomputable (Jade)
  Component name: Guide Agent

  Primary Authors: Max Zhuang, Raunak Bhojwani, Samuel Ching
  Date Created: 5/22/16

  Special considerations:  
  (e.g., special compilation options, platform limitations, etc.) 
  
  ======================================================================*/

//
// ---------------- Open Issues
// ---------------- System includes e.g., <stdio.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <math.h>
#include <time.h>

// ---------------- Local includes  e.g., "file.h"

#include "../lib/memory.h"
#include "../lib/hashtable.h"
#include "../lib/agent.h"
#include "../lib/team.h"
#include "../lib/codedrop.h"
#include "../lib/latlongdist.h"
#include "../lib/randomHexGen.h"

// ---------------- Definitions
static const int BUFSIZE = 1024;
const int HASHSLOTS = 1000;
char DELIM[] = "|";
int MAXSTR = 20000;
int HEXI = 8;

char* handle_socket(int comm_sock, struct sockaddr_in *themp);
int tokenize_input(char *message, char ** msgArray, char delim[]);
int update_ga(char** messageArray, char* gameID, hashtable_t *agentHash, hashtable_t *codeHash );
int del_array(char ** Array);
int send_datagram(int comm_sock, struct sockaddr_in *themp, char *message);

static int socket_setup(char *GShost, int GSport, struct sockaddr_in *themp);
static char* handle_stdin(int comm_sock, struct sockaddr_in *themp);
static void update_agent(char *agentData, char ** agentArray, hashtable_t * agentHash,  char*  gameID);
static void update_codedrop(char *codeData, char ** codeArray, hashtable_t * codeHash);

// ---------------- Structures/Types



/*====================================================================*/
int
main (const int argc, char* argv[])
{
  
  // set defaults for variables
  int logMode = 0;  // 0 = default game logmode, 1 = raw logmode
  int statusReq = 0;
  int valLoop = 0;  // validates while loop

  char *guideID = gen_random_hexcode(8);
  char *gameID = assertp(count_calloc(HEXI, sizeof(char)), "error: invalid gameID");
  strcpy(guideID,"0");  // default game id before joining is 0
  char *gaStatus = assertp(count_calloc(MAXSTR, sizeof(char)),"error: invalid gaStatus");
  strcpy(gaStatus,"GA_STATUS");
  char *gaHint = assertp(count_calloc(MAXSTR, sizeof(char)),"error: invalid gaStatus");
  strcpy(gaHint,"GA_HINT");
  
  // set placeholders for different commandline arguments
  int teamNameArg = 1;
  int playerNameArg = 2;
  int GShostArg = 3;
  int GSportArg = 4;

  // defaults are set, but must be adjusted when optional command line arguments are entered
  if (argc < 5){
    fprintf(stderr, "usage: %s [-v|-log=raw] [-id=#######] teamName playerName GShost GSport", argv[0]);
    exit(1);
  }

  // operates under assumption that commandline arguments are entered in order as ordered and correctly
  if (argc > 5){
    
    // only supports two log mode options for raw/verbose
    if ((strcmp(argv[1], "-log=raw")==0)||(strcmp(argv[1],"-v")==0)){
      logMode = 1;
      if (argc == 6){
	teamNameArg = 2;
	playerNameArg = 3;
	GShostArg = 4;
	GSportArg = 5;
      }
      else if (argc == 7){
	teamNameArg = 3;
	playerNameArg = 4;
	GShostArg = 5;
	GSportArg = 6;
	guideID = argv[2];  // guideID is random unless told otherwise
      }
      else {
	fprintf(stderr, "usage: %s [-v|-log=raw] [-id=#######] teamName playerName GShost GSport", argv[0]);
	exit(2);
      }
    }
    
    // if neither log options are entered, assume no log option
    else {
      if (argc >6){
	fprintf(stderr, "usage: %s [-v|-log=raw] [-id=#######] teamName playerName GShost GSport", argv[0]);
	exit(3);
      }
      else if (argc == 6){
	guideID = argv[1];
	teamNameArg = 2;
	playerNameArg =3;
	GShostArg = 4;
	GSportArg = 5;
      }
      else {
	fprintf(stderr, "usage: %s [-v|-log=raw] [-id=#######] teamName playerName GShost GSport", argv[0]);
	exit(4);
      }
    }
  }
      
  // initialize the data structures
  hashtable_t* agentHash = hashtable_new(HASHSLOTS, agent_delete);
  hashtable_t* codeHash = hashtable_new(HASHSLOTS, code_delete);

  // initialize variables determined by commandline arguments
  char * teamName = assertp(count_calloc(strlen(argv[teamNameArg]) + 1,sizeof(char)),"error:invalid team name");
  strcpy(teamName, argv[teamNameArg]);
  char * playerName = assertp(count_calloc(strlen(argv[playerNameArg]) + 1, sizeof(char)), "error: invaled player name");
  strcpy(playerName, argv[playerNameArg]);
  char * GShost = assertp(count_calloc(strlen(argv[GShostArg]) + 1, sizeof(char)),"error: invalid GShost");
  strcpy(GShost,argv[GShostArg]);
  int GSport = atoi(argv[GSportArg]);
  
  // set up socket
  struct sockaddr_in them;
  int comm_sock = socket_setup(GShost, GSport,  &them);

  // initial message to game server
  char * initStatusMessage = assertp(count_calloc(MAXSTR, sizeof(char)), "Memory allocation for status message failed.\n");
  sprintf(initStatusMessage, "%s|%s|%s|%s|%s|%d", gaStatus,gameID,guideID,teamName,playerName,statusReq);
  int datagramCheck;
  if ((datagramCheck = send_datagram(comm_sock, &them, initStatusMessage)) != 0){
    fprintf(stderr, "initial status Datagram failed to send\n");
    exit(5);
  }

  // while loop handles responses to Guide Agent
  while (true) {

    // for use with select()
    fd_set rfds;
    struct timeval timeout;
    const struct timeval fivesec = {5,0};

    // what stdin (fd 0) and the UDP socket to see when either has an input
    FD_ZERO(&rfds);
    FD_SET(0, &rfds);  //stdin
    FD_SET(comm_sock, &rfds);  //socket
    int nfds = comm_sock+1;
    timeout = fivesec;
    int select_response = select(nfds, &rfds, NULL, NULL, &timeout);

    if (select_response == 0){
      printf("to send a hint to FA: the pebbleID or * for all agents|your hint (free text) of 1..140 printable characters, to ask for a status update: enter the word status");
      fflush(stdout);

      // sends update to game server
      statusReq = 0;
      char * statusMessage = assertp(count_calloc(MAXSTR, sizeof(char)), "Memory allocation for status message failed");
      sprintf(statusMessage, "%s|%s|%s|%s|%s|%d", gaStatus,gameID,guideID,teamName,playerName,statusReq);
      int datagramCheck;
      if ((datagramCheck = send_datagram(comm_sock, &them, statusMessage)) != 0){
	fprintf(stderr, " status Datagram failed to send\n");
	exit(6);
      }
      
    }

    // something is typed in stdin or recieved a message
    else if (select_response > 0){
      if (FD_ISSET(0, &rfds)){
	char *response = handle_stdin(comm_sock, &them);
	int res = strlen(response);

	// something is typed in stdin
	if (res != 0){
	  char** resArray = assertp(count_calloc(res + 1, sizeof(char*)), "Memory allocation for response message array failed.\n");
	  int resArrayLen = tokenize_input(response, resArray, DELIM);

	  // one thing typed in stdin
	  if (resArrayLen == 1){
	    if (strcmp(resArray[0],"status") == 0){    
	      // sends update to game server, request game server send an update
	      statusReq = 1;
	      char * statusMessage = assertp(count_calloc(MAXSTR, sizeof(char)), "Memory allocation for status message failed");
	      sprintf(statusMessage, "%s|%s|%s|%s|%s|%d", gaStatus,gameID,guideID,teamName,playerName,statusReq);
	      int datagramCheck;
	      if ((datagramCheck = send_datagram(comm_sock, &them, statusMessage)) != 0){
		fprintf(stderr, " status Datagram failed to send\n");
		exit(7);
	      }
	    }
	    // one thing typed but not in correct format to request status update
	    else {
	      printf("please type a correct input to stdin");
	      fflush(stdout);
	    }
	  }
	  
	  // two things typed in standard in, if correct format, send hint
	  else if (resArrayLen == 2){
	    //send hint to all agents or single agent if valid
	    agent_t * hintTarg;
	    hintTarg = hashtable_find(agentHash, resArray[0]);

	    //valid
	    if ((strcmp(resArray[0],"*")==0)||(hintTarg != NULL)){
	      if ((hintTarg != NULL) && (strcmp(get_team_name_agent(hintTarg),teamName) != 0)){
		  printf("You can only hint at agents on your own team");
		  fflush(stdout);
		  break;
		}
	       char * hint = assertp(count_calloc(MAXSTR, sizeof(char)), "Memory allocation for hint message failed");
	       sprintf(hint,"%s|%s|%s|%s|%s|%s|%s",gaHint,gameID,guideID,teamName,playerName,resArray[0],resArray[1]);
	       int datagramCheck;
	       if ((datagramCheck = send_datagram(comm_sock, &them, hint)) != 0){
		 fprintf(stderr, " status Datagram failed to send\n");
		 exit(8);
	       }
	    }
	    else {
	      printf("please type a correct input to stdin");
	      fflush(stdout);
	    }
	  }
	    
	  del_array(resArray);
	}
      }

      // check to receive message
      if (FD_ISSET(comm_sock, &rfds)){
	
	char* message = handle_socket(comm_sock, &them);
	int maxwords = strlen(message);
    
	if (maxwords != 0){
	  char** msgArray = assertp(count_calloc(maxwords + 1, sizeof(char*)), "Memory allocation for message array failed.\n");
      
	  // use tokenize_input to convert message to array of strings to handle
	  int arraylen = tokenize_input(message, msgArray, DELIM);
	  if ((arraylen != 0)&&(arraylen == 4)){
	    valLoop = update_ga(msgArray, gameID, agentHash, codeHash);
	  }
	  del_array(msgArray);
	}
	count_free(message);
      }
    }
    
    if (valLoop != 0){
	break;
      }
    // print graphical summary of game
  }
    
  close(comm_sock); 
  hash_delete(agentHash);
  hash_delete(codeHash);
  
  count_free(teamName);
  count_free(gameID);
  count_free(guideID);
  count_free(GShost);
  count_free(playerName);
  
  count_free(gaHint);
  count_free(gaStatus);
  return 0;
}

/**************** socket_setup ****************/
static int
socket_setup(char *GShost, int GSport, struct sockaddr_in *themp){
  struct hostent *hostp = gethostbyname(GShost);
  if (hostp == NULL) {
    fprintf(stderr, "error unknown host");
    exit(1);
  }
  // initialize fields of the server address
  themp->sin_family = AF_INET;
  bcopy(hostp->h_addr_list[0], &themp->sin_addr, hostp->h_length);
  themp->sin_port = htons(GSport);

  //create socket
  int comm_sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (comm_sock < 0){
    fprintf(stderr, "error opening datagram socket");
    exit(2);
  }

  return comm_sock;
}

/**************** handle_socket ****************/
char *
handle_socket(int comm_sock, struct sockaddr_in *themp){
  struct sockaddr_in sender;
  struct sockaddr *senderp = (struct sockaddr *) &sender;
  socklen_t senderlen = sizeof(sender);
  char buf[BUFSIZE];
  int nbytes = recvfrom(comm_sock, buf, BUFSIZE-1, 0, senderp, &senderlen);

  if (nbytes < 0){
    fprintf(stderr,"receiving from socket");
    exit(1);
  } else {
    buf[nbytes] = '\0';  // null terminate string
    if (sender.sin_family != AF_INET)
      printf("From non-Internet address: Family %d\n", sender.sin_family);
    else {
      *themp = sender;
      char * message = assertp(count_calloc(BUFSIZE, sizeof(char)), "Memory allocation for message failed.\n"); // message is freed in while loop
      strcpy(message,buf);
      printf("[%s@%05d]: %s\n", inet_ntoa(sender.sin_addr), ntohs(sender.sin_port), message);
      return message; 
    }
  }
  return NULL;
}

/**************** handle_stdin ****************/
static char*
handle_stdin(int comm_sock, struct sockaddr_in *themp)
{
  int resLim = 149;  // including hex code, hint can not be longer than 140 characters
  if (themp->sin_family != AF_INET){
    printf("confused: server is not AF_INET. \n");
    fflush(stdout);
  }
  else {
    char * responseMessage = assertp(count_calloc(resLim, sizeof(char)), "Memory allocation for message failed.\n");
    if (fgets(responseMessage,resLim,stdin) != NULL){
      return responseMessage;
    }
  }
  return NULL;
}


/****************** send_datagram ***************/
int
send_datagram(int comm_sock, struct sockaddr_in *themp, char *message){
  if (sendto(comm_sock, message, strlen(message), 0,
	     (struct sockaddr *) themp, sizeof(*themp)) < 0){
    perror("sending in datagram socket");
    return 1;
  }
  count_free(message);

  return 0;
}

/****************** tokenize_input ***************/
int tokenize_input(char* message, char** msgArray, char delim[]){
  if (message == NULL){
    fprintf(stderr, "Message is NULL\n");
    return 0;
  }

  if (msgArray == NULL){
    fprintf(stderr, "Message is NULL\n");
    return 0;
  }

  int wordcount = 0;
  char* word = NULL;

  for (word = strtok(message, delim); word != NULL; word = strtok(NULL, delim)){
    char* newWord = assertp(count_calloc(MAXSTR, sizeof(char)), "Memory allocation for word failed.\n"); // newWord placed in msgArray is later deleted within the del_array functions
    strcpy(newWord, word);
    msgArray[wordcount++] = newWord;
  }
  return wordcount;
}

/****************** del_array ***************/
int del_array(char ** Array){
  int i = 0;
  while (Array[i] != NULL){
    count_free(Array[i++]);
  }
  count_free(Array);
  return 0;
}

/****************** update_ga ***************/
int update_ga(char** messageArray, char* gameID, hashtable_t *agentHash, hashtable_t *codeHash ){
  if (messageArray == NULL){
    fprintf(stderr, "message array doesn't exist");
    return 0;
  }
  char * opcode = messageArray[0];
  if (strcmp(opcode, "GAME_STATUS") == 0){
    char * currID = messageArray[1];
    if (strcmp(gameID, currID) != 0){
      gameID = currID;
    }

    // update agents
    int agentDataLen = strlen(messageArray[2]);
    printf("\n\n \n\n \n %s", messageArray[2]);
    if (agentDataLen != 0) {
      char **agentArray = assertp(count_calloc(agentDataLen + 1, sizeof(char*)), "Memory allocation for agent array failed.\n");
      update_agent(messageArray[2], agentArray, agentHash, gameID);
      del_array(agentArray);
    }

    // update code drops
    int codeDataLen = strlen(messageArray[3]);
    if (codeDataLen != 0){
      char **codeArray = assertp(count_calloc(codeDataLen + 1, sizeof(char*)), "Memory allocation for cd array failed.\n");
      update_codedrop(messageArray[3], codeArray, codeHash);
      del_array(codeArray);
    }
    return 0;
  }
  
  else if(strcmp(opcode,"GAME_OVER")==0){
    //print game summary
    return 1;
  }

  return 0;
}

/****************** update_agent ***************/
static void update_agent(char *agentData, char ** agentArray, hashtable_t * agentHash, char* gameID){
  char agentDelim[] = ":";
  char subFieldDelim[] = ",";
  char *agent = NULL;
  int agentNum = 0;

  // loop through agents
  for (agent = strtok(agentData,agentDelim);agent != NULL; agent = strtok(NULL,agentDelim)){
    char *newAgent = assertp(count_calloc(MAXSTR, sizeof(char)),"memory allocation for agent failed.\n");
    strcpy(newAgent,agent);
    agentArray[agentNum++] = newAgent;

    // create array of subfields
    agent_t * currAgent;
    int subFieldLen;
    int subFieldNum;
    subFieldLen = strlen(newAgent);
    char **agentSubField = assertp(count_calloc(subFieldLen + 1, sizeof(char*)), "Memory allocation for agent SubField failed.\n");
    subFieldNum = tokenize_input(newAgent, agentSubField, subFieldDelim);
    if ((subFieldNum != 0)&&(subFieldNum==6)){

      // make the subfields, these frees are accounted for when hashtable is deleted
      char *pebbleID  = assertp(count_calloc(strlen(agentSubField[0]) + 1, sizeof(char)),"memory allocation for agent pebbleID failed.\n");
      strcpy(pebbleID,agentSubField[0]);
      char *teamName = assertp(count_calloc(strlen(agentSubField[1]) + 1, sizeof(char)),"memory allocation for agent team Name failed.\n");
      strcpy(teamName, agentSubField[1]);
      char * playerName = assertp(count_calloc(strlen(agentSubField[2]) + 1, sizeof(char)),"memory allocation for agent player Name failed.\n");
      strcpy(playerName,agentSubField[2]);
      char * playerStatus = assertp(count_calloc(strlen(agentSubField[3]) + 1, sizeof(char)),"memory allocation for agent player status failed.\n");
      strcpy(playerStatus,agentSubField[3]);
      
      float lastKnownLat = atof(agentSubField[4]);
      float lastKnownLong = atof(agentSubField[5]);
      
      struct tm tm;
      char * stringTime = assertp(count_calloc(strlen(agentSubField[6]) + 1, sizeof(char)),"memory allocation for agent time failed.\n");
      strcpy(stringTime, agentSubField[6]);
      tm.tm_sec = atof(stringTime);
      time_t sinceLastContact = mktime(&tm);
      count_free(stringTime);

      // check to see if agent exists
      currAgent = hashtable_find(agentHash, pebbleID);
      if (currAgent == NULL){

	// create dummy socket
	struct sockaddr_in dummy;
	currAgent = agent_new(gameID, pebbleID, teamName, playerName, lastKnownLat, lastKnownLong, playerStatus, dummy);  // no socket information available, so put in dummy
	update_last_cont(currAgent, sinceLastContact);
	hashtable_insert(agentHash, pebbleID, currAgent);
	printf("%s",get_player_status(currAgent));
      }
      else {
	update_last_cont(currAgent, sinceLastContact);
	update_loc(currAgent, lastKnownLat, lastKnownLong);
	update_player_status(currAgent, playerStatus);
      }
    }
    count_free(newAgent);
    del_array(agentSubField);
  }
}

/****************** update_codedrop ***************/
static void update_codedrop(char *codeData, char ** codeArray, hashtable_t * codeHash){
  char codeDelim[] = ":";
  char subFieldDelim[] = ",";
  char *code = NULL;
  int codeNum = 0;

  // loop through all code drops at ":" deliminater
  for (code = strtok(codeData,codeDelim);code != NULL; code = strtok(NULL,codeDelim)){
    char *newCode = assertp(count_calloc(MAXSTR, sizeof(char)),"memory allocation for code failed.\n");
    strcpy(newCode, code);
    codeArray[codeNum++] = newCode;

    // create array of subfields
    codedrop_t * currCode;
    char codeActive[] = "Active";
    char codeNeutralized[] = "Neutralized";
    int subFieldLen;
    int subFieldNum;
    subFieldLen = strlen(newCode);
    char **codeSubField = assertp(count_calloc(subFieldLen + 1, sizeof(char*)), "Memory allocation for code SubField failed.\n");
    subFieldNum = tokenize_input(newCode, codeSubField, subFieldDelim);
    if ((subFieldNum != 0)&&(subFieldNum ==2)){
      currCode = hashtable_find(codeHash, codeSubField[0]);
      if (currCode == NULL){
	if (strcmp(codeSubField[3],"NONE") == 0){
	  currCode = codedrop_new(atof(codeSubField[1]), atof(codeSubField[2]), codeSubField[0], codeActive);
	}
	else {
	  currCode = codedrop_new(atof(codeSubField[1]), atof(codeSubField[2]), codeSubField[0], codeNeutralized);
	}
	hashtable_insert(codeHash, codeSubField[0], currCode);
	printf("INSERT %s\n",get_code_status(currCode));
      }	
      else {
	if (strcmp(codeSubField[3], "NONE") == 0){
	  update_code_status(currCode, codeSubField[3], codeActive);
	}
	else {
	  update_code_status(currCode, codeSubField[3], codeNeutralized);
	}
	printf("EXISTING %s\n", get_code_status(currCode));
      }
    }
    count_free(newCode);
    del_array(codeSubField);
  }
}


