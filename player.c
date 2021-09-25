/*
 * player.c - component of Nuggets, see player.h for documentation
 *
 * Lily Scott, Liam Prevelige, Eliza Crocker May 2021
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "player.h"

/*************** global types ***************/
typedef struct player {
  addr_t* address;
  char* name;
  char* grid;
  int purse;
  int location;
  bool status;
} player_t;

/*************** local function prototypes **************/
static char* normalizeName(char* name, const int maxNameLength);

/*************** global functions ***************/

/********************************************/
/* getter functions - see player.h for documentation */

addr_t*
player_getAddr(player_t* player) {
  if (player != NULL && player->address != NULL){
    return player->address;
  }
  return NULL;  // error
}

char*
player_getName(player_t* player) {
  if (player != NULL && player->name != NULL){
    return player->name;
  }
  return NULL;  // error
}

char*
player_getGrid(player_t* player) {
   if (player != NULL && player->grid != NULL){
    return player->grid;
  }
   return NULL; // error
}

int
player_getPurse(player_t* player) {
  if (player != NULL && player->purse > -1){
    return player->purse;
  }
  return -1;    // error
}

int
player_getLoc(player_t* player) {
  if (player != NULL && player->location > -1){
    return player->location;
  }
  return -1;    // error
}

bool
player_getStatus(player_t* player) {
  if (player != NULL){
    return player->status;
  }
  return false; // error
}

/********************************************/
/* setter functions - see player.h for documentation */

bool
player_setGrid(player_t* player, char* newGrid) {

  // check params
  if (player != NULL && newGrid != NULL) {
    if (player_getGrid(player) != NULL) {
      free(player->grid);
    }
    // allocate memory for new grid, copy param and assign pointer
    char* cpGrid = malloc(strlen(newGrid) + 1);
    if (cpGrid != NULL){
      strcpy(cpGrid, newGrid);
      player->grid = cpGrid;
      return true;
    }
    else {       // memory allocation error
      fprintf(stderr, "player_setGrid: error allocating memory for grid");
      return false;
    }
  }
  return false;  // param error 
}

bool
player_addPurse(player_t* player, int newGold) {

  // check params, only allow if player has name
  if (player != NULL && strlen(player->name) > 0 && newGold > 0) {

    // add new gold
    player->purse += newGold;
    return true;
  }
  return false;  // error
}

bool
player_setLoc(player_t* player, int newLoc) {

  // check params, only allow if player has name
  if (player != NULL && strlen(player->name) > 0 && newLoc >= 0
      && newLoc < strlen(player->grid)) {

    // set new location
    player->location = newLoc;
    return true;
  }
  return false;  // error
}

bool
player_quitGame(player_t* player) {

  // check params, only allow if player has name
  if (player != NULL && strlen(player->name) > 0 && player->status) {

    // set new status
    player->status = false;
    return true;
  }
  return false;  // error
}

/********************************************/

/*************** player_new() ***************/
/* see player.h for documentation */
player_t*
player_new(addr_t* add, char* name, const int maxNameLength)
{
  // check params
  if (add == NULL || !message_isAddr(*add) || name == NULL || maxNameLength < 0) {
    return NULL;
  }
  
  // create the player
  player_t* player = malloc(sizeof(player_t));
  if (player == NULL) {
    return NULL;                                         // error allocating memory
  } else {
    // initialize all values:

    // address given
    player->address = malloc(sizeof(addr_t));
    *(player->address) = *add;

    // name normalized then copied, any externally allocated memory should be freed
    char* newName = normalizeName(name, maxNameLength);  // normalize name
    if (newName == NULL) {
      return NULL;                                       // error allocating memory
    } else player->name = newName;

    // grid initialized externally
    player->grid = NULL;

    // location initialized -1 for external error checking
    player->location = -1;

    // purse initialized empty
    player->purse = 0;

    // status starts true, i.e., player is in game
    player->status = true;
  }

  // initialized player pointer
  return player;
}

/*************** normalizeName() *****************/
/* Normalizes a name according to the Requirements Spec.
 *
 * We return:
 *   the name, normalized
 *
 * We guarantee:
 *   if the name is longer than maxNameLength, any excess characters
 *    are truncated
 *   any nongraphic or blank characters are converted to underscores
 */
static char*
normalizeName(char* name, const int maxNameLength)
{
  // decide whether needs to truncate and get new length
  char* newName;
  int newLength;
  if (strlen(name) > maxNameLength) {
    newLength = maxNameLength;
  } else newLength = strlen(name);

  // allocate memory
  newName = malloc(sizeof(char) * newLength + 1);
  if (newName == NULL) {
    return NULL;
  }
  
  // copy name to newName char by char, truncates if too long
  for (int i = 0; i < newLength; i++) {
    if (!isgraph(name[i]) && !isblank(name[i])) {
      newName[i] = '_';
    } else newName[i] = name[i];
  }

  // terminate newName
  newName[newLength] = '\0';

  // the normalized name
  return newName;
}

/*************** player_summary() ****************/
/* see player.h for documentation */
char*
player_summary(player_t* player, int id)
{
  // get player's letter, if spectator or passed id invalid return empty string
  char c = 'A' + id;
  if (player == NULL || player_getName(player) == NULL || player_getPurse(player) < 0
      || !strcmp(player->name, "") || c < 'A' || c > 'Z') {
    return "";
  }

  // allocate memory for the summary
  char* summary = malloc((sizeof(char) * (14 + strlen(player->name) + 1)));
  if (summary == NULL) {
    return "";    // error allocating memory
  }

  // construct summary w/ player letter, purse, name, then return
  sprintf(summary, "%c %10d %s\n", c, player->purse, player->name);
  return summary;
}

/**************** player_delete() ****************/
/* see player.h for description */
void
player_delete(player_t* player)
{
  // if not NULL, free all allocated memory
  if (player != NULL) {
    free(player->name);          // allocated by player_new
    free(player->address);       // allocated by player_new
    if (player->grid != NULL) {  // only free grid if allocated externally
      free(player->grid);
    }
    free(player);                // allocated by player_new
  }
}
