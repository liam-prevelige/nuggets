/* 
 * game.c - component of Nuggets, see game.h for documentation
 * 
 * Lily Scott, Eliza Crocker, Liam Prevelige May 2021
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "grid.h"
#include "game.h"
#include "player.h"
#include "message.h"

static const int MaxNameLength = 50;   // maximum number of chars in playerName
static const int MaxPlayers = 26;      // maximum number of players
static const int GoldTotal = 250;      // number of gold nuggets in the game
static const int GoldMinNumPiles = 10;  // minimum number of gold piles
static const int GoldMaxNumPiles = 30;  // maximum number of gold piles

/**************** global types ***************/
typedef struct game {
  player_t** playerArray;
  grid_t* grid;
  player_t* spectator;
  int nextPlayerNumber;
  int numPlayersQuit;
  int pilesRemaining;
  int nuggetsRemaining;
} game_t;

/**************** local function prototypes  ****************/
static int playerQuit(game_t* game, addr_t* address);
static bool spectatorQuit(game_t* game);
static int updateAllPlayers(game_t* game, player_t* movedPlayer, int goldChange);
static void sendOK(addr_t* address, char letter);
static void sendGold(addr_t* address, int justCollected, int updatedPurse,
                     int nuggetsRemaining);
static void sendGrid(addr_t* address, int numRows, int numCols);
static void sendDisplay(addr_t* address, char* gridString);


/**************** global functions ****************/

/**************** game_new() ***************/
/* see game.h for documentation */
game_t*
game_new(char* mapName) {
  if (mapName != NULL) {    // check param
    game_t* game = malloc(sizeof(game_t)); // allocate memory for game
    // initialize all variables
    if (game != NULL) {
      // starts empty, allows up to MaxPlayers to join
      game->playerArray = calloc(MaxPlayers, sizeof(player_t*));
      // grid initialization handled by grid module
      game->grid = grid_initialize(mapName, GoldMinNumPiles, GoldMaxNumPiles, &(game->pilesRemaining));
      if (game->grid == NULL){
        return NULL;
        fprintf(stderr, "game_newGame: error initializing grid\n");
      }
      game->nextPlayerNumber = 0;
      game->numPlayersQuit = 0;
      game->spectator = NULL;
      game->nuggetsRemaining = GoldTotal;
      return game; // initialized game pointer
    }
    else { // game memory allocation error
      fprintf(stderr, "game_new: error creating game\n");
      return NULL;
    }
  }
  else {  // NULL mapName    
    fprintf(stderr, "game_new: called with NULL mapName\n");
    return NULL;
  }
}

/*************** game_newPlayer() ***************/
/* see game.h for documentation */
bool
game_newPlayer(game_t* game, addr_t* address, char* realName) {
  if (game != NULL) { // check game param
    if (message_isAddr(*address)) { //check address param
      if (realName == NULL) {
        message_send(*address, "QUIT Sorry - you must provide player's name.");
        return false;
      } 
      bool hasLetter = false;
      // loop through realName to see if there are non blank characters
      for (int i = 0; i < strlen(realName); i++){
        if(!isblank(realName[i])){
          hasLetter = true;
          break;
        }
      }
      if (hasLetter == false) { // if only blank
        message_send(*address, "QUIT Sorry - you must provide player's name.");
        return false;
      }
      else if (game->nextPlayerNumber >= MaxPlayers){ // check if game can accept more players
        message_send(*address, "QUIT Game is full: no more players can join.");
        return false;
      }
      else {
        player_t* player = player_new(address, realName, MaxNameLength); // initialize player
        if (player != NULL) {
          char letter = 'A' + game->nextPlayerNumber; // assign player letter
          grid_playerToGrid(game->grid, player, letter); // inserts player into master grid and sets player's grid
          game->playerArray[game->nextPlayerNumber] = player; //insert into array at end
          // send OK, GRID, GOLD and DISPLAY messages
          sendOK(address, letter);
          sendGrid(address, grid_getNR(game->grid), grid_getNC(game->grid));
          sendGold(address, 0, player_getPurse(player), game->nuggetsRemaining);
          sendDisplay(address, player_getGrid(player));   
          if (updateAllPlayers(game, player, 0) != 0){ // update all player's grids
            fprintf(stderr, "game_newPlayer: error updating all players\n");
            return false;
          }
          game->nextPlayerNumber++; // increment nextPlayerNumber
        }
        else {
          fprintf(stderr, "game_newPlayer: error allocating memory for player\n");
          return false;
        }
      }
      return true; // successfully added player
    }
    else { // address not valid
      fprintf(stderr, "game_newPlayer: called invalid player address\n");
      return false;
    }
  }
  else { // NULL game
    fprintf(stderr, "game_newPlayer: called with NULL game\n");
    return false;
  }
}

