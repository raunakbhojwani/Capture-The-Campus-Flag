## README.md for CS50 Final Project: Project Incomputable
### Author: Max Zhuang, Raunak Bhojwani and Max Zhuang
### Date: 31 May, 2016

### Compiling:
	make clean
	make

### Usage:

* gameserver.c is a 

* Project name: Project Incomputable

* Component name: gameserver.c
	
* Primary Author:  Max Zhuang, Raunak Bhojwani and Samuel Ching

* Date Created: 5/31/16


### Example command lines:
* ./gameserver.c -log=raw codedrop.dat 51888


### Exit status:
*	0 - success
*	1 - If the arguments are not entered properly or there is the exit.

### Assumptions:

1. Assumes that if the code drop file path is file-readable, it is the correct codedrop file path. 

2. Assumes that the playername and teamname are 1 string long without whitespaces - the playername and teamname validation checks for this property.

3. Assumes that the GA player will join before the FA player (from the same team) joins. Else, the gameserver will ignore the FA player.

4. Assumes that Game Server port is always open and not taken by another team

5. Assumes that the server port number will be given correctly because we are unable to validate that (i.e. it could be any port number).

### Non-Assumptions:

None.

### Limitations:

1. There will be a time lag of at most 14s between when the game actually ends and when the server declares GAME_OVER. This is due to the fact that the Game Server will be remain open as long as there is no incoming datagram. It is only after the incoming datagram (every 15s by the FA) is handled, that the server will terminate and send out the GAME_OVER.



  














