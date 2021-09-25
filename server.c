/* 
 * server.c - component of Nuggests
 * 
 * Lily Scott, Eliza Crocker, Liam Prevelige May 2021
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <ctype.h>
 #include <stdbool.h>
 #include <unistd.h>
 #include "grid.h"
 #include "game.h"
 #include "message.h"

/**************** global variables *****************/
static game_t* game;

/**************** local functions ****************/
static int parseArgs(const int argc, char* argv[], char** mapFile);
static int playGame(FILE* logfile);
static bool handleClientMessage(void* arg, const addr_t from, const char* message);

/**************** main() ****************/
/* Parses command line arguments and initializes other modules.
 */
int
main(const int argc, char* argv[]) {
  
  char* mapFile = NULL;

  if (!parseArgs(argc, argv, &mapFile)) {       // parse arguments
    
    if ((game = game_new(mapFile)) == NULL) {   // initialize game
      fprintf(stderr, "error initializing game from mapfile\n");
      return 1;                                 // game initialization error
    }
    
    if (playGame(stderr) != 0) {                // begin gameplay
      fprintf(stderr, "error initializing the network\n");
      return 2;                                 // network error
    }
    return 0;                                   // success 
  }
  return 3;                                     // parseArgs error
 }

/**************** parseArgs() ****************/
/* Parses and handles command line arguments, setting appropriate pointers.
 *
 * Caller provides:
 *   number of args, argc; array of args, argv; pointer to mapFile
 *
 * We guarantee:
 *   if wrong number of args, return nonzero
 *   mapFile pointer set to second arg
 *   if third arg (seed) provided, passed to srand, 
 *    else pass getpid() as required
 *
 * We return:
 *   0 if successful, nonzero otherwise
 */
static int
parseArgs(const int argc, char* argv[], char** mapFile){

  if (argc != 2 && argc != 3) {
    fprintf(stderr, "invalid number of arguments");
    return 1;
  }

  // can assume mapfile is valid
  *mapFile = argv[1];

  if (argc == 3) {          // seed provided
    int seed;
    sscanf(argv[2], "%d", &seed);
    srand(seed);
  } else srand(getpid());   // no seed provided
  
  return 0;
}

/**************** playGame() ****************/
/* Initialize the message module, begin receiving messages, 
 * and shut down the module when game ends.
 * 
 * Caller provides:
 *   a logfile with which to initialize the message module
 * 
 * We guarantee:
 *   if initialization of message module fails, we return nonzero
 *   otherwise, we begin receiving messages from the client,
 *     shut down the module at game end, and return 0
 *   we do not use optional timeout feature of message_loop()
 * 
 * We return:
 *   nonzero if error initializign message module, 0 otherwise 
 */
static int
playGame(FILE* logfile) {

  // initialize message module
  int ourPort =  message_init(logfile);
  if (ourPort == 0) {
    return 1;                // failure to initialize message module
  }

  // begin receiving messages
  message_loop(NULL, 0, NULL, NULL, handleClientMessage);

  // shut down the module
  message_done();

  return 0;                  // success
}

/**************** handleClientMessage() ****************/
/* Parses messages from a client and calls the corresponding function in the game module.
 * 
 * Caller provides: 
 *   address from which message received, the message
 * 
 * We guarantee:
 *   if "PLAY" message, pass address to game_newPlayer()
 *   if "SPECTATE" message, pass address to game_newSpectator()
 *   if "KEY Q" message, pass address to game_clientQuit()
 *   if other valid "KEY" message, pass address and keystroke to game_playerMove()
 *   any other message, send "ERROR" message back to client
 */
static bool
handleClientMessage(void* arg, const addr_t from, const char* message)
{
  // invalid address
  if (!message_isAddr(from)) {
    fprintf(stderr, "error: message received from invalid address");
    return false;
  }

  // allocate memory for address and assign pointer 
  addr_t* other = malloc(sizeof(addr_t));
  if (other == NULL) {
    fprintf(stderr, "error: address memory allocation");
    return false;
  }
  *other = from;
 
  // handle PLAY message
  if (!strncmp(message, "PLAY ", strlen("PLAY "))) {

    // log message received
    fprintf(stderr, "handlePlay: \'%s\'\n", message);

    // extract message content, player's real name
    const char *content = message + strlen("PLAY ");  
    char* name = malloc(strlen(content) + 1);
    strcpy(name, content);

    // add new player to game with name at address
    game_newPlayer(game, other, name);
    
    free(name);
  }

  // handle SPECTATE message
  else if (!strcmp(message, "SPECTATE")) {

    // log message received
    fprintf(stderr, "handleSpectate: \'%s\'\n", message);

    // add new spectator to game at address
    if (!game_newSpectator(game, other)){
      fprintf(stderr, "error handling SPECTATE");
    }
  }

  // handle KEY message
  else if (!strncmp(message, "KEY ", strlen("KEY "))) {

    // log message received
    fprintf(stderr, "handleKey: \'%s\'\n", message);

    // extract message content, keystroke
    const char *content = message + strlen("KEY ");
    if (strlen(content) != 1) {  // malformatted message
      // send ERROR message to client
      char* error = "ERROR message format, expected \'KEY k\'";
      message_send(*other, error);
    } else {
      char key = content[0];
      
      // move keystroke
      if (key == 'h' || key == 'l' || key == 'j' || key == 'k' ||
          key == 'y' || key == 'u' || key == 'b' || key == 'n' ||
          key == 'H' || key == 'L' || key == 'J' || key == 'K' ||
          key == 'Y' || key == 'U' || key == 'B' || key == 'N') {

        // -1 returned on game end
        if (game_playerMove(game, other, key) == -1) {
            free(other);
            return true;
        }
      }
      
      // quit keystroke
      else if (key == 'Q') {
        if (game_clientQuit(game, other) == -1) {
          free(other);
          return true;
        }
      }

      // unknown keystroke
      else {
        // send ERROR message to client
        char* error = "ERROR usage, unknown keystroke";
        message_send(*other, error);
      }
    }   
  }

  // malformatted message
  else {
    // send ERROR message to client
    char* error = "ERROR unexpected message";
    message_send(*other, error);
  }
  
  // free address and return
  free(other);
  return false;
}
