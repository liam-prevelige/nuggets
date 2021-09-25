/*
 * grid.c - component of Nuggets, see grid.h for documentation
 *
 * Liam Prevelige, Lily Scott, Eliza Crocker May 2021
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "file.h"
#include "grid.h"
#include "player.h"

/**************** file-local global variables ****************/
// Constants for characters as given and named in implementation spec
static const char Rock = ' ';
static const char HorBdry = '-';
static const char VerBdry = '|';
static const char Corner = '+';
static const char RoomSpot = '.';
static const char Passage = '#';
static const char MyPlayer = '@';
static const char GoldSpot = '*';

static const float ErrorMargin = 0.0001; // error margin to account for equality due to rounding issues  
static const float OrigCharIndicator = '!';  // indicator character for replacing with character in original grid 

/************* global types ************/
typedef struct grid {
  int numRows;  // number of rows in 2D representation of grid string
  int numColumns;  // number of columns in 2D representation of grid string
  char* masterGrid;  // string with current map and game-state information (i.e. players, gold)
  char* originalGrid; // string with original map loaded from file (only rocks, walls/corners, hallways)
} grid_t;

/************** global functions ***********/

/*************** grid_initialize() *************/
/* see grid.h for description */
grid_t*
grid_initialize(char* mapFilePath, int goldMinNumPiles, int goldMaxNumPiles, int* numPilesPoint)
{
  // Read and store contents of map file to be used for a grid
  FILE* mapFile = fopen(mapFilePath, "r");
  if (mapFile == NULL) {
    return NULL;  // invalid map file path
  }
  int currNR = file_numLines(mapFile);  // num rows = number of lines in file

  char* firstLine = file_readLine(mapFile);
  int currNC = strlen(firstLine) + 1;   // num columns = number of characters, including newline, in first line

  char* currGrid = malloc(currNC * currNR + 1);
  char* restOfFile = file_readFile(mapFile);

  currGrid = strcpy(currGrid, firstLine); 
  free(firstLine);
  currGrid = strcat(currGrid, "\n");

  if (firstLine == NULL || restOfFile == NULL) {
    return NULL;  // invalid contents of map file path
  }

  currGrid = strcat(currGrid, restOfFile);  // concatanate all of file's contents to be used as the map
  free(restOfFile);


  // Allocate memory for grid and contained pointers
  grid_t* grid = malloc(sizeof(int) + sizeof(int) + sizeof(grid->masterGrid) + sizeof(grid->originalGrid));
  grid->masterGrid = malloc(strlen(currGrid) + 1);    // string with current game-state information
  grid->originalGrid = malloc(strlen(currGrid) + 1);  // string with original map file, no gold, players, etc.

  // Store all variables with completed information in grid pointers
  grid->numRows = currNR;
  grid->numColumns = currNC;
  strcpy(grid->originalGrid, currGrid);

  // Randomly generate number of piles given the parameter constraints, apply to master grid
  int numPiles = rand() % (goldMaxNumPiles - goldMinNumPiles + 1) + goldMinNumPiles;
  *numPilesPoint = numPiles;

  for (int i = 0; i < numPiles; i++) {
    // Add gold to curr grid for saving in masterGrid -- original shouldn't have gold allocated
    int totalIndices = currNC * currNR;
    int location = rand() % totalIndices;
    while (currGrid[location] != RoomSpot) {
      location = rand() % totalIndices;
    }
    currGrid[location] = GoldSpot;
  }

  strcpy(grid->masterGrid, currGrid);  // now master grid updated with gold, store in grid
  free(currGrid);
  fclose(mapFile);

  return grid;  // return pointer to completed grid
}

