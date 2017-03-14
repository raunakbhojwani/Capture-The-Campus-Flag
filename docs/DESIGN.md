# Project Incomputable Design Spec - Jade

* Adapted from Prof Kotz’s Lecture 14 Design Specs for Tiny Search Engine - Crawler
* Based on requirements in Prof Kotz’s Project Incomputable

## Diagram

![Design Spec Diagram](PI.png)

## Field Agent

	**The querier shall:**
		1. execute from a command line with usage syntax ./querier pageDirectory indexFilename
					where pageDirectory is a directory produced by the crawler, and
					where indexFilename is an index file produced by the indexer.
		2. load the index from indexFilename into an internal data structure.
		3. read search queries from stdin, one per line, until EOF.
		4. clean and parse each query according to the syntax described below.
		5. print the ‘clean’ query for user to see.
		6. use the index to identify the set of documents that satisfy the query, as described below.
					if no documents satisfy the query, print No documents match.
					otherwise, rank the resulting set of documents according to its score, as described below, and print the set of documents in decreasing rank order; for each, list the score, document ID and URL. (Obtain the URL by reading the first line of the relevant document file from the pageDirectory.)
		
	**The querier shall validate its command-line arguments:**
		1. pageDirectory is a directory produced by the crawler, and
		2. indexFilename is the name of a readable file.

	**The querier may assume:**
		1. that the input directory and files follow the designated formats.
		2. pageDirectory has files named 1, 2, 3, …, without gaps.
		3. The content of document files in pageDirectory follow the format as defined in the crawler specs; thus your code (to read a document file) need not have extensive error checking.
		4. The content of the file named by indexFilename follows our Index file format; thus your code (to recreate an index structure by reading a file) need not have extensive error checking.
	
	**Example Output from Querier**

		Query: dartmouth or computer science or programming or unix or doug mcilroy 
		Matches 7 documents (ranked):
		score 292 doc   7: http://old-www.cs.dartmouth.edu/~dfk/papers/index.html
		score   9 doc   4: http://old-www.cs.dartmouth.edu/~dfk/postdoc.html
		score   7 doc   6: http://old-www.cs.dartmouth.edu/~dfk/people.html
		score   6 doc   1: http://old-www.cs.dartmouth.edu/~dfk/index.html
		score   5 doc  10: http://old-www.cs.dartmouth.edu/~dfk/ChampionInternationalProfessor.html
		score   4 doc   9: http://old-www.cs.dartmouth.edu/
		score   4 doc   5: http://old-www.cs.dartmouth.edu/~dfk/teaching.html
		--------------------------------------------------------------------------------------------

		Query: and 
		Error: 'and' cannot be first

		Query: or 
		Error: 'or' cannot be first

		Query: and dartmouth 
		Error: 'and' cannot be first

		Query: or dartmouth 
		Error: 'or' cannot be first

		Query: dartmouth college or 
		Error: 'or' cannot be last

		Query: dartmouth college and 
		Error: 'and' cannot be last

		Query: dartmouth college and or computer 
		Error: 'and' and 'or' cannot be adjacent

		Query: dartmouth college and and computer 
		Error: 'and' and 'and' cannot be adjacent

		Query: dartmouth college or and computer 
		Error: 'or' and 'and' cannot be adjacent
		Error: bad character '5' in query.
		Error: bad character '!' in query.
		Error: bad character '-' in query.

**Design Specification:**

	The querier makes use of an inverted index data structure. An inverted index data structure is simply a hashtable, implemented using lists. Each list has a key and data, and in this structure the key is the word, and its data is a Counter data structure. Each word has its own counter. Counters are also implemented like linked lists and contain a key and data. In this case the key will be the docID and data is the count, as in the number of times that word has appeared.

	In summary, the querier allows the user to search for words in a directory of pages.

**Purpose**

A Pebble app that communicates with the game server to transfer information to the server and guide agents.


**User interface**

The pebble app will allow players to choose from a list of names, and then enter them into the game, where they will have the ability to neutralize code drops or capture opposing team members

**Inputs and outputs**

* End the Game Server the player’s current location four times per minute.
* Inform the Game Server when this player neutralizes a code drop (by providing a 4-digit hex code).
* Inform the Game Server when this player has captured another player (by providing a 4-digit hex code).
* Receive from Game Server hints from the Guide Agent, to then be presented on the app
* Receive from Game Server 4-digit hex code indicating this Field Agent is target of capture.
* Receive from Game Server indication that this Field Agent has been successfully captured.


**Functional decomposition into modules**

* captureOpponent: functionality to capture an opponent
* neutralizeDrop: functionality to neutralize a code drop using a 4-digit Hex Code
* communicateServer: functionality to relay information about location

**Dataflow through modules**

Respond to the user interface of the app to appropriately allow for captures or neutralizations, communicating with each module with checks in the main function of the program, the location of the player is the data that will be transferred into each module.

**Major data structures**

No data structures are needed for the field agent

**Extensions**

* Stationary or moving (accelerometer API)
* Step count and calories burned
* Radar capability
* Enhance UI if time allows

##Guide Agent
point person: Max

**Purpose**

