# CS50 Nuggets
## Implementation Spec

In this document, we detail the implementation-specific decisions we made in development of our final project.

## Data structures 

The main data structure used in our implementation is the `game`, of the global `game` type. The `game` is a global variable within the `server` module, which contains all information needed by the `server` to conduct gameplay. The `game` struct is defined as here:

```c
typedef struct game {
  player_t** playerArray;
  grid_t* grid;
  player_t* spectator;
  int nextPlayerNumber;
  int numPlayersQuit;
  int pilesRemaining;
  int nuggetsRemaining;
} game_t;
```

The `game` struct in turn holds the following variables and data structures:

* `playerArray`

`playerArray` is an array of `players` in the game. Each `player` is of the global `player` type implemented in the `player.c` module. The `player` struct is defined as here:

```c
typedef struct player {
  addr_t* address;
  char* name;
  char* grid;
  int purse;
  int location;
  bool status;
} player_t;
```

The `player` struct holds the following information about each `player`:

1. a pointer to its `address`, of global type `addr` implemented in the `message.c` module
2. its `name`, a string
3. a string representation of the `grid` displayed to the `player` at any given time
4. the amount of gold in its `purse`, a string
5. an index referring to the `player`'s `location` in the `grid` string
6. its game `status`, a boolean representing whether the `player` is still in the game

The `playerArray` starts empty with memory allocated in `game_new` to allow for up to `MaxPlayers` to be added later. `players` are added to the array within `game_newPlayer`. This function calls `player_new`, which initializes the variables within the `player` struct.


The `player`'s `grid`, `purse`, `location` are updated by the `game.c` and `grid.c` modules throughout gameplay, with calls to `player_setGrid`, `player_addPurse`, and `player_setLoc`, respectively. The `player`'s `status` is changed to `false` when the `player` quits the game, with a call to `player_quitGame`.

* `grid`

`grid` is of the global `grid` type implemented in the `grid.c` module. The `grid` struct is defined as here:

```c
typedef struct grid {
  int NR;
  int NC;
  char* masterGrid;
  char* originalGrid;
} grid_t;
```

The `grid` struct holds the following information about the `grid`:

1. its number of rows, `NR`, an int
2. its number of columns, `NC`, an int
3. a string representing the `masterGrid`, a map with the current locations of all players and gold piles
4. a string representing the `originalGrid`, a blank map of rooms and connected hallways

The `grid` is initialized in `game_new` with a call to `grid_initialize`, which randomizes the number and locations of all gold piles in the game and inserts them into the `grid`.

* `spectator`

The `spectator` is of the global `player` type implemented in the `player.c` module.


The `spectator` may or may not be null. A new `spectator` is added to the `game` with a call to `game_newSpectator`, with `grid` the `masterGrid` of the global `game` variable's `grid`. That is, the `spectator` knows and sees all gridpoints at all times. The `spectator` is different from a normal `player` in other ways as well: its `purse` remains empty; it does not have a `location`; its `status` is always true - when the `spectator` quits the game, it is forgotten by the `server` and removed from the `game` instead of setting its `status` to false.

* `nextPlayerNumber`, the next index in which to add a `player` to the `playerArray`
* `numPlayersQuit`, the number of `players` who have quit the game; if this reaches `MaxPlayers`, the game ends
* `pilesRemaining`, the number of gold piles remaining
* `nuggetsRemaining`, the number of gold nuggets remaining

Thus the global `game` variable holds all data structures used by the `server`.

## Control flow

The `server` controls the network and much of the message handling to and from the client. The `server` is implemented in `server.c`, with the following __four functions__:

### main

`main` parses the command line arguments and initializes other modules.

Pseudocode:

```
call parseArgs()
if successful,
   create new game, return nonzero on error
   call playGame(), return nonzero on error
   return 0
else return nonzero
```

### parseArgs

`parseArgs` parses and handles command line arguments, setting appropriate pointers.

Pseudocode:

```
check number of arguments (2 or 3)
   return nonzero on error
assign second argument to mapFile
if has a third argument 
   scan seed to int
   pass seed to srand
else pass getpid() to srand
return 0
```

### playGame

`playGame` takes a logfile and initializes the message module, begins receiving messages, and shuts down the module at game end.

Pseudocode:

```
initialize message module with given logfile
if port nonzero,
   start message loop without timeout feature, with handleClientMessage() as helpers
   close message module
   return 0
else return nonzero
```

