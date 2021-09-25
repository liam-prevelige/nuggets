/*
 * playertest.c - unit test for player module of Nuggets
 *
 * simulates gameplay with a dummy player and spectator and tests boundary cases
 *
 * Usage: ./playertest
 *
 * Lily Scott, Eliza Crocker, Liam Prevelige May 2021
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "player.h"
#include "message.h"

/********** main **********/
int
main(const int argc, char* argv[])
{
  // wrong number of arguments
  if (argc != 1) {
    fprintf(stderr, "usage: %s\n", argv[0]);
    exit(1);
  }

  // initialize address, arbitrary for testing player so long as valid
  const char* hostname = "localhost";
  const char* portString = "10804";
  addr_t add = message_noAddr();
  message_setAddr(hostname, portString, &add); 
  if (!message_isAddr(add)) {
    fprintf(stderr, "developer error: invalid address\n");
    exit(2);
  }

  char* playerName = "beyonce";
  int maxNameLength = 50;        // as in specs
  player_t* player = player_new(&add, playerName, maxNameLength);
  if (player == NULL) {
    // since already checked validity of addr and know maxNameLength > 0,
      // memory allocation error only possible reason for NULL return value
    fprintf(stderr, "error: player_new() memory allocation error\n");
    exit(3);
  }

  // now test gameplay functions

  // we've initialized player's address, so should not be null
  if (player_getAddr(player) == NULL) {
    fprintf(stderr, "error: player_getAddr()\n");
    exit(4);
  } else printf("success: player_getAddr()\n");

  // we've given the player a name, so should not be empty
  if (!strcmp(player_getName(player), "")) {
    fprintf(stderr, "error: player_getName()\n");
    exit(5);
  } else printf("success: player_getName() returns %s\n", player_getName(player));

  // we have yet to set the player's grid, so should return the empty string
  if (player_getGrid(player) != NULL) {
    fprintf(stderr, "error: player_getGrid() NULL not returned\n");
    exit(6);
  } else printf("success: player_getGrid() returned NULL\n");
    
  // player grid is initially empty, so setting it should work
  char* grid = "this is a grid";
  if (!player_setGrid(player, grid)) {
    fprintf(stderr, "error: player_setGrid() initial call\n");
    exit(7);
  } else printf("success: player_setGrid() initial call grid: %s\n", grid);

  // should be able to overwrite grid
  grid = "this grid good";
  if (!player_setGrid(player, grid)) {
    fprintf(stderr, "error: player_setGrid()\n");
    exit(8);
  } else printf("success: player_setGrid() with new grid: %s\n", grid);

  // we've set the player's grid, so should give us the proper grid
  if (!strcmp(player_getGrid(player), "")) {
    fprintf(stderr, "error: player_getGrid()\n");
    exit(9);
  } else printf("success: player_getGrid() returns %s\n", player_getGrid(player));

  // player's purse should be empty
  if (player_getPurse(player)) {
    fprintf(stderr, "error: player_getPurse() empty purse\n");
    exit(10);
  } else printf("success: player_getPurse() when empty\n");

  // add to player's purse
  int goldToAdd = 23;
  if (!player_addPurse(player, goldToAdd)) {
    fprintf(stderr, "error: player_addPurse() valid gold amt to empty purse\n");
    exit(11);
  } else printf("success: player_addPurse() with valid gold amt %d\n", goldToAdd);

  // should not be able to subtract gold from purse
  goldToAdd = -2;
  if (player_addPurse(player, goldToAdd)) {
    fprintf(stderr, "error: player_addPurse() invalid gold amt\n");
    exit(12);
  } else printf("success: player_addPurse() did not add invalid gold amt %d\n", goldToAdd);

  // should not be able to "add" zero gold
  goldToAdd = 0;
  if (player_addPurse(player, goldToAdd)) {
    fprintf(stderr, "error: player_addPurse() zero gold\n");
    exit(13);
  } else printf("success: player_addPurse() did not add zero gold\n");

  // get gold amount, should be nonzero since gold added
  if (!player_getPurse(player)) {
    fprintf(stderr, "error: player_getPurse() purse nonempty\n");
    exit(14);
  } else printf("success: player_getPurse() returns %d\n", player_getPurse(player));

  // add more gold
  goldToAdd = 29;
  if (!player_addPurse(player, goldToAdd)) {
    fprintf(stderr, "error: player_addPurse() valid gold amt purse nonempty\n");
    exit(15);
  } else printf("success: player_addPurse() added %d to nonempty purse\n", goldToAdd);

  // get gold amount, should be sum of two amts added
  if (!player_getPurse(player)) {
    fprintf(stderr, "error: player_getPurse() purse nonempty\n");
    exit(16);
  } else printf("success: player_getPurse() returns %d\n", player_getPurse(player));

  // get location, should be -1 since unchanged
  if (player_getLoc(player) != -1) {
    fprintf(stderr, "error: player_getLoc() unchanged location\n");
    exit(17);
  } else printf("success: player_getLoc() with unchanged location\n");

  // change location with valid input
  int loc = strlen(player_getGrid(player)) / 2;
  if (!player_setLoc(player, loc)) {
    fprintf(stderr, "error: player_setLoc() with valid location\n");
    exit(18);
  } else printf("success: player_setLoc() with valid location %d\n", loc);

  // should not be able to change location with negative input
  loc = -loc;
  if (player_setLoc(player, loc)) {
    fprintf(stderr, "error: player_setLoc() set negative location\n");
    exit(19);
  } else printf("success: player_setLoc() did not set negative location %d\n", loc);

  // should not be able to change location out of bounds
  loc = strlen(player_getGrid(player)) + 1;
  if (player_setLoc(player, loc)) {
    fprintf(stderr, "error: player_setLoc() set out of bounds location\n");
    exit(20);
  } else printf("success: player_setLoc() did not set out of bounds %d > %d\n", loc,
                (int) (strlen(player_getGrid(player)) - 1));

  // get location should be what we set
  if (player_getLoc(player) == -1) {
    fprintf(stderr, "error: player_getLoc() did not find set location\n");
    exit(21);
  } else printf("success: player_getLoc() returns %d\n", player_getLoc(player));

  // player status should be true as initialized
  if (!player_getStatus(player)) {
    fprintf(stderr, "error: player_getStatus() false before quit\n");
    exit(22);
  } else printf("success: player_getStatus() player in game\n");

  // player quits game
  if (!player_quitGame(player)) {
    fprintf(stderr, "error: player_quitGame() with player in game\n");
    exit(23);
  } else printf("success: player_quitGame() player in game\n");

  // player should not be able to quit if no longer in game
  if (player_quitGame(player)) {
    fprintf(stderr, "error: player_quitGame() double quit\n");
    exit(24);
  } else printf("success: player_quitGame() disallow double quit\n");

  // get status should be false after quit
  if (player_getStatus(player)) {
    fprintf(stderr, "error: player_getStatus() should be false after quit\n");
    exit(25);
  } else printf("success: player_getStatus() after quit\n");

  // print player's summary with valid id
  int id = 0;
  char* summary = player_summary(player, id);
  if (!strcmp(summary, "")) {
    fprintf(stderr, "error: player_summary() with valid id\n");
    exit(26);
  } else {
    printf("success: player_summary() with valid id, see below:\n");
    printf("%s", summary);
    free(summary);
  }
  
  // check player summary with invalid negative id
  id = -1;
  if (strcmp(player_summary(player, id), "")) {
    fprintf(stderr, "error: player_summary() allows negative id\n");
    exit(27);
  } else printf("success: player_summary() disallows negative id\n");

  // check player summary with out of bounds id
  id = 26;
  if (strcmp(player_summary(player, id), "")) {
    fprintf(stderr, "error: player_summary() allows out of bounds id\n");
    exit(28);
  } else printf("success: player_summary() disallows out of bounds id\n");
  
  // delete the player
  player_delete(player);
  printf("success: player_delete() deletes player\n");
    
  // now check player creation on var combos of addr/name/maxNameLength params

  // invalid address
  addr_t newAdd = message_noAddr();
  player_t* ghost = player_new(&newAdd, playerName, maxNameLength);
  if (ghost != NULL) {
    fprintf(stderr, "error: player_new() took invalid address\n");
    exit(29);
  } else printf("success: player_new() stops invalid address\n");

  // invalid maxNameLength
  maxNameLength = -1;
  ghost = player_new(&add, playerName, maxNameLength);
  if (ghost != NULL) {
    fprintf(stderr, "error: player_new() took invalid maxNameLength\n");
    exit(30);
  } else printf("success: player_new() stops invalid maxNameLength\n");
  
  // playerName > maxNameLength, name should truncate
  playerName = "dylan o'brien";
  maxNameLength = strlen(playerName) / 2;
  player_t* dylan = player_new(&add, playerName, maxNameLength);
  if (!strcmp(player_getName(dylan), playerName)) {
    fprintf(stderr, "error: player_new() does not truncate beyond maxNameLength\n");
    exit(31);
  } else printf("success: player_new() truncates %s to %s\n", playerName, player_getName(dylan));
  player_delete(dylan);
  
  // test getters/setters on NULL player, none should take NULL player

  if (player_getAddr(ghost) != NULL) {
    fprintf(stderr, "error: player_getAddr() took null player\n");
    exit(32);
  } else printf("success: player_getAddr() blocks null player\n");

  if (player_getName(ghost) != NULL) {
    fprintf(stderr, "error: player_getName() took null player\n");
    exit(33);
  } else printf("success: player_getName() blocks null player\n");

  if (player_getGrid(ghost) != NULL) {
    fprintf(stderr, "error: player_getGrid() took null player\n");
    exit(34);
  } else printf("success: player_getGrid() blocks null player\n");

  if (player_getPurse(ghost) != -1) {
    fprintf(stderr, "error: player_getPurse() took null player\n");
    exit(35);
  } else printf("success: player_getPurse() blocks null player\n");

  if (player_getLoc(ghost) != -1) {
    fprintf(stderr, "error: player_getLoc() took null player\n");
    exit(36);
  } else printf("success: player_getLoc() blocks null player\n");

  if (player_getStatus(ghost)) {
    fprintf(stderr, "error: player_getStatus() took null player\n");
    exit(37);
  } else printf("success: player_getStatus() blocks null player\n");

  // valid grid
  grid = "this is a valid grid";
  if (player_setGrid(ghost, grid)) {
    fprintf(stderr, "error: player_setGrid() took null player\n");
    exit(38);
  } else printf("success: player_setGrid() blocks null player\n");

  // valid gold amt
  goldToAdd = 12;
  if (player_addPurse(ghost, goldToAdd)) {
    fprintf(stderr, "error: player_addPurse() took null player\n");
    exit(39);
  } else printf("success: player_addPurse() blocks null player\n");

  // valid location for any grid
  loc = 0;
  if (player_setLoc(ghost, loc)) {
    fprintf(stderr, "error: player_setLoc() took null player\n");
    exit(40);
  } else printf("success: player_setLoc() blocks null player\n");

  if (player_quitGame(ghost)) {
    fprintf(stderr, "error: player_quitGame() took null player\n");
    exit(41);
  } else printf("success: player_quitGame() blocks null player\n");
  
  // no name, insert as spectator
  playerName = "";
  maxNameLength = 50;
  player_t* spectator = player_new(&add, playerName, maxNameLength);
  if (spectator == NULL) {
    fprintf(stderr, "error: player_new() does not allow empty name\n");
    exit(42);
  } else printf("success: player_new() created spectator\n");

  // test setter functions on spectator

  // spectator should be able to setGrid
  if (!player_setGrid(spectator, grid)) {
    fprintf(stderr, "error: player_setGrid() does not allow spectator grid\n");
    exit(43);
  } else printf("success: player_setGrid() allows spectator to set grid\n");
  
  // spectator should not be able to addPurse
  if (player_addPurse(spectator, goldToAdd)) {
    fprintf(stderr, "error: player_addPurse() allows spectator to add gold\n");
    exit(44);
  } else printf("success: player_addPurse() blocks spectator\n");

  // spectator should not be able to setLoc
  loc = strlen(grid) / 2;
  if (player_setLoc(spectator, loc)) {
    fprintf(stderr, "error: player_setLoc() allows spectator location\n");
    exit(45);
  } else printf("success: player_setLoc() blocks spectator\n");

  // spectator need not be able to change its status
  if (player_quitGame(spectator)) {
    fprintf(stderr, "error: player_quitGame() allows spectator to quit\n");
    exit(46);
  } else printf("success: player_quitGame() blocks spectator\n");

  // spectator summary should be empty string
  if (strcmp(player_summary(spectator, 0), "" )) {
    fprintf(stderr, "error: player_summary() should return empty string for spectator\n");
    exit(47);
  } else printf("success: player_summary() returns empty string for spectator\n");
  
  player_delete(spectator);
  printf("success: player_delete() deletes spectator\n");

  printf("clean!\n");
}


