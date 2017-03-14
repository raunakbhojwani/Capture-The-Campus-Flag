/*  log.h a module which handles the logging to files

  Project name: Project Incomputable
  Component name: log.c 

  Primary Author: Max Zhuang, Raunak Bhojwani, Samuel Ching
  Date Created: 5/30/16

  Special considerations:  
    (e.g., special compilation options, platform limitations, etc.) 
  
======================================================================*/

/** Handles Game Mode Logging - it is triggered by default if the user does not specify the log mode.
* Takes in the log purpose, the log message and elapsed time of the game.
* If log is successful, it will true, else it will return false.
*/ 
bool game_mode_logging(char* logPurpose, char *message, double elapsedTime);

/** Handles error logging - it will log all errors in both raw and game modes. 
* Takes in the error message.
* If log is successful, it will true, else it will return false.
*/ 

bool error_logging(char* errorMsg);


/** Handles Raw Mode Logging of Outbound Messages - it is only triggered if the user specified the raw log mode.
* Takes in the log message, the current timestamp of the local machine and the elapsed game time.
* If log is successful, it will true, else it will return false.
*/ 
bool raw_mode_logging_outbound(char* message, time_t timeStamp, double elapsedTime);

/** Handles Raw Mode Logging of Inbound Messages - it is only triggered if the user specified the raw log mode.
* Takes in the log message, the current timestamp of the local machine and the elapsed game time.
* If log is successful, it will true, else it will return false.
*/ 
bool raw_mode_logging_inbound(char* message, time_t timeStamp, double elapsedTime);


/** Creates the header for the log file everytime there is a new run.
* If log is successful, it will true, else it will return false.
*/ 
bool log_header(time_t sysTime, char* gameID, int portNumber, char* ipAddr);