### handleMessage

`handleMessage` takes the address of a client and a message received from the client, parses the message, and calls the corresponding function in the game module.

Pseudocode:

```
check that client address is valid, if not, log error and return false
allocate memory for address pointer
if successful, assign pointer to client address, otherwise log error and return false
if PLAY message
   log message received
   allocate memory for player's real name
   extract message content and copy to name
   call game_newPlayer, passing game, client address, name
   free name
else if SPECTATE message
   log message received
   call game_newSpectator, passing game, client address
else if KEY message
   log message received
   check for single keystroke, if malformatted, send error to client
   extract keystroke
   if keystroke is valid move character
      call game_playerMove, passing game, client address, keystroke
      if returned -1, game end
         free client address
         return true to end loop
   else if keystroke is valid quit character
      call game_clientQuit, passing game, client address
   else unknown keystroke
      send error to client
else malformatted message
   send error to client
free client address
return false to continue loop
```

## Other modules

### game

The `game` module takes on responsibility for the bulk of the gameplay, implementing the `game` struct and providing functions to start and end the game, add and remove players and spectators, and allow for player movement.


The __five primary functions__ implemented in the `game` module are as follows:

* `game_new`

Takes the pathname of a mapFile and initializes the `game`, returning a pointer to it.


Pseudocode:

```
if mapFile pathname NULL, log error and return NULL
otherwise,
   allocate memory for game pointer
   if successful
      allocate memory for playerArray allowing for MaxPlayers
      call grid module to initialize grid
      initialize other variables:
         nextPlayerNumber starts at 0, first index in array
         spectator starts NULL
         nuggetsRemaining starts at GoldTotal
      return game pointer
   else log memory allocation error and return NULL
 ```

* `game_newPlayer`

Takes a `game`, an `address` and a `realName` and initializes a `player` struct, then adding it into the game's `playerArray`, sending GRID, GOLD, and DISPLAY messages to the client upon success.


Pseudocode:

```
if game is NULL, log error and return false
check that player's address is valid, if not, log error and return false
if player's realName is NULL, send QUIT message to client and return false
else loop through characters in real name
   if character not blank, real name is valid
   else send QUIT message to client and return false
if MaxPlayers already in playerArray
   send QUIT message to client and return false
else initialize player
if unsuccessful, log error and return false
otherwise assign player letter to A plus nextPlayerNumber
insert player into grid 
insert player into playerArray at index nextPlayerNumber
send OK message to player with call to helper function, pass letter
send GRID message to player with call to helper function, pass grid dimensions
send GOLD message to player with call to helper function, pass 0 gold collected
send DISPLAY message to player with call to helper function, pass player's grid
update all players' grids with call to helper function
if unsuccessful, log error and return false
otherwise, increment nextPlayerNumber and return true
```

* `game_newSpectator`

Takes a `game` and an `address` and creates a `player` to be assigned as the `game`'s `spectator`, sending an existing `spectator` a QUIT message and sending the new `spectator` a GRID, GOLD, and DISPLAY message upon success.


Pseudocode:


```
if game NULL or address invalid, log error and return false
otherwise, if game already has spectator
   send QUIT message to spectator
   if unsuccessful, log error and return false
   otherwise create new player, passing empty string as player name
   if unsuccessful, log error and return false
   otherwise,
      allocate memory for spectator's grid
      if unsuccessful, log error and return false
      otherwise,
         copy masterGrid to spectator's grid
         send GRID message to spectator with call to helper function, pass grid dimensions
         send GOLD message to spectator with call to helper function, 0 gold in purse and 0 gold collected
         send DISPLAY message to spectator with call to helper function, pass masterGrid
         set spectator's grid, then free
         set game's spectator pointer to spectator and return true
```

* `game_playerMove`

Takes a players's address and a key representing a movement command, and does the work to update the player's position, adjust the visible map, and send the updated player grid to all players.

Pseudocode:

