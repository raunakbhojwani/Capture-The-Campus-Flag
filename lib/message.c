/*	message.c - a module to handle receiving and sending messages

	Project name: Project Incomputable
	Component name: Message.c

	Primary Author:	Max Zhuang, Raunak Bhojwani and Samuel Ching
	Date Created: 5/22/16

	Based off Prof Kotz's chatserver.c module

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
#include <netdb.h>
#include <sys/socket.h>


// ---------------- Local includes  e.g., "file.h"

#include "memory.h"

// ---------------- Constant definitions 

// ---------------- Macro definitions

// ---------------- Structures/Types 

// ---------------- Private variables 

static const int BUFSIZE = 8100;

// ---------------- Private prototypes 

/*====================================================================*/


/**************** socket_setup ****************/

int
socket_setup(const int gameServerPort, char* ipAddr)
{
  // Create socket on which to listen (file descriptor)
  int comm_sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (comm_sock < 0) {
    perror("Opening Datagram Socket");
    exit(1);
  }

  // Name socket using wildcards
  struct sockaddr_in server;  // server address
  server.sin_family = AF_INET; // IPV4
  server.sin_addr.s_addr = INADDR_ANY; //IP Address of the Server
  server.sin_port = htons(gameServerPort);

  if (bind(comm_sock, (struct sockaddr *) &server, sizeof(server))) {
    perror("Binding Socket Name");
    exit(2);
  }

  // Print our assigned port number
  socklen_t serverlen = sizeof(server);	// length of the server address


    // Get HostName
  //struct hostent* serverHost = gethostbyaddr(&server, serverlen, AF_INET);
  //strcpy(hostname, serverHost->h_name);

  //getnameinfo(&server, serverlen, hostname, strlen(hostname), NULL, 0);
  ipAddr = inet_ntoa(server.sin_addr);

  if (getsockname(comm_sock, (struct sockaddr *) &server, &serverlen)) {
    perror("Getting Socket Name");
    exit(3);
  }
  printf("Ready at port %d\n", ntohs(server.sin_port));

  return (comm_sock);
}


/**************** handle_socket ****************/

char* 
handle_socket(int comm_sock, struct sockaddr_in *themp)
{
  // socket has input ready
  struct sockaddr_in sender;		 // sender of this message
  struct sockaddr *senderp = (struct sockaddr *) &sender;
  socklen_t senderlen = sizeof(sender);  // must pass address to length
  char buf[BUFSIZE];	      // buffer for reading data from socket
  int nbytes = recvfrom(comm_sock, buf, BUFSIZE-1, 0, senderp, &senderlen);

  if (nbytes < 0) {
    perror("receiving from socket");
    exit(1);
  } else {      
    buf[nbytes] = '\0';     // null terminate string
    if (sender.sin_family != AF_INET)
      printf("From non-Internet address: Family %d\n", sender.sin_family);
  	else{
      // assign the themp sockaddr_in struct to the sender.
      *themp = sender; 
  		char* message = assertp(count_calloc(BUFSIZE, sizeof(char)), "Memory allocation for message failed.\n");
  		strcpy(message,buf);
      printf("[%s@%05d]: %s\n", inet_ntoa(sender.sin_addr), ntohs(sender.sin_port), message);
  		return message;
  		}
	}
	return NULL;
}

/****************** send_datagram ***************/

int 
send_datagram (int comm_sock, struct sockaddr_in *themp, char* message)
{	
  if (themp->sin_family != AF_INET) {
    printf("You have yet to hear from any client.\n");
    fflush(stdout);
    return 1;
  } 

  if (sendto(comm_sock, message, strlen(message), 0, 
	     (struct sockaddr *) themp, sizeof(*themp)) < 0){
    perror("sending in datagram socket");
    return 2;
  }

  //count_free(message);
  return 0;
}

