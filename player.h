/*
 * player.h - header file for player.c
 *
 * Lily Scott, Eliza Crocker, Liam Prevelige May 2021
 */

#include <stdio.h>
#include "message.h"

/************** global types **************/
typedef struct player player_t;

/************** getters **************/

/************** player_getAddr **************/
/* Returns a player's address.
 *
 * Caller provides:
 *   valid pointer to a player
 *
 * We return:
 *   the player's address
 *   NULL if the player or its address are NULL
 */
addr_t* player_getAddr(player_t* player);

/************** player_getName **************/
/* Returns a player's name.
 *
 * Caller provides:
 *   valid pointer to a player
 * 
 * We return:
 *   the player's name
 *   NULL if the player or its name are NULL
 */
char* player_getName(player_t* player);

/************ player_getGrid *************/
/* Returns a player's grid.
 *
 * Caller provides: 
 *   valid pointer to a player
 *
 * We return:
 *   the player's grid
 *   NULL if the player or its grid are NULL
 */ 
char* player_getGrid(player_t* player);

/************ player_getPurse **************/
/* Returns a player's purse.
 *
 * Caller provides:
 *   valid pointer to a player
 *
 * We return:
 *   the player's purse
 *   -1 if the player is NULL or its purse is < 0
 */
int player_getPurse(player_t* player);

/************ player_getLoc **************/
/* Returns a player's location.
 *
 * Caller provides:
 *   valid pointer to a player
 *
 * We return:
 *   the player's location
 *   -1 if the player is NULL or its location is < 0
 */
int player_getLoc(player_t* player);

/************* player_getStatus *************/
/* Returns a player's status.
 *
 * Caller provides: 
 *   valid pointer to a player
 * 
 * We return:
 *   the player's status
 *   false if the player is NULL
 */
bool player_getStatus(player_t* player);

/************** setters **************/

/************** player_setGrid *************/
/* Sets a given player's grid pointer to a new grid.
 *
 * Caller provides:
 *   valid pointer to a player, valid string pointer to a new grid
 *
 * We return:
 *   true if successful, false otherwise
 * We guarantee:
 *   player and new grid non null
 *   player's grid pointer set to new grid otherwise
 *   free player's old grid if existent
 *   we make a copy of new grid, thus caller is responsible for parameter passed to us
 */
bool player_setGrid(player_t* player, char* newGrid);

/************** player_addPurse *************/
/* Adds gold to a player's purse.
 *
 * Caller provides:
 *   valid pointer to a player, valid positive int amount of new gold to add
 *
 * We return:
 *   true if successful, false otherwise
 * We guarantee:
 *   a spectator may not add to their purse
 *   player non null, newGold > 0
 *   newGold added to player's purse otherwise
 */
bool player_addPurse(player_t* player, int newGold);

/************** player_setLoc **************/
/* Sets a given player's location.
 *
 * Caller provides:
 *   valid pointer to a player, valid location index
 *
 * We return: 
 *   true if successful, false otherwise
 * We guarantee:
 *   a spectator may not set their location
 *   player non null, new location a valid index in grid string
 *   player's location set to new location otherwise
 */
bool player_setLoc(player_t* player, int newLoc);

/************** player_quitGame *************/
/* Changes a player's status to false.
 *
 * Caller provides:
 *   valid pointer to a player
 * 
 * We return:
 *   true if successful, false otherwise
 * We guarantee:
 *   a spectator may not quit the game
 *   player non null, current status is true
 *   player's status set to false otherwise
 */
bool player_quitGame(player_t* player);

/************** primary functions **************/

/************** player_new ***************/
/* Creates a new instance of a player struct.
 *
 * Caller provides:
 *   pointer to a client's address, valid string player's realName,
 *   int representing maxNameLength
 *
 * We return:
 *   pointer to a new player, or NULL if error
 * We guarantee:
 *   address is valid and non null, name is non null, maxNameLength > 0
 *   name is normalized according to Requirements Spec
 *   name is copied, so caller is free to later modify parameter passed
 *   grid starts NULL, location starts -1
 *   purse starts empty, status starts true
 *   return NULL if param or memory allocation errors
 * Caller is responsible for:
 *   later calling player_delete()
 */
player_t* player_new(addr_t* add, char* name, const int maxNameLength);

/*************** player_summary **************/
/* Create summary of player's game performance to be included 
 *  in GAME OVER message.
 *
 * Caller provides: 
 *   valid pointer to a player, id representing letter past A
 * 
 * We return:
 *   a string containing the player's ID letter, the amount
 *     of gold in their purse, and their name
 *   empty string if error or player is spectator
 * We guarantee: 
 *   player is not spectator
 *   player non null, id within valid range A-Z
 *   player's name non null and purse >= 0
 *   otherwise, empty string returned
 *   we construct a summary containing the player's id, purse, and name
 * Caller is responsible for:
 *   later freeing the returned string
 */
char* player_summary(player_t* player, int id);

/*************** player_delete *************/
/* Deletes the player and frees all memory.
 * 
 * Caller provides:
 *   valid pointer to a player
 * 
 * We guarantee:
 *   the player's address and name are freed
 *   if memory has been externally allocated for the grid, the grid is freed
 *   the player itself is freed
 */
void player_delete(player_t* player);