```
given an address, iterate over players and create a variable for the current player; if corresponding player doesn't exist, return false 
initialize variable for current character at destination
initialize variable for index of destination
given a command in the form of a character,

if lowercase of command != command
	initialize variable to track number of iterations
	while game_playerMove is true
		call game_playerMove with lowercase of command
		iterations += 1
	if iterations = 0
		no movement occurred; return false
	else
		movement occurred; return true

get index of destination

if the command is character for move left
	set current character to master grid's character at player's index - 1, set index of destination
else if command is character for move right
	set current character to master grid's character at player's index + 1, set index of destination
else if command is character for move up
	set current character to master grid's character at player's index + NC, set index of destination
else if command is character for move down
	set current character to master grid's character at player's index - NC, set index of destination
else if command is character for up and left
	set current character to master grid's character at player's index - NC - 1, set index of destination
else if command is character for up and right
	set current character to master grid's character at player's index - NC + 1, set index of destination
else if command is character for down and left
	set current character to master grid's character at player's index + NC - 1, set index of destination
else if command is character for down and right
	set current character to master grid's character at player's index + NC + 1, set index of destination
else
	invalid character; return -3

if 'A' <= current character <= 'A' + # players ever in game
	get player from index using letter in grid
	set other player coordinates to current player's current position in game's array of index coordinates
else if current character = '*'
	initialize integer for gold, set to random number between MIN_GOLD and MAX_GOLD/NUM_PILES
	remove gold from game purse goldRemaining
	decrease actualNumGoldPiles by one
	on error, send ERROR message to player
	otherwise
	send the player a GOLD message with amount of gold
	if actualNumGoldPiles = 0
		iterate over all players
			send the player GAME_OVER message
else if current character != '.'
	invalid move position; return false

set player coordinate to index of destination in game's array of index coordinates
iterate over all players, for each player do
	call `grid_updatePlayerGrid` for player, store output
	on error if output is null, send ERROR message to player, return false
	otherwise
	send DISPLAY message to player with output
return true
```

* `game_endGame` 

Takes a game pointer sends a GAME OVER message to all clients and frees all allocated memory.


Pseudocode:

```

if game not NULL
   allocate memory for the summary message
   concatenate a message header onto the game summary
   loop through players in the player array
      get a summary for each player
	  if successful
	     concatenate each player's summary onto the game summary
	  free the player's summary
    loop through players in the player array
	   if still in game
	      send player summary message
	   regardless delete player
	free playerArray
	delete grid
	if game has spectator
	   send summary message
	   delete spectator
	free summary
	free game

	on error, send ERROR message to stderr
```

The `game` module also implements __seven helper functions__:

* `playerQuit` 

Takes a game pointer and a player's address and does the work to remove them from the game, sending them a QUIT message.


Psuedocode:

```
if game not NULL
   if valid address
      loop through players in player array
            if a player matches given address
               if that player status is true
				  update the master grid to remove player
				  call player_quitGame
				  send a QUIT message to the player
				  increment numPlayersQuit
				  if numPlayerQuit is equal to MaxPlayers
				     call game_endGame
					   return -1
				  otherwise
				     updateAllPlayers
				on error, send ERROR message to stderr
				return 1
				otherwise
        if none of the players match the address,          
           send ERROR message to the stderr
		return 1
	if invalid address
	   send ERROR message to the stderr
	   return 1
	if game is NULL
	return 1
```

* `spectatorQuit` 

Takes a game pointer and does the work to remove a spectator sending them a QUIT message


Pseudocode:

```
if game not NULL
	if game spectator is non-NULL
	   get the address of the spectator
	   send QUIT message to the spectator
	   delete the spectator
	   set spectator pointer in game to NULL
	   return true
	otherwise 
	send ERROR message to stderr because spectator not in game so cannot quit
otherwise game is NULL
send ERROR message to stderr
return false
```

* `updateAllPlayers` 

Takes a game pointer, a player pointer, and a int goldChange and informs all players about the game update. 


Pseudocode:
```
if game is not NULL
	if movedPlayer is not NULL
		if spectator is not NULL
			send display message to spectator
			if gold changed
				send gold message to spectator
		loop through all players
			get an updated grid for player
			on error, send ERROR message to stderr
			return -1
			send display message to player
			set player's grid to new grid
			free new grid
			if gold changed
				if player is the the player that moved
					send gold message including the additions to purse
				otherwise
					send gold message with no update to purse
		if finish loop
		return 0
	if movedPlayer is NULL
		send ERROR message to stderr
		return 1
if game is NULL
	send ERROR message to stderr
	return 1

```

* `sendOK` 

Takes an address pointer and a letter and creates and sends OK message to player.


Pseudocode:
```
if parameters are valid
	allocate memory for message
	add OK and letter to message
	log to stderr
	send message to address
	free message
on error
	send error message to stderr
```

* `sendGrid` 

