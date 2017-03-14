/*  log.c a module which handles the logging to files

  Project name: Project Incomputable
  Component name: log.c 

  Primary Author: Max Zhuang, Raunak Bhojwani, Samuel Ching
  Date Created: 5/30/16

  Special considerations:  
    (e.g., special compilation options, platform limitations, etc.) 
  
======================================================================*/

/** Global Variables **/
//Log File Path
char* logFilePath = "gameserver.log";

/***** Global Includes *****/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <time.h>

#include "../lib/memory.h"


/************************** game_mode_logging ****************************/

bool game_mode_logging(char* logPurpose, char *message, double elapsedTime)
{ 

  FILE *fp = fopen(logFilePath, "a");
  if (fp == NULL) {
    fprintf(stderr, "gameserver.c: Cannot open gameserver logfile.\n");
    return false;
  }
  
  fprintf(fp, "Elapsed GameTime: %f\t Log Code:%s\n", elapsedTime , logPurpose);
  fprintf(fp, "------------------------------------------------------------------------\n");
  fprintf(fp, "%s\n", message);

  count_free(message);

  fclose(fp);

  return true;
}

/************************** error_logging ****************************/


bool error_logging(char* errorMsg)
{
  
  FILE *fp = fopen(logFilePath, "a");
  if (fp == NULL) {
    fprintf(stderr, "gameserver.c: Cannot open gameserver logfile.\n");
    return false;
  }  
  
  fprintf(fp, "Error Found: %s\n", errorMsg);

  fclose(fp);

  return true; 
}



/************************** raw_mode_logging_outbound ****************************/
bool raw_mode_logging_outbound(char* message, time_t timeStamp, double elapsedTime)
{
	FILE *fp = fopen(logFilePath, "a");
  	
  	if (fp == NULL) {
    fprintf(stderr, "gameserver.c: Cannot open gameserver logfile.\n");
    return false;
  	}
  	fprintf(fp, "Elapsed GameTime: %f\n", elapsedTime);
  	fprintf(fp, "%s: \tOutBound Message: %s\n", ctime(&timeStamp), message);

  	fclose(fp);
  	return true;
}

/************************** raw_mode_logging_inbound ****************************/

bool raw_mode_logging_inbound(char* message, time_t timeStamp, double elapsedTime)
{
	FILE *fp = fopen(logFilePath, "a");
  	
  	if (fp == NULL) {
    fprintf(stderr, "gameserver.c: Cannot open gameserver logfile.\n");
    return false;
  	}

  	fprintf(fp, "Elapsed GameTime: %f\n", elapsedTime);
  	fprintf(fp, "%s:\tInBound Message: %s\n", ctime(&timeStamp), message);

  	fclose(fp);
  	return true;
}


/************************** log_header ****************************/

bool log_header(time_t sysTime, char* gameID, int portNumber, char* ipAddr)
{	
	FILE *fp = fopen(logFilePath, "a");
  	
  	if (fp == NULL) {
    fprintf(stderr, "gameserver.c: Cannot open gameserver logfile.\n");
    return false;
  	}
  	struct tm* timeinfo;
  	timeinfo = localtime(&sysTime);

	 
  	fprintf(fp, "\n\n****************************** New Game *******************************\n\n\n");
  	fprintf(fp, "Current System Date and Time is %s\n", asctime(timeinfo));
  	fprintf(fp, "Current GameID is %s\n", gameID);
  	fprintf(fp, "The Server's IP Address is %s\n", ipAddr);
  	fprintf(fp, "The server port number is %d\n", portNumber);
  	fprintf(fp, "Let the Games begin\n\n");

  	fclose(fp);
  	return true;
}