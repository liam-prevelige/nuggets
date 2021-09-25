/* 
 * gridtest.c - test program for grid
 * 
 * Lily Scott, Eliza Crocker, Liam Prevelige May 2021
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grid.h"
#include "player.h"

int 
main() 
{
  grid_t* grid = NULL;  // start grid off uninitialized
  int* numPiles = malloc(sizeof(int));  // pointer to receive number of piles

  printf("Try creating grid with invalid map path\n");
  grid = grid_initialize("../invalid/map/path.txt", 10, 30, numPiles);
  if (grid == NULL) {
    printf("Grid is null, invalid map path properly discovered.\n");
  }
  else {
    fprintf(stderr, "grid_initialize failed to detect null grid.\n");
    return 1;
  }

  printf("Create first grid\n");
  grid = grid_initialize("./maps/main.txt", 10, 30, numPiles);
  if (grid == NULL) {
    fprintf(stderr, "grid_initialize failed for grid.");
    return 1;
  }
  printf("Grid successfully created with %d piles.\n", *numPiles);
  free(numPiles);

  char* masterGridString = grid_getMasterGrid(grid);

  printf("\nMaster grid loaded with gold:\n");
  printf("%s\n", masterGridString);
  
  printf("\n\nOriginal grid saved without gold...\n");
  char* originalGrid = grid_getOriginalGrid(grid);
  printf("%s\n", originalGrid);


  printf("\nTest update master with NULL grid, valid index, and valid character...\n");
  grid_updateMaster(NULL, 1145, 'A');
  printf("Test update master with valid grid, invalid index, and valid character...\n");
  grid_updateMaster(grid, -1, 'A');
  printf("Test update master with valid grid, valid index, and invalid character...\n");
  grid_updateMaster(grid, 1145, '1');
  printf("Test update master with valid grid, valid index, and valid character...\n");
  grid_updateMaster(grid, 1145, 'A');
  printf("\nUpdated grid: expected to have one character change with the letter 'A':\n");
  printf("%s\n", grid_getMasterGrid(grid));

  printf("\nTest getVisible NULL grid, valid index...\n");
  grid_getVisible(NULL, 20);
  printf("\nTest getVisible valid grid, invalid index...\n");
  grid_getVisible(grid, -1);

  printf("\nTest getVisible invalid grid, valid index...\n");
  char* visibleGrid = grid_getVisible(NULL, 1143);
  printf("\nTest getVisible valid grid, invalid index...\n");
  visibleGrid = grid_getVisible(grid, -1);
  printf("\nTest getVisible valid grid, valid index...\n");
  visibleGrid = grid_getVisible(grid, 1145);

  printf("\nGot visible grid:\n");
  printf("%s\n", visibleGrid);
  free(visibleGrid);

  printf("Test isVisiblePoint with invisible point:\n");
  bool isVisibleTest1 = grid_isVisiblePoint(grid, 1157, 15);
  if (isVisibleTest1) {
    fprintf(stderr, "failed to determine point wasn't visible.\n");
    return 1;
  }
  printf("Correctly determined point isn't visible.\n");

  printf("Test isVisiblePoint with valid grid, visible point:\n");
  bool isVisibleTest2 = grid_isVisiblePoint(grid, 1157, 1145);
  if (!isVisibleTest2) {
    fprintf(stderr, "failed to determine point is visible.\n");
    return 1;
  }
  printf("Correctly determined point is visible.\n");


  printf("\nTest updatePlayerGrid with NULL grid, NULL player:\n");
  grid_updatePlayerGrid(NULL, NULL);
  printf("Test updatePlayerGrid with valid grid, NULL player:\n");
  grid_updatePlayerGrid(grid, NULL);

  printf("\nTest deleteGrid with NULL grid:\n");
  bool successfulDelete1 = grid_delete(NULL);
  if (successfulDelete1) {
    fprintf(stderr, "failed to detect null grid.\n");
    return 1;
  }
  printf("Successfully detected null grid.\n");

  printf("Test deleteGrid with valid grid:\n");
  bool successfulDelete2 = grid_delete(grid);
  if (!successfulDelete2) {
    fprintf(stderr, "failed to delete grid.\n");
    return 1;
  }
  printf("\nSuccessfully deleted grid.\n");

  int* secondNumPiles = malloc(sizeof(int));  // pointer to receive number of piles
  printf("\nTest grid initialization with different map file:\n");
  grid_t* secondGrid = grid_initialize("./maps/jello.txt", 10, 30, secondNumPiles);
  char* masterGrid = grid_getMasterGrid(secondGrid);
  printf("New master grid:\n%s\n", masterGrid);
  printf("\nTesting getters:\n");
  printf("Got number of rows: %d\n", grid_getNR(secondGrid));
  printf("Got number of columns: %d\n", grid_getNC(secondGrid));
  printf("Got original grid: \n%s\n", grid_getOriginalGrid(secondGrid));
  printf("Got master grid: \n%s\n", grid_getMasterGrid(secondGrid));

  printf("\nDeleting new grid.\n");
  free(secondNumPiles);
  grid_delete(secondGrid);

}