/*************** game_newSpectator() *************/
/* see game.h for documentation */
bool
game_newSpectator(game_t* game, addr_t* address){
  if (game != NULL) { // check game param
    if (message_isAddr(*address)){ // validate address
      if (game->spectator != NULL) { // check if game has spectator
        if (!spectatorQuit(game)){ // quit that spectator and check for error
          fprintf(stderr, "game_newSpectator: error quitting the previous spectator\n");
          return false;
        }
      }
      player_t* player = player_new(address, "", MaxNameLength); // create new player without name
      if (player != NULL) { // make sure successful
        char* grid = malloc(strlen(grid_getMasterGrid(game->grid)) + 1);
        if (grid != NULL) {
          strcpy(grid, grid_getMasterGrid(game->grid)); // copy masterGrid for spectator
          sendGrid(address, grid_getNR(game->grid), grid_getNC(game->grid));
          sendGold(address, 0, 0, game->nuggetsRemaining);
          sendDisplay(address, grid);
          player_setGrid(player, grid); // set player's grid to grid
          free(grid);
          game->spectator = player;
          return true; // successfully added spectator
        }
        else { // defense check memory allocation
          fprintf(stderr, "game_newSpectator: error allocating grid\n");
        }
      }
      else { // defense check memory allocation
        fprintf(stderr, "game_newSpectator: error allocating player\n");
      }
    }
    else { // address invalid
      fprintf(stderr, "game_spectatorNew: called invalid address\n");
    }
  }
  else { // game NULL
    fprintf(stderr, "game_spectatorNew: called with NULL game\n");
  }
  return false; // unsuccessful
}

/*************** game_playerMove() *************/
/* see game.h for documentation */
int
game_playerMove(game_t* game, addr_t* address, char commandKey){
  if (game != NULL){ // check game param
    if (message_isAddr(*address)) { // check address param
    player_t* player = NULL;
    char playerChar;
    int goldChange = 0;

    // identify player to move
    for (int i = 0; i < game->nextPlayerNumber; i++) { // loop through playerArray
    // check if the player's address matches the target address
      if (message_eqAddr(*address, *(player_getAddr(game->playerArray[i])))) { 
        player = game->playerArray[i]; // set player to player matching
        playerChar = 'A' + i; // calculate playerChar from array index
        break;
      }
    }
    // check player identified
    if (player == NULL) { // player not in playerArray
      if (message_eqAddr(*address, *(player_getAddr(game->spectator)))){ // check if address matches spectator
        message_send(*address, "ERROR usage: spectator cannot move\n"); // send spectator usage error
      }
      fprintf(stderr, "game_playerMove: address does not match any player\n");
      return 1; // player to move 
    }
    
    //find destination index based on commandKey

    int destinationIdx;
    //handle capital letters
    if (tolower(commandKey) != commandKey) {
      int numIterations = 0;
      while (game_playerMove(game, address, tolower(commandKey)) == 0) {
        numIterations++;
      }
      if (numIterations == 0) { // make sure that move of lowercase successful
        return 1;
      }
      return 0;
    }
    int NC = grid_getNC(game->grid);
    switch(commandKey) {
      case 'h':  // left 
        destinationIdx = player_getLoc(player) - 1;
        break;
      case 'l':  // right   
        destinationIdx = player_getLoc(player) + 1;
        break;
      case 'j':  // down
        destinationIdx = player_getLoc(player) + NC;
        break;
      case 'k':  // up
        destinationIdx = player_getLoc(player) - NC;
        break;
      case 'y':  // diag up left
        destinationIdx = player_getLoc(player) - NC - 1;
        break;
      case 'u':  // diag up right
        destinationIdx = player_getLoc(player) - NC + 1;
        break;
      case 'b':  // diag down left
        destinationIdx = player_getLoc(player) + NC - 1;
        break;
      case 'n':  // diag down right
        destinationIdx = player_getLoc(player) + NC + 1;
        break;
      default:
        printf("default");
        return 1; // invalid key
    }
    // get the character at destination
    char* masterGrid = grid_getMasterGrid(game->grid);
    char destinationChar = masterGrid[destinationIdx];
    // handle different cases of destinationChar

    // player hits another player
    if (isalpha(destinationChar)) {
      player_t* destinationPlayer = game->playerArray[destinationChar - 'A']; // identify player at destination
      if (destinationPlayer == NULL) {
        fprintf(stderr, "game_playerMove: unable to find player corresponding to letter intersected: FATAL ERROR\n");
        return -1;
      }
      // switch players in master
      grid_updateMaster(game->grid, player_getLoc(destinationPlayer), playerChar);
      grid_updateMaster(game->grid, player_getLoc(player), destinationChar);
      // switch player locations
      player_setLoc(destinationPlayer, player_getLoc(player));
      player_setLoc(player, destinationIdx);
    }
    // player hits gold pile
    else if (destinationChar == '*') {
      // not last pile
      if (game->pilesRemaining > 1) {
        int maxInPile = 10 + game->nuggetsRemaining / game->pilesRemaining;
        goldChange = rand() % maxInPile + 1; // calculate random amount of gold in pile
        game->nuggetsRemaining -= goldChange;
        game->pilesRemaining -= 1;
        player_addPurse(player, goldChange); // give gold to player purse
      }
      // last pile
      else if (game->pilesRemaining == 1) {
        goldChange = game->nuggetsRemaining; //
        player_addPurse(player, goldChange);
        sendGold(player_getAddr(player), goldChange, player_getPurse(player), game->nuggetsRemaining);
        game_endGame(game);
        return -1;
      }
      char* originalGrid = grid_getOriginalGrid(game->grid); 
      grid_updateMaster(game->grid, player_getLoc(player), originalGrid[player_getLoc(player)]); // switch gold pile to origin character
      grid_updateMaster(game->grid, destinationIdx, playerChar); // update master with player
      player_setLoc(player, destinationIdx); // update player location
    }
    else if (destinationChar != '.' && destinationChar != '#') {
      // not a valid space for player to move
      return 1;
    }
    else{
      // update player's location with original grid and move player
      char* originalGrid = grid_getOriginalGrid(game->grid); 
      grid_updateMaster(game->grid, player_getLoc(player), originalGrid[player_getLoc(player)]);
      grid_updateMaster(game->grid, destinationIdx, playerChar);
      player_setLoc(player, destinationIdx);
    }             
    return updateAllPlayers(game, player, goldChange);
    }
    else { // invalid address
        fprintf(stderr, "game_playerMove: called with invalid address\n");
    }
  }
  else { // game NULL
    fprintf(stderr, "game_playerMove: called with NULL game\n");
  }
  return 1;
}