/*************** grid_updateMaster() *************/
/* see grid.h for description */
bool
grid_updateMaster(grid_t* grid, int idx, char addedChar) 
{
  if (grid == NULL || idx > (grid->numColumns * grid->numRows) || idx < 0) {
    return false; // invalid parameter(s)
  }
  if (addedChar == OrigCharIndicator) {
    grid->masterGrid[idx] = grid->originalGrid[idx];  // Swap character from master grid with one from original map file when asked using indicator
  }
  // Check all other possible characters according to specs -- if no overlap, return false
  if (addedChar != GoldSpot && addedChar != MyPlayer 
      && addedChar != Rock && addedChar != RoomSpot 
      && addedChar != Passage && !isalpha(addedChar)) {
    return false;
  }

  // Valid character and parameters provided, update character at index and return true for success
  grid->masterGrid[idx] = addedChar;
  return true;
}

/*************** grid_getVisible() *************/
/* see grid.h for description */
char*
grid_getVisible(grid_t* grid, int playerIdx)
{
  if (grid == NULL || playerIdx < 0 || playerIdx > grid->numColumns*grid->numRows) {
    return NULL;  // invalid parameter(s)
  }

  // Create and initialize new grid to contain player's visible grid
  char* visibleGrid = malloc(strlen(grid->masterGrid) + 1);
  if (visibleGrid != NULL) {
    for (int i = 0; i < strlen(grid->masterGrid); i++) {
      if (i % grid->numColumns == grid->numColumns - 1) {
        visibleGrid[i] = '\n';  // since grid strings contain newlines, add at the end of every row
      }
      else {
        visibleGrid[i] = Rock;  // otherwise initialize with whitespace, a rock
      }
    }
    // Iterate over all characters, using helper to determine visibility
    for (int i = 0; i < grid->numRows * grid->numColumns; i++) {
      if (grid_isVisiblePoint(grid, i, playerIdx)) {
        visibleGrid[i] = grid->masterGrid[i]; // if visible, update visible grid with character of master
      }
    } 
    visibleGrid[playerIdx] = MyPlayer;
    visibleGrid[strlen(grid->masterGrid)]= '\0';
    return visibleGrid;
  }

  fprintf(stderr, "grid_getVisible: error allocating memory for visible grid");
  return NULL;
}

