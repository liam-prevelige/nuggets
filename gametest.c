/* 
 * gametest.c - test program for game
 * 
 * Lily Scott, Eliza Crocker, Liam Prevelige May 2021
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include "game.h"
 #include "message.h"
 #include "log.h"
static addr_t* newAddress();

int
main(const int argc, char* argv[])
{
  
  if (argc != 2){
    fprintf(stderr, "should have one argument: mapName\n");
    return 1;
  }
  message_init(stderr);
  char* mapFile = argv[1];
  game_t* game;
  if ((game = game_new(mapFile)) == NULL){
      fprintf(stderr, "error initializing game from mapfile\n");
      return 2;
  }

  fprintf(stderr, "\n\ntesting game_newPlayer\n\n");
  addr_t* address1 = newAddress("45678");
  if (game_newPlayer(game, address1, "Beyonce")){
    fprintf(stderr, "successful\n");
  } else {
    fprintf(stderr, "unsuccessful\n");
  }
  fprintf(stderr, "game_newPlayer: should be successful\n");
  addr_t* address2 = newAddress("35671");
  if (game_newPlayer(game, address2, "Bree")){
    fprintf(stderr, "successful\n");
  } else {
    fprintf(stderr, "unsuccessful\n");
  }
  fprintf(stderr, "game_newPlayer: should be successful\n");
  addr_t* address6 = newAddress("15632");
  if (game_newPlayer(game, address6, "Tim")){
    fprintf(stderr, "successful\n");
  } else {
    fprintf(stderr, "unsuccessful\n");
  }
  fprintf(stderr, "game_newPlayer: should be successful\n");
  addr_t* address7 = newAddress("09876");
  if (game_newPlayer(game, address7, "Halle")){
    fprintf(stderr, "successful\n");
  } else {
    fprintf(stderr, "unsuccessful\n");
  }
  fprintf(stderr, "game_newPlayer: should be successful\n");
  addr_t addressBad = message_noAddr();
  if (game_newPlayer(game, &addressBad, "Brad")){
    fprintf(stderr, "successful\n");
  } else {
    fprintf(stderr, "unsuccessful\n");
  }
  fprintf(stderr, "game_newPlayer: should be unsuccessful because address is not good\n");
  addr_t* address8 = newAddress("29456");
  if (game_newPlayer(game, address8, "")){
    fprintf(stderr, "successful\n");
  } else {
    fprintf(stderr, "unsuccessful\n");
  }
  fprintf(stderr, "game_newPlayer should be unsuccessful because name is empty string\n");
  if(game_newPlayer(game, address8, "         ")){
    fprintf(stderr, "successful\n");
  } else {
    fprintf(stderr, "unsuccessful\n");
  }
  fprintf(stderr, "game_newPlayer: should be unsuccessful because name is only whitespace\n");


  fprintf(stderr, "\n\ntesting game_newSpectator\n\n");
  addr_t* address3 = newAddress("51234");
  if (game_newSpectator(game, address3)){
    fprintf(stderr, "successful\n");
  } else {
    fprintf(stderr, "unsuccessful\n");
  }
  fprintf(stderr, "game_newSpectator: should be successful\n");
  addr_t* address4 = newAddress("23934");
  if (game_newSpectator(game, address4)) {
  fprintf(stderr, "successful\n");
  } else {
    fprintf(stderr, "unsuccessful\n");
  }
  fprintf(stderr, "game_newSpectator: should be successful and send quit message to previous spectator address\n");
  if (game_newSpectator(game, &addressBad)) {
    fprintf(stderr, "successful\n");
  } else {
    fprintf(stderr, "unsuccessful\n");
  }
  fprintf(stderr, "game_newSpectator: should be unsuccessful because address is not good\n");

  fprintf(stderr, "\n\ntesting game_playerMove\n\n");
  if (game_playerMove(game, address1, 'k') == 0){
    fprintf(stderr, "successful\n");
  } else {
    fprintf(stderr, "unsuccessful\n");
  }
  fprintf(stderr, "game_playerMove: should be successful\n");
  if (game_playerMove(game, &addressBad, 'k') == 0) {
    fprintf(stderr, "successful\n");
  } else {
    fprintf(stderr, "unsuccessful\n");
  }
  fprintf(stderr, "game_playerMove: should be unsuccessful because invalid address\n");
  if (game_playerMove(game, address2, 'a') == 0) {
    fprintf(stderr, "successful\n");
  } else {
    fprintf(stderr, "unsuccessful\n");
  }
  fprintf(stderr, "game_playerMove: should be unsuccessful because invalid key\n");


  fprintf(stderr, "\n\ntesting game_clientQuit\n\n");
  if (game_clientQuit(game, address7) == 0){
    fprintf(stderr, "successful\n");
  } else {
    fprintf(stderr, "unsuccessful\n");
  }
  fprintf(stderr, "\n\ntesting game_clientQuit\n\n");
  if (game_clientQuit(game, address4) == 0){
    fprintf(stderr, "successful\n");
  } else {
    fprintf(stderr, "unsuccessful\n");
  }
  fprintf(stderr, "game_clientQuit: should be successful\n");
  if (game_clientQuit(game, &addressBad) == 0){
    fprintf(stderr, "successful\n");
  } else {
    fprintf(stderr, "unsuccessful\n");
  }
  fprintf(stderr, "game_clientQuit: should be unsuccessful because invalid address\n");

  fprintf(stderr, "\n\ntesting game_endGame\n\n");
  game_endGame(game);
  fprintf(stderr, "game_endGame: should be successful\n");


  fprintf(stderr, "\n\ntesting all functions with bad game\n\n");
  addr_t* address5 = newAddress("33457");
  game_newPlayer(NULL, address5, "Dylan");
  fprintf(stderr, "game_newPlayer: should be unsuccessful because game is bad\n");
  game_newSpectator(NULL, address5);
  fprintf(stderr, "game_newSpectator: should be unsuccessful because game is bad\n");
  game_playerMove(NULL, address5, 'h');
  fprintf(stderr, "game_playerMove: should be unsuccessful because game is bad\n");
  game_endGame(NULL);
  fprintf(stderr, "game_endGame: should be unsuccessful because game is bad\n");
  free(address1);
  free(address2);
  free(address3);
  free(address4);
  free(address5);
  free(address6);
  free(address7);
  free(address8);
}

static addr_t* newAddress(char* addr){
  addr_t* address = malloc(sizeof(addr_t));
  if (!message_setAddr("localhost", addr, address)){
    fprintf(stderr, "error setting address\n");
    return NULL;
  }
  return address;
}