/*************** game_clientQuit() *************/
/* see game.h for documentation */
int
game_clientQuit(game_t* game, addr_t* address){
  if (game != NULL){ // check game param
    if (message_isAddr(*address)){ // validate address
      if (game->spectator != NULL){ // check if address to quit is spectator
        if (message_eqAddr(*address, *(player_getAddr(game->spectator)))){
          spectatorQuit(game); // call spectator quit
          return 0;
        }
        // otherwise in both scenarios quit player
        else {
          return playerQuit(game, address);
        }
      }
      else {
        return playerQuit(game, address);
      }
    }
    else { // invalid address
      fprintf(stderr, "game_clientQuit: called with invalid address\n");
      return 1;
      }
    }
  else { // NULL game
    fprintf(stderr, "game_clientQuit: called with NULL game\n");
    return 1;
  }
}

/*************** playerQuit() *************/
/* Change a player's game status and send them a QUIT message.                             
 *                                                                                         
 * Caller provides:                                                                        
 *   valid address to a player                                                             
 * We guarantee:                                                                           
 *   valid game pointer                                                                    
 *   the provided address is valid                                                         
 *   if the player is in the game, we change their status to false                         
 *   and send them a quit message                                                          
 * We return:                                                                              
 *   0 is successful, 1 if unsuccessful, -1 if need to endGame                             
 */