/*************** grid_isVisiblePoint() *************/
/* see grid.h for description */
bool
grid_isVisiblePoint(grid_t* grid, int mapPointIdx, int playerIdx)
{
  // Check validity of grid and range of indices for each provided index, ensuring within grid limits
  if (grid == NULL || playerIdx < 0 || playerIdx > grid->numColumns*grid->numRows  || mapPointIdx < 0 || mapPointIdx > grid->numColumns*grid->numRows) {
    return false; // invalid parameters
  }

  int mc = (int)(mapPointIdx % grid->numColumns); // map column index in 2D representation
  int mr = (int)(mapPointIdx / grid->numColumns); // map row index in 2D representation
  int pc = (int)(playerIdx % grid->numColumns); // player column index in 2D representation
  int pr = (int)(playerIdx / grid->numColumns); // player row index in 2D representation

  // If current character is part of a passage, check if visible - only one extended character in passage is visible at a time
  if (grid->masterGrid[mapPointIdx] == Passage) {
    if (!grid_checkForVisiblePassage(grid, mapPointIdx, mr, pr, mc, pc)) {
      return false;
    }
  }

  // set default starting and ending (x,y) for moving through the grid to validate visibility 
  float startY = mr;
  float startX = mc;
  float endY = pr;
  float endX = pc;

  // flip starting and ending (x,y) if necessary  
  if (pr > mr) {
    startY = (float)pr;
    startX = (float)pc;
    endY = (float)mr;
    endX = (float)mc;
  }

  // Set the x and y step for moving character by character in grid string to determine visibility
  float xinc;  // Increment in x-direction (i.e. across a row)
  float yinc = 0.; // Increment in x-direction (i.e. across a column)
  if (startX - endX != 0.){
    float numerator = (float)abs(startY - endY);
    float denominator = (float)abs(startX - endX);  // no divide by zero error
    yinc = -1*(float)numerator/denominator;   // since starting y is always the lower coordinate, and row number is in ascending order, make yinc negative

    if (startX > endX) {
      xinc = -1.; // move left across a row
    }
    else {
      xinc = 1.;  // move right across a row
    }
  }
  else {
    // move up in a column
    xinc = 0.;
    yinc = -1.;
  }

  // Begin the process of moving from a starting coordinate to ending coordinate, checking intersected point(s) along the way
  // If intersection occurs through a wall's coordinate or between, the points will not be visible to one another
  float currY = startY;
  float priorY = startY;
  for (int currX = startX + xinc; (currX-endX) != 0 || (currY-endY) > ErrorMargin; currX += xinc) {   // continue incrementing through list until end point is reached
    currY += yinc;
    // Two points in the grid can be intersected in the vertical direction; see if the 'top' character is a barrier of some sort
    char topChar = grid->masterGrid[(((int)(currY))*grid->numColumns) + currX]; // row*number of items in row + column = current position - casting to int moves current coordinate UP
    if (topChar == HorBdry || topChar == Rock || topChar == Corner || topChar == VerBdry || topChar == Passage) {  // visibility-blocking character
      if ((int)currY != currY) {  // true when line connecting two points splits between coordinates on the grid
        char bottomChar = grid->masterGrid[(((int)(currY))*grid->numColumns) + currX + grid->numColumns];
        // In this case, check the 'lower' character in the grid - if it is also a barrier of some sort, gridpoints are not visible
        if (bottomChar == HorBdry || bottomChar == Rock || bottomChar == Corner || bottomChar == VerBdry || bottomChar == Passage) {  // visibility-blocking character
          if (!(currY-endY < ErrorMargin) || currX != endX) {   // if we haven't reached our destination (according to some small error margin)
            return false; // not visible
          }
        }
      }
      else {  // line directly intersects with a coordinate that's a barrier of some sort
        if (!(currY-endY < ErrorMargin) || currX != endX) {  // if we haven't reached our destination (according to some small error margin)
          return false; // not visible
        }
      }
    }
    
    // Line connecting two points to determine visibility can also fall between two gridpoints in the x-direction
    // If this is the case, visibility is interrupted if both the left and right coordinate is a barrier
    int rowDiff = (int)priorY - (int)currY;  // number of gridpoints intersected in this step (i.e. steep slope)
    // For every row intersected, check left & right gridpoint and determine if barrier
    for (int i = 1; i <= rowDiff; i++) {
      int rightIndex = (((int)currY)*grid->numColumns) + currX + grid->numColumns*i; 
      char rightChar = grid->masterGrid[rightIndex]; // character to the right of the split for the current step in the row intersected
      if (rightChar == HorBdry || rightChar == Corner || rightChar == VerBdry || rightChar == Rock || rightChar == Passage) {   // visibility-blocking character
        int leftIndex = (((int)currY)*grid->numColumns) + currX + grid->numColumns*i - xinc; 
        char leftChar = grid->masterGrid[leftIndex];  // character to the left of the split for the current step in the row intersected
        if (leftChar == HorBdry || leftChar == Rock || leftChar == Corner || leftChar == VerBdry || leftChar == Passage) {  // visibility-blocking character
          
          bool reachedYDest = ((int)(((((int)currY)*grid->numColumns) + grid->numColumns*i)/grid->numColumns) - endY < ErrorMargin); // has this iteration reached target y -- current iteration's row position - ending row < rounding error
          bool notStartChar = leftIndex != startX+startY*grid->numColumns && rightIndex != startX+startY*grid->numColumns;  // same index as where we started?
          if ((!reachedYDest || currX != endX) && notStartChar) { // if we haven't reached destination and have a new character, then the visibility is blocked
            return false; // not visible
          }
        }
      }
    }
    if (currY-endY < ErrorMargin && currX == endX) {   // have we reached destination, protecting from rounding errors
      return true;
    }
    priorY = currY;
  }
  return true;  // if nothing else returned, visibility isn't blocked
}

