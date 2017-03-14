# Project Incomputable Implementation Spec - Jade
Authors: Max Zhuang, Raunak Bhojwani, Samuel Ching

## Lib Module

### Functional Prototypes & Parameters

**Agent Module**

	* agent_new()
	
	`agent_t* agent_new(int gameID, int pebbleID, char* teamName, char* playerName,
    float agentLat, float agentLong, char* playerStatus, struct sockaddr_in* agentSoc);`
    
    * agent_del()
	
	void agent_del(agent_t* agent);

	* update_last_cont()

	void update_last_cont(agent_t* agent, int seconds);

	* update_loc()

	void update_loc(agent_t* agent, float agentLat, float agentLong);

	* update_player_status

	void update_player_status(agent_t* agent, char* status);

	* Update_player_soc

	void update_player_soc(agent_t* agent, sockaddr_in* agentSoc);

**Team Module**
	
	team_new()
	
	team_t* team_new(int guideID, int gameID, char* teamName, char* guideStatus, struct sockaddr_in* SocInfo);
	
	team_del()
	
	void team_del(team_t* team);
	
	insert_agent()
	
	void insert_agent(team_t* team, agent_t* agent);
	
	find_agent()
	
	agent_t* find_agent(team_t* team, int pebbleID);
	
	update_guide_status()
	
	void update_guide_status(team_t* team, char* guideStatus);
	
	update_GASoc()
	
	void update_GASoc(team_t* team, struct sockaddr_in* socInfo);

**Message Module**
	
	socket_setup()
	
	int socket_setup(const char* gameServerPort);
	
	handle_socket()
	
	char* handle_socket(int comm_sock, struct sockaddr_in *themp);
	
	send_datagram()
	
	int send_datagram (int comm_sock, struct sockaddr_in *themp, char* message);



## Field Agent Module
    
The Field Agent is designed to run on the Pebble and play the game indicated by the requirement specification.
    
**Function decomposition** 
Not including helper functions:

	captureOpponent()
	neutralizeDrop()
	sendMessage()
	receiveMessage()    
	    
Pseudocode for the field agent is as follows:

* Set gameID to 0
* Set pebbleID to relevant ID
* Choose team and set team name
* Set Player name from name chosen
* Use location details to set lat and long
* Set codeID and captureID to NULL
* Send GS lat and long 4 times every minute via protocol
* Receive hints from GS, if any
* If the option to neutralize is selected:
	* Send message to GS using protocol after taking as input 4-digit hex code and storing in codeID
	* Receive message from GS regarding the code drop
	* If successful, indicate so and carry on
	* If unsuccessful, indicate so and carry on
* If the option to capture is selected:
	* Send message to GS using protocol
	* Receive message from GS regarding nearby opposition, showing 4-digit hex codes that are relevant
	* Save chosen hex code to captureID
	* Send message to GS using protocol to capture
	* Receive message of capture
	* If successful, indicate so and carry on
	* If unsuccessful, indicate so and carry on

## Guide Agent Module

Function decomposition:

*not including helper functions*

* Functions
	
	`argCheck(int argc, char*argv[]);`
   `connectGs(char* Gshost, int Gsport);`
   `makeMap(hashtable_t *cdTable, hashtable_t *faTable);`
   `receiveCode(char * OPCODE);`
   `sendCode(char *OPCODE);`


Psuedo code:

* Check commandline arguments
    *  greater than 4 arguments
    *  two optional arguments to handle - log, guideID
        * If guideID is called, restart guide agent with the guide ID, else provide guideID
        * If log mode is entered set log mode, else set log mode to raw - log everything
 
* Connects with Game Server gshost and port number - joins game
 * opens up UDP socket
 * send datagram to the Game Server at its preconfigured host/port address
* Initialize Data Structures
    * cd hashtable
    * fa hashtable
* Creates graphical map of game status to terminal
    * use gtk package
    * see current status of each player on user’s team
    * choose player to send a hint to
    * sends status request to gs
* Continuous while loop for incoming datagrams - joins game
    * allows for receiving of datagram
        * parses datagram, extracts opcode
        * if opcode matches, determine function to call
        * logs messages if log mode is raw
    * every 5 minutes sends status request to gs
* Location/status updates
    * update hashtables accordingly - cd or fa
* call create graphical map to draw updated map onto terminal
* update logfile
* status request
    * sends to gs
* hint request
    * computes closest cd to selected fa
    * formats 
* send request

## Game Server

Functional Prototypes & Parameters
	
	update_fa_location()
	neutralize_code()
	capture_player()
	update_GA()
	send_hint()
	
Pseudocode

* Check commandline arguments
* Check optional arguments - including logging mode and game level
* Start the game time
* Initialize the game ID
* Initialize the data structures
* Read the list of code drops in from the file
* Continuous while-loop waiting for incoming datagrams
	* Upon successful receipt of a datagram, the message is parsed and the OPCODE is extracted
    * Initialize a temporary sockaddr_in variable to hold the address of the agent.
	* If the OPCODE matches the list of OPCODEs, then, use the function table (different levels, different function tables) to determine which function to call.
Within each OPCODE function, validate the number of parameters - based on the number of elements in the array.
    * It will then validate the individual parameters, and update the appropriate records
    * It will also assign the sockaddr_in struct to the agent who sent the message
    * It will then execute the relevant actions

    * Else, send an error message back to the offending agent.

    * Print a textual summary of game status to terminal

## Data Structures
 
3 Types of Structs:
	
	Struct Team: 
		guideID:
		guideStatus:
		guideLastSeen:
		gameID: 
		agentList: List of agent structs

	Struct Agent:
		gameID
		pebbleID
		teamName
		playerName
		agentLat
		agentLong
		playerStatus
		sockaddr_in struct (Storing IP Address & Port number of each agent)
	
	Struct codeDrop:
		codeLat
		codeLong
		neutralizingTeam
		codeID
		codeStatus
    
Then, having these structs, we’ll have two hashtables - a hashtable of teams and a hashtable of codeDrops. 

    Hashtable (Teams):
		Key: TeamName
		Data: Team Struct

    Hashtable (Codes):
		Key: CodeID
		Data: codeDrop Struct