static int
playerQuit(game_t* game, addr_t* address) {
  if (game != NULL) { // check game param
    if (message_isAddr(*address)) { // validate address
    // loop through players to find player with target address
      int i;
      for (i = 0; i < game->nextPlayerNumber; i++) {
        if (message_eqAddr(*address, *(player_getAddr(game->playerArray[i])))) {
          if (player_getStatus(game->playerArray[i])) { // if still playing
            grid_updateMaster(game->grid, player_getLoc(game->playerArray[i]), '!'); // update master (!) is remove character indicator
            player_quitGame(game->playerArray[i]); 
            message_send(*address, "QUIT Thanks for playing!");
            game->numPlayersQuit++; // increment playersQuit
            if (game->numPlayersQuit == MaxPlayers){ // check if game can still accept players
              // all possible players have joined and exited so game is over
              game_endGame(game);
              return -1;
            }
            return updateAllPlayers(game, game->playerArray[i], 0); // update all players
          }
          else {
            fprintf(stderr, "game_playerQuit: called on player with game status already false\n");
          }
        }
      }
      if (i == game->nextPlayerNumber) { // finished loop without finding player
        fprintf(stderr, "game_playerQuit: address not in the game\n");
        return 1;
      }
    }
    else{ // invalid address
      fprintf(stderr, "game_playerQuit: called with invalid address\n");
      return 1;
    }
  }
  else { // game NULL
    fprintf(stderr, "game_playerQuit: called with NULL game\n");
    return 1;
  }
  return 1;
}

/*************** spectatorQuit() *************/
/* Send a QUIT message to the spectator, delete spectator and remove them from the game.   
 *                                                                                         
 * Caller provides:                                                                        
 *   valid address to a client                                                             
 * We guarantee:                                                                           
 *   valid game pointer                                                                    
 *   game has spectator                                                                    
 * We return:                                                                              
 *   true if successful otherwise false                                                    
 */
static bool
spectatorQuit(game_t* game) {
  if (game != NULL) { // check game param
    if (game->spectator != NULL) { // check that game has specator
      addr_t* address = player_getAddr(game->spectator);
      message_send(*address, "QUIT Thanks for watching!");
      player_delete(game->spectator);
      game->spectator = NULL;
      return true;
    } 
    else { // game does have spectator to quit
    fprintf(stderr, "game_spectatorQuit: called without spectator\n");
    }
  }
  else { // game NULL
  fprintf(stderr, "game_spectatorQuit: called with NULL game\n");
  }
  return false;
}

/*************** game_endGame() *************/
/* see game.h for documentation */
void
game_endGame(game_t* game) {
  if (game != NULL){ // check game param
    int lineLength = 14 + MaxNameLength;
    char* header = "QUIT GAME OVER:\n";
    char* summary = malloc(strlen(header) + (lineLength * game->nextPlayerNumber) + 1); // allocate enough memory for entire summary
    sprintf(summary, "%s", ""); // insert a null terminator
    strcat(summary, header); // add header to summary
    // loop through all players and get their summary
    for (int i = 0; i < game->nextPlayerNumber; i++) {
      char* line = player_summary(game->playerArray[i], i);
      if (strcmp(line, "")) { // if not empty
        strcat(summary, line); // add player's summary to summary
      }
      free(line); // free player summary
    }
    // loop through all players and send player summary
    for (int i = 0; i < game->nextPlayerNumber; i++) {
      if(player_getStatus(game->playerArray[i])){
        message_send(*(player_getAddr(game->playerArray[i])), summary);
      }
      player_delete(game->playerArray[i]); // free each player
    }
    free(game->playerArray); // free array after all players free
    grid_delete(game->grid); // delete grid

    if (game->spectator != NULL) { // if has spectator
      message_send(*(player_getAddr(game->spectator)), summary); // send spectator end game summary
      player_delete(game->spectator); // delete spectator
    }
    free(summary);
    free(game);
  }
  else { // game NULL
    fprintf(stderr, "game_endGame: called with NULL game\n");
  }
}

/*************** updateAllPlayers() *************/
/* 
 * loops through all players and spectator
 * sends updated display and gold if applicable
 * returns 0 if successful, 1 if error, and -1 if fatal error
 */
static int updateAllPlayers(game_t* game, player_t* movedPlayer, int goldChange){
  if (game != NULL) { // check game param
    if (movedPlayer != NULL) { // check movedPlayer param
      // if game has spectator update display and gold if change
      if (game->spectator != NULL){
        sendDisplay(player_getAddr(game->spectator), grid_getMasterGrid(game->grid));
        if (goldChange > 0){
          sendGold(player_getAddr(game->spectator), 0, 0, game->nuggetsRemaining);
        }
      } 
      // looop through all players and send updated displays and gold    
      for (int i = 0; i < game->nextPlayerNumber; i++) {
        char* updatedGrid = grid_updatePlayerGrid(game->grid, game->playerArray[i]);
        if (updatedGrid == NULL) {
          fprintf(stderr, "updateAllGrids: updatePlayerGrid failed: FATAL ERROR\n");
          return -1;
        }
        sendDisplay(player_getAddr(game->playerArray[i]), updatedGrid);
        player_setGrid(game->playerArray[i], updatedGrid); // update player's grid
        free(updatedGrid);
        if (goldChange > 0){ // if gold changed send gold messages
          // if player is the player that moved include just collected gold, and updated purse  
          if (game->playerArray[i] == movedPlayer) {
            sendGold(player_getAddr(movedPlayer), goldChange, player_getPurse(movedPlayer), game->nuggetsRemaining);
          }
          else { // other players get unchanged purse and 0 gold just collected
            sendGold(player_getAddr(game->playerArray[i]), 0, player_getPurse(game->playerArray[i]),
                    game->nuggetsRemaining);
          }
        }
      }
      return 0; // loop successful
    } 
    else { // player NULL
      fprintf(stderr, "updateAllPlayers: called with NULL player\n");
    }
  }
  else { // game NULL
    fprintf(stderr, "updateAllPlayers: called with NULL game\n");
  }
  return 1; // unsuccessful
}