Takes an address pointer, int numRows and int numCols and creates and sends GRID message to player.


Pseudocode:
```
if parameters are valid
	allocate memory for message
	convert numRows to string
	convert numCols to string
    add GRID to message
    add numRows to message
    add space to message
    add numCols to message
    add space to message
    log to stderr
    send message to address
    free message
on error
   send error message to stderr
```

* `sendGold` 

Takes an address pointer, int justCollected, int updatedPurse, int nuggetsRemaining and creates and sends GOLD message to player/


Pseudocode:
```
if parameters are valid
   allocate memory for message
   convert justCollected to string
   convert updatedPurse to string
   convert nuggetsRemaining to string
   add GOLD to message
   add justCollected to message
   add space to message
   add updatedPurse to message
   add space to message
   add nuggetsRemaining to message
   log to stderr
   send message to address
   free message
on error
   send error message to stderr
```

* `sendDisplay` 

Takes an address pointer and a gridString


Pseudocode:

```
if parameters are valid
   allocate memory for message
   add DISPLAY to message
   add gridString to message
   log to stderr
   send message to address
   free message
on error
   send error message to stderr
```


### grid

We create a re-usable module `grid.c` to handle the initialization, formatting, and visibility constraints of the NR x NC grid of gridpoints the game is played on. We chose to write this as a separate module to streamline other pieces of the nuggets code and simplify the process of pivoting to new approaches for displaying the `grid`, if necessary. 

* `grid_initialize` 

Takes a path to a grid file and stores its contents, including number of rows and columns where a 2D representation is created with newline characters. It also takes a minimum and maximum count of gold, and a pointer to an integer which is set the number of piles once they're initialized in the master grid. 


Pseudocode:

```
open the given file path to the grid for reading; on error, return NULL
set NR equal to the number of lines in the file
read the first line and write contents to a string
if the first line is null, return null
set NC equal to the number of characters in the first line
read the rest of the file and write the rest of the grid contents to a string
initialize a new string for the starting grid, add the first line, add the rest of the file if the corresponding string isn’t null
create a new instance of *grid*, storing the string from the file as the original grid the game starts with, as well as the values for NR and NC  
randomly generate a number of piles between the min and max count.
for each of the created piles
	 randomly generate an index for the pile until a valid index (one where a room spot exists in master grid) is created
	 assign the corresponding character at the index in master grid to the character for gold
add the master grid with gold piles to the grid object  
close the file, clean up temporary variable copies  
return a pointer to the newly created *grid*
```

* `grid_updateMaster` 

Takes a grid object, index, and character, replacing the old character in the grid with the new character at the given index.


Pseudocode:

```
given a grid object, validate the grid isn’t null and the index is greater than 0 and less than the maximum index for the given grid; if error, return false  
given a character, validate the character is one of the game characters listed in the Requirements spec; if not valid, return false
replace the character in the master grid at the passed index with the passed character and return true
```

* `grid_getVisible` 

Takes a grid object and an index of a player's location, constructing a character array that holds the visible grid for the player.


Pseudocode:

```
given a grid object, validate the grid isn’t null; if null, return null
given an index that represents the player's location, validate it falls within the range of the map; on error, return null
create a new character array for the player’s new grid of size NR*NC
initialize every character in the array to the Rock character, unless the character is at the end of the line; if it is, initialize the character to newline char
iterate over every character in mastergrid, and for each:
	 if isVisiblePoint returns true with the master grid, current map index, and the current player’s index
		 add the gridpoint from the master grid at map index to the player’s grid
return player's grid
```

* `grid_isVisiblePoint` 

Takes a grid object, index of a player's location, index of an arbitrary gridpoint, and returns a boolean indicating whether it's visible to the player.


Pseudocode:

