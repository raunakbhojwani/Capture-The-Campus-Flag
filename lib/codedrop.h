/* 	codedrop.h module for creating a team struct

	Project name: Project Incomputable
	Component name: Lib/codedrop.h

	This file contains ...
	
	Primary Author:	
	Date Created:	
	
======================================================================*/
// do not remove any of these sections, even if they are empty


// ---------------- Prerequisites e.g., Requires "math.h"

// ---------------- Constants

// ---------------- Structures/Types

typedef struct codedrop codedrop_t;

// ---------------- Public Variables

// ---------------- Prototypes/Macros

/** Creates a new codedrop struct **/

codedrop_t* codedrop_new(float codeLat, float codeLong, char* codeID, char* codeStatus);

/** updates the code status **/

int update_code_status(codedrop_t* codedrop, char* neutralizingTeam, char* codeStatus);

/** deletes the codedrop struct **/

void codedrop_del(codedrop_t* codedrop);

/**Getter Method for code drop status**/

char* get_code_status(codedrop_t* codedrop);


/** Getter method for the latitude of the code drop **/

float get_code_lat(codedrop_t* codedrop);

/** Getter method for the longtitude of the code drop **/

float get_code_long(codedrop_t* codedrop);

/** Getter method to get the Neutralizing team for the code drop */
char* get_neuteam(codedrop_t* codedrop);