/*************** sendOK() *************/
/*
 * creates and sends OK message
*/
static void sendOK(addr_t* address, char letter){
  if (address != NULL && message_isAddr(*address)){ // check params
    char* message = malloc(strlen("OK ") + 2); // allocate memory for message
    sprintf(message, "%s%c", "OK ", letter); // add letter
    fprintf(stderr, "sendOK: %s\n", message); // log to stderr
    message_send(*address, message); // send message
    free(message);
  }
  else { // invalid params
    fprintf(stderr, "sendOK: invalid arguments\n");
  }
}

/*************** sendGrid() *************/
/*
 * creates and sends grid message
*/
static void sendGrid(addr_t* address, int numRows, int numCols) {
  if (address != NULL && message_isAddr(*address) && numRows > 0 && numCols > 0){ // check params
    char* message = malloc(strlen("GRID ") + 10); // allocate memory for message
    char stringRows[4];
    char stringCols[4];
    sprintf(stringRows, "%d", numRows); // convert numRows to string
    sprintf(stringCols, "%d", numCols); // convert numCols to string
    strcpy(message, "GRID "); // copy GRID to message
    strcat(message, stringRows); // add rows
    strcat(message, " "); // add space
    strcat(message, stringCols); // add cols
    fprintf(stderr, "sendGrid: %s\n", message); // log to stderr
    message_send(*address, message); // send message
    free(message); 
  }
  else { // invalid params
    fprintf(stderr, "sendGrid: invalid arguments\n");
  }
}

/*************** sendGold() *************/
/*
 * creates and sends gold message
*/
static void sendGold(addr_t* address, int justCollected, int updatedPurse, int nuggetsRemaining) {
  if (message_isAddr(*address) && justCollected >= 0 && updatedPurse >= 0 && nuggetsRemaining>= 0){ //check params
    char* message = malloc(strlen("GOLD ") + 12); // allocate memory for message
    char stringJustCollected[3];
    char stringUpdatedPurse[3];
    char stringNuggetsRemaining[3];
    sprintf(stringJustCollected, "%d", justCollected); // convert justCollected to string
    sprintf(stringUpdatedPurse, "%d", updatedPurse); // convert updatedPurse to string
    sprintf(stringNuggetsRemaining, "%d", nuggetsRemaining); // convert nuggetsRemaining to string
    strcpy(message, "GOLD "); // copy GOLD to message
    strcat(message, stringJustCollected); // add justCollected
    strcat(message, " "); // add space
    strcat(message, stringUpdatedPurse); // add updatedPurse
    strcat(message, " "); // add space
    strcat(message, stringNuggetsRemaining); // add nuggetsRemaining
    fprintf(stderr, "sendGold: %s\n", message); // log to stderr
    message_send(*address, message); // send message
    free(message);
  }
  else { // invalid params
    fprintf(stderr, "sendGold: invalid arguments\n");
  }
}

/*************** sendDisplay() *************/
/*
 * creates and sends display message
*/
static void sendDisplay(addr_t* address, char* gridString) {
  if (address != NULL && message_isAddr(*address) && gridString != NULL){ // check params
    char* message = malloc(strlen("DISPLAY\n") + strlen(gridString) + 1); // allocate memory for message
    strcpy(message, "DISPLAY\n"); // copy DISPLAY\n to message 
    strcat(message, gridString); // add gridString
    fprintf(stderr, "sendDisplay:\n"); // log to stderr
    message_send(*address, message); // send message
    free(message);
  }
  else { //invalid params
    fprintf(stderr, "sendDisplay: invalid arguments\n");
  }
}
