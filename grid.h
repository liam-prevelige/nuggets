/*
 * grid.h - header file for grid module of Nuggets game
 * 
 * The grid module provides the data structure to represent 
 * a map as a grid, with functions to update the grid based on the 
 * game state and player interactions, interpreting a map 
 * text file.
 * 
 * Liam Prevelige, Lily Scott, Eliza Crocker May 2021
 */

#include <stdbool.h>
#include "player.h"

/**************** global types ****************/
typedef struct grid grid_t;


/**************** functions ****************/

/**************** grid_initialize ****************/
/* Create a new grid, loading the contents of a map file and distributing gold.
 *
 * Caller provides
 *   string for path to map file, min and max number of gold piles, and a pointer to be updated according to the number of gold piles allocated.
 * We return:
 *   pointer to a new grid, or NULL if error.
 * We guarantee:
 *   A NULL map file path is ignored
 *   Otherwise, the grid is initialized with the map file.
 *      Creates string for original grid loaded from file and separate master grid with gold distributed 
 *   Gold is randomly distributed given constraints and the pointer to the number of piles is updated
 * Caller is responsible for:
 *   providing valid map file contents, if readable
 *   later calling grid_delete.
 * Note
 *   Newlines are included in the string for the maps
 */
grid_t* grid_initialize(char* mapFilePath, int goldMinNumPiles, int goldMaxNumPiles, int* numPiles);

/**************** grid_updateMaster ****************/
/* Update the master grid with the given character at the given index in the grid string.
 *
 * Caller provides
 *   valid grid, index into grid string, and the character to be added.
 * We return:
 *   true if successful addition, or false if error.
 * We guarantee:
 *   A null grid or index less than 0 or greater number of indices in grid is ignored 
 *   If no error, character is inserted in the grid
 */
bool grid_updateMaster(grid_t* grid, int idx, char addedChar);

/**************** grid_getVisible ****************/
/* Get a visible map as a string given an index in the grid.
 *
 * Caller provides
 *   valid grid, index of player's coordinates in grid.
 * We return:
 *   true if successful creation of a visibility map, or null if error.
 * We guarantee:
 *   A null grid or index less than 0 or greater number of indices in grid is ignored 
 *   If no error, a string containing characters according to visibility in specs is returned
 */
char* grid_getVisible(grid_t* grid, int playerIdx);

/**************** grid_checkForVisiblePassage ****************/
/* Check if a given 2D location is a passage and visible from given 2D coordinates.
 *
 * Caller provides
 *   valid grid, index of map coordinate, map coordinate's row, map coordinate's column, player coordinate's row, player coordinate's column.
 * We return:
 *   true if character is visible passage, or false if error or not visible.
 * We guarantee:
 *   A null grid or non passageway character is ignored 
 *   True is returned if a passage character is visible from given coordinate, false otherwise
 */
bool grid_checkForVisiblePassage(grid_t* grid, int mapPointIdx, int mr, int pr, int mc, int pc);

/**************** grid_isVisiblePoint ****************/
/* Check if a given pair of coordinates can see one another in a grid.
 *
 * Caller provides
 *   valid grid, index of map coordinate, index of player coordinate in grid string.
 * We return:
 *   true if a pair of characters are visible to one another, or false if error or not visible.
 * We guarantee:
 *   A null grid or invalid index provided for either coordinate is ignored 
 *   True is returned if the given pair of coordinates can see one another, false otherwise
 */
bool grid_isVisiblePoint(grid_t* grid, int mapPointIdx, int playerIdx);

/**************** grid_updatePlayerGrid ****************/
/* Create a given player's grid by combining their current grid and their visible grid based on their current location.
 *
 * Caller provides
 *   valid grid, valid player.
 * We return:
 *   a string representing the character's combined 'known' and visible grid or null if error
 * We guarantee:
 *   A null grid or invalid index provided for either coordinate is ignored 
 *   True is returned if the given pair of coordinates can see one another, false otherwise
 */
char* grid_updatePlayerGrid(grid_t* grid, player_t* player);

/**************** grid_playerToGrid ****************/
/* Calculates a random location and if viable ('.') inserts player at that location.
 *
 * Caller provides
 *   valid grid, valid player, letter used to represent character in grid.
 * We return:
 *   a string representing the character's combined 'known' and visible grid or null if error
 * We guarantee:
 *   A null grid or invalid index provided for either coordinate is ignored 
 *   True is returned if the given pair of coordinates can see one another, false otherwise
 */
void grid_playerToGrid(grid_t* grid, player_t* player, char letter);

/**************** grid_delete ****************/
/* Deletes the whole grid.
 *
 * Caller provides
 *   valid grid pointer.
 * We guarantee:
 *   A null grid is ignored 
 *   Null pointer variables of grid are ignored
 * We return:
 *   True if successfully deleted, false on error
 */
bool grid_delete(grid_t* grid);

/**************** grid_getNR ****************/
/* Gets the number of rows in the grid.
 *
 * Caller provides
 *   valid grid pointer.
 * We guarantee:
 *   A null grid is ignored 
 * We return:
 *   An integer for the number of rows in the grid string
 */
int grid_getNR(grid_t* grid);

/**************** grid_getNC ****************/
/* Gets the number of columns in the grid.
 *
 * Caller provides
 *   valid grid pointer.
 * We guarantee:
 *   A null grid is ignored 
 * We return:
 *   An integer for the number of columns in the grid string
 */
int grid_getNC(grid_t* grid);

/**************** grid_getMasterGrid ****************/
/* Gets the master grid string which contains up-to-date information on players, gold, and overall gameplay.
 *
 * Caller provides
 *   valid grid pointer.
 * We guarantee:
 *   A null grid is ignored 
 * We return:
 *   A string with the full contents of the nuggets game
 */
char* grid_getMasterGrid(grid_t* grid);

/**************** grid_getOriginalGrid ****************/
/* Gets the original grid string which contains nothing aside from the original map structure.
 *
 * Caller provides
 *   valid grid pointer.
 * We guarantee:
 *   A null grid is ignored 
 * We return:
 *   A string with the original contents of the nuggets game's map
 */
char* grid_getOriginalGrid(grid_t* grid);