/*************** grid_checkForVisiblePassage() *************/
/* see grid.h for description */
bool
grid_checkForVisiblePassage(grid_t* grid, int mapPointIdx, int mr, int pr, int mc, int pc)
{
  if (grid->masterGrid[mapPointIdx] == Passage) {   // error check 
    if (abs(mr - pr) > 1 || abs(mc - pc) > 1) {  // only passages further than one character away are potentially invisible
      char* mGrid = grid->masterGrid;
      int numColumns = grid->numColumns;
      int numAdjacentPassage = 0;   
      // Check for passage characters up/down/left/right
      if (mGrid[mapPointIdx - 1] == Passage) {
        numAdjacentPassage += 1;
      }
      if (mGrid[mapPointIdx + 1] == Passage) {
        numAdjacentPassage += 1;
      }
      if (mGrid[mapPointIdx + numColumns] == Passage) {
        numAdjacentPassage += 1;
      }
      if (mGrid[mapPointIdx - numColumns] == Passage) {
        numAdjacentPassage += 1;
      }
      if (numAdjacentPassage > 1) { // any passage distanced from character is only visible if only one neighboring passage character
        return false;
      }
    }
  }
  return true;  // either visible hallway or wrong character entered, either way not blocked passage
}

/*************** grid_updatePlayerGrid() *************/
/* see grid.h for description */
char*
grid_updatePlayerGrid(grid_t* grid, player_t* player)
{
  if (grid == NULL || player == NULL) {
    return NULL;  // invalid parameter
  }
  // Get player's current known grid
  char* knownGrid = player_getGrid(player);
  char* newGrid = grid_getVisible(grid, player_getLoc(player)); // get visible grid based on new player coordinates
  // if either grid is null, at least return what's currently accessible
  if (knownGrid == NULL) {
    return newGrid;
  }
  if (newGrid == NULL) {
    return knownGrid;
  }
  for (int i = 0; i < grid->numRows * grid->numColumns; i++) {  // iterate over all characters, filling in gaps in visible grid with known
    if (newGrid[i] == Rock) {
      if (knownGrid[i] == MyPlayer || isalpha(knownGrid[i]) || knownGrid[i] == GoldSpot) {  // known grid converts to room spot
        knownGrid[i] = RoomSpot;

      }
      newGrid[i] = knownGrid[i];
    }
  }
  return newGrid;
}

/*************** grid_playerToGrid() *************/
/* see grid.h for description */
void
grid_playerToGrid(grid_t* grid, player_t* player, char letter){
  if (grid != NULL && player != NULL  && letter >= 'A') { // ignore invalid parameters
    int totalSpots = grid->numColumns * grid->numRows;
    int location = rand() % totalSpots; // assign player to a random spot of what's accessible
    while (grid->masterGrid[location] != RoomSpot) {  // only room spots count as possible starting locations
      location = rand() % totalSpots;
    }
    // get visible grid and update starting information
    grid_updateMaster(grid, location, letter);
    char* playerGrid = grid_getVisible(grid, location);
    player_setGrid(player, playerGrid);
    free(playerGrid);
    player_setLoc(player, location);
  }
}

/*************** grid_delete() *************/
/* see grid.h for description */
bool
grid_delete(grid_t* grid)
{
  if (grid == NULL) {
    return false; // invalid param
  }
  if (grid->masterGrid != NULL) {
    free(grid->masterGrid);
  }
  if (grid->originalGrid != NULL) {
    free(grid->originalGrid);
  }
  free(grid);
  return true;  // successful delete
}

/*************** grid_getNR() *************/
/* see grid.h for description */
int
grid_getNR(grid_t* grid)
{
  if (grid == NULL) {
    return -1;  // invalid param
  }
  return grid->numRows;
}

/*************** grid_getNC() *************/
/* see grid.h for description */
int
grid_getNC(grid_t* grid)
{
  if (grid == NULL) {
    return -1;  // invalid param
  }
  return grid->numColumns;
}

/*************** grid_getMasterGrid() *************/
/* see grid.h for description */
char*
grid_getMasterGrid(grid_t* grid)
{
  if (grid == NULL) {
    return NULL;  // invalid param
  }
  return grid->masterGrid;
}

/*************** grid_getOriginalGrid() *************/
/* see grid.h for description */
char*
grid_getOriginalGrid(grid_t* grid)
{
  if (grid == NULL) {
    return NULL; // invalid param
  }
  return grid->originalGrid;
}
