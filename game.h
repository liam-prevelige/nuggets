/* 
 * game.h - header file for game.c
 *
 * The `game` module takes on responsibility for the bulk of the gameplay, 
 * implementing the `game` struct and providing functions to start and end the game, 
 * add and remove players and spectators, and allow for player movement.
 * 
 * Lily Scott, Eliza Crocker, Liam Prevelige May 2021
 */

 #include <stdio.h>
 #include "message.h"
 #include "player.h"

/**************** global types ****************/
typedef struct game game_t;

/**************** game_new()) ****************/
/* Create a new game and initialize components.
 *
 * Caller provides:
 *   valid mapFile, we assume valid
 * We guarantee:
 *   the mapName is not NULL
 * We return:
 *   a pointer to game if successful
 *   NULL if error
 */
game_t* game_new(char* mapName);

/**************** game_newPlayer ****************/
/* Create new player and add to playerArray
 * If successful, send GRID, GOLD and DISPLAY messages to client
 * also update all other player's displays
 *
 * Caller provides:
 *   valid game pointer
 *   valid address to a client.
 *   name of the player
 * We guarantee:
 *   the provided address is valid and that a nonwhite character is in the name
 * We return:
 *   true if successful else false
 */
bool game_newPlayer(game_t* game, addr_t* address, char* realName);

/**************** game_newSpectator ****************/
/* Create new player and assign to game spectator
 * if there was already a spectator send it QUIT message
 * If player creation successful, send GRID, GOLD and DISPLAY messages to client
 *
 * Caller provides:
 *   valid game pointer
 *   valid address to a client.
 * We guarantee:
 *   the provided address is valid
* We return:
 *   true if successful else false
 */
bool game_newSpectator(game_t* game, addr_t* address);

/************* game_playerMove **************/
/* Given a character, determine destination
 * handle if another player is at that destination
 *        if gold is at that destination
 *        or if that destination is invalid
 * update master grid based on players changes
 * update all player's displays
 * if gold collected, update all player's golds
 *
 * Caller provides:
 *   valid game pointer
 *   valid address to a client
 *   char indicating move command
 * We guarantee:
 *   game is not NULL
 *   the provided address is valid
 * We return:
 *   0 is successful, 1 if unsuccessful, -1 if fatal error or endGame
 */
int game_playerMove(game_t* game, addr_t* address, char commandKey);

/************* game_clientQuit **************/
/* Calls game_specatatorQuit is address matches spectator address 
 * otherwise calls game_playerQuit
 *
 * Caller provides:
 *   valid address to a client
 * We guarantee:
 *   valid game pointer
 *   provided address is valid
 *   if the player is in the game, we change their status to false
 *   and send them a quit message
 * We return:
 *   0 is successful, 1 if unsuccessful, -1 if need to endGame
 */
int game_clientQuit(game_t* game, addr_t* address);

/*************** game_endGame **************/
/* Ends the game and sends a QUIT message to each player and the spectator
 * containing a summary of the game, then frees all memory.
 *
 * We guarantee:
 *   each player is included in the game summary with their letter representation,
 *   gold count, and name
 *   we send a QUIT message to each player in the playerArray containing the game 
 *   summary, then call player_delete
 *   we free the playerArray and call grid_delete on the masterGrid
 *   if the spectator is nonNULL, we send them a QUIT message containing the
 *   game summary, then call player_delete
 *   we free any memory allocated for the game
 */
void game_endGame(game_t* game);
