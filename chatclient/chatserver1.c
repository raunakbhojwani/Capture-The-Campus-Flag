/* 
 * chatserver - an example chat server using UDP.
 *
 * Read messages from a UDP socket and print them to stdout;
 * read messages from stdin and send them back to client.
 * 
 * usage: ./chatserver
 *  and ctrl-C to exit.
 *
 * David Kotz, May 2016
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>	      // bool type
#include <unistd.h>	      // read, write, close
#include <string.h>	      // strlen
#include <strings.h>	      // bcopy, bzero
#include <arpa/inet.h>	      // socket-related calls
#include <sys/select.h>	      // select-related stuff 
#include "file.h"	      // readline

/**************** file-local constants and functions ****************/
static const int BUFSIZE = 1024;     // read/write buffer size
static int socket_setup();

/**************** main() ****************/
int
main(const int argc, char *argv[])
{
  // no arguments to parse

  // set up a socket on which to receive messages
  int comm_sock = socket_setup();
  
  // Receive datagrams, print them out, read response from term, send it back
  while (true) {	      // loop exits on EOF from stdin
    char buf[BUFSIZE];	      // buffer for reading data from socket
    struct sockaddr_in them;  // sender's address
    struct sockaddr *themp = (struct sockaddr *) &them;
    socklen_t themlen = sizeof(them);
    int nbytes = recvfrom(comm_sock, buf, BUFSIZE-1, 0, themp, &themlen);

    if (nbytes < 0) {
      perror("receiving from socket");
      exit(1);
    } else {      
      buf[nbytes] = '\0';     // null terminate string

      // where was it from?
      if (them.sin_family != AF_INET)
	printf("From non-Internet address: Family %d\n", them.sin_family);
      else
	printf("[%s@%05d]: %s\n", 
	       inet_ntoa(them.sin_addr),
	       ntohs(them.sin_port), 
	       buf);

      // send a response back to the client
      printf("Response?: ");
      char *response = readline(stdin);
      if (response == NULL) 
	break; // on EOF, exit the loop

      if (sendto(comm_sock, response, strlen(response), 0, 
		 (struct sockaddr *) &them, sizeof(them)) < 0){
	perror("sending in datagram socket");
	exit(6);
      }
      free(response);
    }
  }

  close(comm_sock);
  putchar('\n');
  exit(0);
}

/**************** socket_setup ****************/
/* All the ugly work of preparing the datagram socket;
 * exit program on any error.
 */
static int
socket_setup()
{
  // Create socket on which to listen (file descriptor)
  int comm_sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (comm_sock < 0) {
    perror("opening datagram socket");
    exit(1);
  }

  // Name socket using wildcards
  struct sockaddr_in server;  // server address
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = 0;
  if (bind(comm_sock, (struct sockaddr *) &server, sizeof(server))) {
    perror("binding socket name");
    exit(2);
  }

  // Print our assigned port number
  socklen_t serverlen = sizeof(server);	// length of the server address
  if (getsockname(comm_sock, (struct sockaddr *) &server, &serverlen)) {
    perror("getting socket name");
    exit(3);
  }
  printf("Ready at port %d\n", ntohs(server.sin_port));

  return (comm_sock);
}
