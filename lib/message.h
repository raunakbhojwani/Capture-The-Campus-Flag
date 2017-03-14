/*	message.h - a module to handle receiving and sending messages

	Project name: Project Incomputable
	Component name: Message.c

	Primary Author:	Max Zhuang, Raunak Bhojwani and Samuel Ching
	Date Created: 5/22/16

	Based off Prof Kotz's chatserver.c module

	Special considerations:  
		(e.g., special compilation options, platform limitations, etc.) 
	
	
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

// ---------------- Constants

// ---------------- Structures/Types

// ---------------- Public Variables

// ---------------- Prototypes/Macros

/* socket_setup(const char* gameServerPort)
 * All the ugly work of preparing the datagram socket;
 * exit program on any error.
 */

int socket_setup(const int gameServerPort, char* hostname);

/* handle_socket()
 * Socket has input ready; receives a datagram and returns it as a string
 * If there is an incoming connection, receive and return the datagram as a char*.
 * Else, return NULL
 * Exit on any socket error. 
 */ 

char* handle_socket(int comm_sock, struct sockaddr_in *themp);


/* send_datagram() 
* Has a message to send out to either the Guide Agent or the Field Agent
*  return 0 if there is no client to whom we can send;
* return 1 if message was sent successfully.
* exit on any socket error
*/ 

int send_datagram (int comm_sock, struct sockaddr_in *themp, char* message);


