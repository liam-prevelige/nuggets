# CS50 Nuggets Server
## Design Spec

### User interface

The nugget's first interface with the user is on the command-line; the user must give one argument and an optional second. The first argument is the pathname of a text file that holds the map. The second optional argument is a seed for the random-number generator.

```
server map.txt [seed]
```

After the user starts running the server, the server prints "START OF LOG" followed by a message "ready at port [PORT]."

As players connect to the server, the server logs the messages that it is sending and receiving from the player to stderr.

When the game is over the server will print a game-over summary. The game-over summary shall be the phrase "GAME OVER:" followed by a simple textual table with one row for every player (including any who quit early) and with three columns: player letter, player purse, and player name. (as stated in the Requirements).

### Inputs and outputs

**Input**: The server reads a map file from a pathname, given by the `mapPath` parameter (as described in the Requirements). An optional seed for the random-number generator can also be provided to change random behavior such that the random generations are repeatable. The seed is given by the `seed` parameter (as described in the Requirements).

As the game is played, the server will receive messages from the clients that must be parsed and interpreted to progress game play.

**Output**: The server logs messages it receives from clients to stderr. After interpreting these messages, it constructs its own messages to send back to the clients including DISPLAY updates, GOLD updates, GRID updates, QUIT messages, any errors and GAME SUMMARY when the game is over. The server logs these to stderr as well.

### Functional decomposition into modules

We anticipate the following functions in the `server` module:

1. _main_, which parses arguments and initializes other modules
2. _parseArgs_, which parses and handles command line arguments
3. _playGame_, which initializes the network, announces the port number and loops through messages
4. _handleClientMessage_, which parses the messages and calls the corresponding game functions

The `game` module provides the data structure to represent the game state, including its players, the game map, and the amount of gold in the game.

We anticipate the following functions in the `game` module:
1. _game_new_, which loads the map and places gold at random
2. _game_newPlayer_, which accepts a new player into the game
3. _game_newSpectator_, which accepts a new spectator into the game
4. _game_playerMove_, which updates the game state based on a player's actions
5. _game_clientQuit_, which removes a client from the game
6. _game_endGame_, which prepares a tabular game summary and sends a message to all clients

The `grid` module provides the data structure to represent a grid, and functions to update the grid based on the game state and what is visible to a player, interpreting the map text file.

We anticipate the following functions in the `grid` module:
1. _grid_initialize_, which takes pathname of a map file and min and max gold piles and drops a random number of gold piles into the grid and returns a grid pointer
2. _grid_updateMaster_, which changes the master grid based on the index and character given
3. _grid_getVisible_, returns grid of what can be "seen" from a given index
4. _grid_updatePlayerGrid_, takes a player and the grid a player can "see" and updates its "known" grid to be displayed
5. _grid_delete_, which frees each component of the grid

And some additional helper modules:

1. _player_, a module creating the player struct holding the player's display grid, address, gold count, status, location and accompanying functions to update.  
2. _message_, a provided module providing functions to send and interpret messages
3. _log_, a provided module providing the data structure to represent a log, and to write a log to a file


### Pseudocode for logic/algorithmic flow

The `server` will run as follows:

    parse the command line, validate parameters, initialize module *game*
    call playGame

where *playGame:*

     initializes fields of server address using global server constants,
     creates socket,
     connects the socket to the server,
     announces port number,
     and calls message_loop, passing handleClientMessage

where *handleClientMessage:*

     checks that the address was valid
     checks if message starts with PLAY
        create a new player
     checks if message starts with SPECTATE
        create new spectator
     checks if message starts with KEY
        if following key is a move key
            move player 
        if following key is quit
            change player status to false


The `game` module contains the following functions:


*game_new* which:

     creates new instance of a game struct
     allocates memory for playerArray
     reads the map file from map pathname string and initializes master grid
     
*game_newPlayer* which: 
     
     receives player address
     if numPlayers is less than maxPlayers
        creates a new instance of a player
        gets random location making sure that it's valid
        inserts the new player into the hashtable
        update the master grid with player
        construct player's grid with initial visibility
        sends the player a GRID message
        send the player a GOLD message
        send the player a DISPLAY message
        for each other active player
            check if new player is visible
                update player's grid and send it a DISPLAY message
        
*game_newSpectator* which:
     
     receive spectator address
     if there already is a spectator 
        send current spectator a QUIT message
        forget the current spectator
     initialize new spectator
     set spectator's grid as master
     insert new spectator into game struct
     send GRID message
     send DISPLAY message

*playerMove* which:
     
     receive and valid keystroke
     determine and validate the new location, if valid
        if new location contains pile of gold
            add gold to player's purse
            remove gold from master grid
            remove gold from counterset
            if no more gold remaining
                call endGame
        if another player at new location
            update master grid to swap players
        else
            update player's location in master grid
        update player's grid with new visibility and send new DISPLAY message
        for each other active player
            check if visible grid has changed
                update player's grid and send it a DISPLAY message

*game_clientQuit* which:

     determines if client is player or spectator
     if player
        changes the player's status
        removes the player from the master grid
        for each other active player
            check if visible grid has changed
                update player's grid and send it a DISPLAY message
     if spectator
        send spectator quit message
        remove spectator from game
        
*game_endGame* which:
     
     create summary header
     loops from the player hashtable
         add to summary each player's playerID, gold collected and player's name
     send GAME OVER message to all players


The `grid` module contains the following functions:

*grid_initializeGrid* which:
     
     open file from map pathname
     read in file to string
     calculate NR and NC
     intialize grid struct with given information
     calculate the random number of gold pile to be inserted from min and max parameters
     insert * in random . of master grid
    
*grid_updateMaster* which:

     replace character at given location of master grid with character passed

*grid_getVisible* which:
     
     validate given gridpoint
     interate over each row, column
     create line segment between every point
     if intersected 
        add character to visible grid
    return visible grid

*grid_updatePlayerGrid* which:
     
     validate player
        save old grid locally
        getNewVisibleGrid
        compare the old grid to the new grid
        if the characters conflicts
            handle conflicts according to Requirements Spec
        replace player's grid with new

*grid_delete* which:

     frees all memory associated with the grid

### Major data structures

The key data structure is the *game* which contains information about the overall game state.

It contains the following major data structures:

    1. an array of points to *player* structs 
    2. the spectator's *player* if there is a spectator 
    3. a master *grid*

A *player* is a data structure containing a string for playerID, a *grid* of the map that is displayed, a count of the gold in the player's purse, the player's current location, a boolean representing their playing status, and player's name and their address.

A *grid* contains the number of rows and number of column of the map as well as a string containing the masterGrid which includes gold piles and an original map for reference when returning piles and characters back to grid information.

### Testing Plan

Unit Testing: 


_player_: The functionality of player will be tested in a testing driver `playertest.c`
_grid_: The functionality of grid is will be tested in a testing driver `gridtest.c`
_game_: The functionality of game will be tested in a testing driver `gametest.c`
_server_: The server will be tested through integration and system testing.
 

Integration Testing: 

_server_: The server as a complete program will be tested with a bash script *testing.sh*. This will test specifically the functionality of game by sending messages to the server that will then make corresponding calls to the game module. We will test edge cases and any possible message that the server can receive from the client including various invalid messages. The script will include memory testing using valgrind.

As we test we will redirect stderr to a logfile to study the server's output and make use of the provided player to test our server.


### Division of work

Lily- leads implementation of server.c, game functions 1-3

Liam- leads implementation of grid module, game functions 4

Eliza- leads implemtation of player module, game functions 5-7