```
given a grid object, validate it isn't null; on error, return false
given a index of a gridpoint and an index of the current player position, validate they are both within the range of the grid; on error, return False
convert the map and player indices to x,y coordinates that represent their position in the columns X rows 2-dimensional grid
determine if the map character is a passage character; if it is, call a helper to determine if it's invalid for visibility, returning false if so
initialize a starting x, starting y, ending x, and ending y float that represent the (x,y) coordinate of the starting and ending character
set the starting character's (x,y) coordinates to the one that's lower down on the grid (i.e. has higher y-value/row index) and the ending character (x,y) to the other character's coordinates  
set an x and y increment for iterating from the starting character's coordinates to the ending character's coordinates - the y increment is the slope of the two points, but negative, since y-values are ascending. Set the x-increment to +1 or -1, depending on whether the target x-coordinate is to the left or right of the starting coordinates
starting at the starting coordinate, traverse over all characters in the line to the destination character, incrementing by x-increment, y-increment for each iteration
	 for each iteration, if the current character at or above the line to the ending character is a barrier (boundary/corner), do:
	 	 if the line splits two characters vertically, check the other character. If it's also a barrier and we haven't reached the end character, then visibility is blocked and return false
		 If the line doesn't split two characters vertically, then the barrier blocks visibility, if we haven't reached end character then return false
	 since the line can also split characters horizontally, determine whether the most recent step along the line intersected two horizontally adjacent characters (i.e. if the difference between the prior y-coordinate and the nearest character is less than the y-increment)
		if characters were split horizontally by the line, for each pair of split characters, do:
			If the right character is a barrier, check if the left character was a barrier.
				If it is and we haven't reached the end character, return false
	 if the character at the index in the grid = "#"
		if x_player - x_character > 1 or y_player - y_character > 1
			only the next '#' of a hallway can be visible at a time; return false
	 if the distance between the current location along the line and the destination is extremely small, return true
	 otherwise, update the prior y coordinate with the current y coordinate 
since no visibility was reported as blocked, return true
```

* `grid_checkForVisiblePassage` 

Takes a grid object, index of a map point in the character array, row and column of a player, and row and column of the map point, and returns a boolean stating whether it's a passage character that's visible to the player based on its position in the passageway alone (i.e. excluding obstacles).


Pseudocode:

```
if the character is a passage and the player's distance to the passageway is greater than 1, do:
	 get the master grid and initialize an integer to track the number of neighboring passage characters
	 iterate over top, bottom, left, right characters, incrementing the count for neighboring passage characters if applicable
	 if the adjacent passage count is greater than 1, meaning it's an edge passage character, return false
return true
```

* `grid_updatePlayerGrid` 

Takes a player object and gets its new visible grid, concatanating with the player's prior 'known' grid.


Pseudocode:

```
if given player object is null
	 return null
instantiate a string, set equal to player’s current known grid
instantiate a string for new grid of equal size to the known grid
set new grid equal to the output of `grid_getVisible`
if known grid is null
	 return new grid
if new grid is null
 	 return old grid
for int i = 0; i < number of characters in the master grid; i++
	 if the new grid at i is a rock, do:
		  if the known grid at i is the character for my player, gold, or another player, do:
			  set the known grid's character to a room spot
			set the new grid's character at i to the old grid's character at i
return the new grid
```

* `grid_playerToGrid` 

Takes a grid object, a player object, and a character for a new player, adding the character at random to the grid and updating grid/player as necessary.


Pseudocode:

```
if given grid or player object is null, or character isn't valid, leave
generate a random location between the minimum and maximum index in the grid string, continuing to generate the location until a room spot is found
update the master grid string with the player's character at the given location
set the player's grid to the visible grid at their destination location
```

* `grid_delete` 

Takes a grid object, returning true upon successful delete.


Pseudocode:

```
if given grid pointer is null, return false
if the grid's master grid pointer isn't null, free the master grid
if the grid's original grid pointer isn't null, free the original grid
free the grid struct
delete successful; return true
```

The `grid` module also implements the following __getter functions__:

* `grid_getNR` 

Takes a grid object, returning the number of rows in the contained master/original grid.


Pseudocode:

```
if given grid pointer is null, return -1
return the grid's pointer to number of rows
```

* `grid_getNR` 

Takes a grid object, returning the number of columns in the contained master/original grid.


Pseudocode:

```
if given grid pointer is null, return -1
return the grid's pointer to number of columns
```

* `grid_getMasterGrid` 

Takes a grid object, returning the pointer to the character array that contains the master grid.


Pseudocode:

```
if given grid pointer is null, return null
return the grid's pointer to string for the master grid
```

* `grid_getOriginalGrid` 

Takes a grid object, returning the pointer to the character array that contains the original grid.

Pseudocode:

```
if given grid pointer is null, return null
return the grid's pointer to string for the original grid
```

### player

We create a resuable `player` module to handle a `player` in the game. The `player` module implements the `player` struct and provides functions to create (and delete) a player, retrieve and update its information, and construct a summary of its game performance. 


These functions as implemented in `player.c` are delineated below:


The `player` module implements __six getter functions__.