A Unix program that communicates with the game server and interacts with the player who acts as a guide
User interface

	./guideagent [-log=raw] teamName GShost GSport

Log is written in guideagent.log to record the flow of a component’s activity, particularly for testing and debugging - the UI will allow for raw and game modes for logging information

Team name
Game Server Host
Game Server Port Number 

**Inputs and outputs**

* Receive new members, location and status updates of team members and others, receive indications of status and location of code drops, and receive requests for hints for a specific player - GAME_STATUS
* Sends status updates - GA_STATUS 
* Send the hints in 1..140 printable characters (as defined by isprint()), excluding the pipe | symbol - GA_HINT
* Updates into log file in log directory


**Functional decomposition into modules**

create graphical game summary using gtk package

*See Game Server* 

**Dataflow through modules**

*See Game Server* 

**Major data structures**

*See Game Server*

##Game Server
point person: Sam

**Purpose**

A program which runs on the CS50 Unix Server, communicating between the Field Agents and the Guide Agents as well as storing information about the Game State.

**User interface**

An example of the command-line interface for the Game Server is as follows:

    ./gameserver [-log=raw] codeDropPath GSport logfileName

where codeDropPath Is a file pathname for the file containing specifications about the code drops, GSport is the port number for the Game Server and logfileName is the filepath to the logfile output.    

**Inputs and outputs**

* Input:
	* Initial input: Command Line Parameter (see User Interface) which has to be validated

   * Subsequent input: Messages from Guide Agents and Field Agents to Game Server.
    These messages will take the format of a string of at most 16383 characters, beginning with the OPCODE. For instance, a string would begin with OPCODE | … | …, with … representing other fields. 
    
    * OPCODEs:
		*	`FA_LOCATION` - is a formatted message that informs the Game Server (GS) of its location and identifiers that allow the GS and Guide Agent (GA) to uniquely identify a particular Field Agent (FA).
		
		* `FA_NEUTRALIZE` - is a formatted message that informs the GS of its current location and the unique identifier that represents the page of source code found at that location.
FA_CAPTURE - is a formatted message that informs the GS which opposing FA is in the vicinity, which is then relayed to the capturing FA. If the capture is a success, then communicate the result of the capture to both the capturing FA and the captured FA. 

		* `GA_STATUS` is a formatted message in which the GA gets a game status update or joins the game if it is not currently in any game.
		* `GA_HINT` is a formatted message which allows the GA to send hints to its FAs.

		* `GAME_STATUS` is a formatted message which allows the game server to send updates to the FAs or GAs upon request.

		* `GS_CAPTURE_ID` is a formatted message which the GS sends to a FA that the FA is about to be captured. 

		* `GAME_OVER` is a formatted message which the GS sends to all FAs and GAs - with the key statistics of the game as well as the fact that the game is over

    * Output: 

	1. It will communicate the following with field agents via protocol:
		* Forward hints from the Guide Agent
		* Send a Notice of ‘Maybe-Captured’ with the code
		* Send a Notice of ‘captured’
		* Send game status updates, on request, including:
			* Status about the team’s guide: two modes - pending or active/ as well as time since the last contact
			* Status of all Field Agents on that particular team
			* Game Statistics: Elapsed Time, number of active agents, number of active teams, number of code drops yet to be neutralized

	2. It will communicate with Guide Agents via protocol:
		* Send game status updates, on request, to include:
		* Game Statistics: Elapsed Time, number of active agents, number of active teams, number of code drops yet to be neutralized
		* Location and Status of all Code Drops
		* Location, name, team and status of all Field Agents
		
	3. Present a textual summary of the game status on the terminal, updating the summary as the game evolves. It includes the status of each code drop (both location and status), field agent (location, direction and status) as well as statistics: Elapsed Time, number of active agents, number of active teams, number of code drops yet to be neutralized.

	4. It will also output all of its activity to a logfile.
    
**Functional decomposition into modules**
    
* socketSetup: Preparing the datagram socket
* handleSocket: Receives the datagram, parses the input and updates the data structures according to the OPCODEs. 
* parseInput: Helper function to parse the input on 2 levels: First, tokenize the string according to the pipes as delimiters, then, validate the OPCODEs and check the OPCODES to see how many tokens the string should have/if the tokens in the string matches the options available in the OPCODE.
* receiveMsg: Helper function for handleSocket which allows the GS to receive datagrams from GA/FAs - 
* sendMsg: Helper function for handleSocket which allows the GS to send datagrams back to the GA/FAs - 
* createSummary: Output a summary of the game status onto the terminal
    
    In addition, we will be using the following data structures created in TSE:
- Hashtables
- Lists
    
**Dataflow through modules**

1. Create a socket for the GS

2. In a while loop that runs continuously, the incoming datagrams from the GAs and FAs will be handled by handleSocket, contingent on the OPCODEs. 
    
3. At the end of handling each datagram, call createSummary to print a textual summary to the terminal.

**Major data structures**
    
3 Types of Structs:
	
	Struct Team: 
		guideID:
		guideStatus:
		guideLastSeen:
		teamStatus:
		numOperationsActive:
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

    
**Extensions**

Level 2 and Level 3 Game modes
    