* `player_getAddr` takes a `player` and returns its `address`, or `NULL` on error.
* `player_getName` takes a `player` and returns its `name`, or `NULL` on error.
* `player_getGrid` takes a `player` and returns its `grid`, or `NULL` on error.
* `player_getPurse` takes a `player` and returns its `purse`, or -1 on error.
* `player_getLoc` takes a `player` and returns its `location`, or -1 on error.
* `player_getStatus` takes a `player` and returns its `status`, or false on error.

The `player` module also implements __four setter functions__.

* `player_setGrid`

Takes a `player` and a `newGrid` string, setting the `player`'s `grid` to `newGrid`.

Pseudocode:

```
if player and newGrid nonNULL
   if player's grid nonNULL
      free the player's grid
   allocate memory for copy pointer
   if successful
      copy newGrid into copy pointer
      set player's grid to copy
      return true
   otherwise
      log memory allocation error and return false
otherwise return false
```

* `player_addPurse`

Takes a `player` and a positive `newGold` amount, adding `newGold` to the `player`'s `purse`. A `spectator` is not allowed to add gold to its `purse`, so we check that the `player`'s `name` is not the empty string.


Pseudocode:

```
if player nonNULL, player's name is not empty, newGold positive number
  add newGold to player's purse, return true
else return false
```

* `player_setLoc`

Takes a `player` and a `newLoc` index, setting the `player`'s `location` to `newLoc`. A `spectator` does not have a `location`, so we check that the `player`'s `name` is not the empty string.


Pseudocode:

```
if player nonNULL, player's name is not empty, newLoc valid index in player's grid
   set player's location to newLoc, return true
else return false
```

* `player_quitGame`

Takes a `player` and sets its `status` to false. A `spectator` is forgotten by the server upon quitting and thus need not ever change its `status`, so we check that the `player`'s `name` is not the empty string.


Pseudocode:

```
if player nonNULL, player's name is not empty, player's status true
   set player's status to false, return true
else return false
```

As part of our defensive programming strategy, we do not implement setter functions for a `player`'s `address` or `name`, as these should never be changed once initialized. As mentioned above, we prevent a `spectator` from making certain changes, and we additionally do not allow for the functionality to decrement a `player`'s `purse`, nor change its `status` to true.


The `player` module implements __three primary functions__.

* `player_new`

Takes an `address` and a player's `name`, as well as the `maxNameLength`, and creates a new instance of a `player` struct, initializing all variables and returning a pointer to the `player`.


Psuedocode:

```
validate parameters
   if address NULL or invalid, name NULL, or maxNameLength less than zero
      return NULL
allocate memory for the player
if unsuccessful, return NULL
otherwise, initialize all values
   allocate memory for player's address
   if successful, set address pointer, else return NULL
   normalize name with call to helper function
   if successful, set name pointer, else return NULL
   set grid to NULL
   set location to -1
   initialize purse to 0
   initialize status to true
return the pointer to the initialized player
```

* `player_summary`

Takes a `player` and an `id`, their index in the `game`'s `playerArray`, and returns a string summarizing their performance in the game. 


Psuedocode:

```
get the player's letter by adding id to 'A'
validate parameters
   if player NULL, player's name empty (i.e., spectator), or id out of bounds, return empty string
allocate memory for the summary, return empty string on error
construct the summary, including:
   the player's letter
   the gold in the player's purse
   the player's real name
return the summary
```

* `player_delete`

Frees all allocated memory corresponding to a given `player`.


Pseudocode:

```
if player nonNULL
   free player's name
   free player's address
   if player's grid nonNULL, i.e., memory allocated externally
      free player's grid
   free the player
```   

The `player` finally implements one static helper function.

* `normalizeName`

Takes a player's entered `name` and a `maxNameLength` and normalizes the name by coverting invalid characters to underscores and truncating names that are too long.


Pseudocode:

```
determines length of normalized name, either length of name or maxNameLength, whichever is less
allocate memory for normalized name, return NULL on error
loop through characters in name up to determined length
   if character is nongraphic and nonblank
      set character in normalized name to underscore
   else set character in normalized name to corresponding character in name
terminate normalized name with null terminator
return the normalized name
```

## Function prototypes

### server

```c
static int parseArgs(const int argc, char* argv[], char** mapFile);
static int playGame(FILE* logfile);
static bool handleClientMessage(void* arg, const addr_t from, const char* message);
```

### game

```c
game_t* game_new(char* mapName);
void game_newPlayer(game_t* game, addr_t* address, char* realName);
void game_newSpectator(game_t* game, addr_t* address);
bool game_playerMove(game_t* game, addr_t* address, char commandKey);
int game_clientQuit(game_t* game, addr_t* address);
void game_endGame(game_t* game);
static int playerQuit(game_t* game, addr_t* address);
static bool spectatorQuit(game_t* game);
static int updateAllPlayers(game_t* game, player_t* movedPlayer, int goldChange);
static void sendOK(addr_t* address, char letter);
static void sendGold(addr_t* address, int justCollected, int updatedPurse, int nuggetsRemaining);
static void sendGrid(addr_t* address, int numRows, int numCols);
static void sendDisplay(addr_t* address, char* gridString);
```

### grid

```c
grid_t* grid_initialize(char* mapFilePath, int goldMinNumPiles, int goldMaxNumPiles, int* numPiles);
bool grid_updateMaster(grid_t* grid, int idx, char addedChar);
char* grid_getVisible(grid_t* grid, int playerIdx);
bool grid_isVisiblePoint(grid_t* grid, int mapPointIdx, int playerIdx);
bool grid_checkForVisiblePassage(grid_t* grid, int mapPointIdx, int mr, int pr, int mc, int pc);
char* grid_updatePlayerGrid(grid_t* grid, player_t* player);
void grid_playerToGrid(grid_t* grid, player_t* player, char letter);
bool grid_delete(grid_t* grid);
int grid_getNR(grid_t* grid);
int grid_getNC(grid_t* grid);
char* grid_getMasterGrid(grid_t* grid);
char* grid_getOriginalGrid(grid_t* grid);
```

### player

```c
addr_t* player_getAddr(player_t* player);
char* player_getName(player_t* player);
char* player_getGrid(player_t* player);
int player_getPurse(player_t* player);
int player_getLoc(player_t* player);
bool player_getStatus(player_t* player);
bool player_setGrid(player_t* player, char* newGrid);
bool player_addPurse(player_t* player, int newGold);
bool player_setLoc(player_t* player, int newLoc);
bool player_quitGame(player_t* player);
player_t* player_new(addr_t* add, char* name);
char* player_summary(player_t* player, int id);
void player_delete(player_t* player);
static char* normalizeName(char* name, const int maxNameLength);
```

## Error handling and recovery

Our code uses extensive error checking and defensive programming methods to prevent the `server` from crashing. By verifying each of our parameters within each function and validating messages received from any clients, we ensure that our `server` continues to run even if it encounters erroneous situations. All code uses defensive-programming tactics to catch and log an error to stderr if passed invalid parameters or otherwise comes across an error. We redirect stderr to a logfile to review any such errors. All modules are set up with methods to notify of failure through their return values.


Should any unexpected message be received, we send an "ERROR" message to any relevant clients to warn them of the issue and allow them to respond accordingly. This will not cause any clients to disconnect or otherwise fail. 

## Testing plan

Here is an implementation-specific testing plan.  

### Unit testing

There are three modules - `game`, `player`, `grid` - each of which will have a corresponding testing driver - `gametest.c`, `playertest.c`, `gridtest.c`. The executables will test each function with various arguments - all combinations of invalid and valid inputs - while printing the resulting returned value. All functions have a return value that have some indiciation of invalid inputs when applicable.  


We test our `server` through extensive integration and system testing, as the bulk of the functionality occurs in the other modules and the server serves itself to integrate the other modules and the client.

### Regression testing

We conduct thorough testing with each update to our modules to ensure no previously addressed bugs reappear, through both rerunning of our unit tests and eventually our system tests.

### Integration/System testing
We will invoke the server for several games using the given client bot. Each game will include the joining and quitting of varying combinations of players and spectators. A different map, including one created by jell-0, will be used for a total of 10 runs with a different map. Note that erroneous arguments are already tested using our unit tests, and will not be repeated in this script. The server will be initialized with and without a seed over the different tests. For each setup of server, the following outlines a flow of players/spectators joining and quitting: 1 player joins->1 player quits->2 players join->1 player quits->1 spectator joins->1 spectator quits->1 spectator joins->another spectator joins, kicking out the original-> until max players is reached {2 players join, 1 player quits}. Three of the setups of the server, with differing maps and use of a seed, will be duplicated and run with valgrind to check for memory errors.  
