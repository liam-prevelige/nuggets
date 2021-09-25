# CS50 Nuggets
## Testing Spec

In this document, we detail our approach to testing our implementation of the _Nuggets_ game, specifically the procedures used for unit testing of each module and program, regression testing throughout development, and integration testing of our modules through a system test of our server.

## Unit Testing

We implemented three modules - `game`, `grid`, and `player` - each of which has a corresponding test driver - `gametest.c`, `gridtest.c`, and `playertest.c`. The executables test each function of its respective module with various arguments, all combinations of valid and invalid inputs, and printing success or error messages to stdout or stderr accordingly, based on expected return values of each function based on input. The outputs of each of these executables can be found at its respective output file - `gametest.out`, `gridtest.out`, or `playertest.out`. Additionally, we made use of valgrind when running each of our unit tests to ensure no memory leaks or errors were found. 


Our `server` itself handles only message handling and does not implement any of the game's functionality. We made use of the provided `miniclient` to test any invalid messages potentially received by the `server` (such as invalid keystrokes and other malformatted messages as described by the Requirements Spec). All other functionality of the `server` was tested heavily through integration and system testing, detailed below.

## Regression Testing

Throughout development of our final project, we continued to run our unit tests (and, towards the final stages of development, our system tests) as changes were made to each of our various modules and programs. We additionally built out each unit test to test any new functionality introduced. Through repeated testing of each module, we were able to confirm that any previously addressed bugs had not been reintroduced with changes made. We made thorough use of valgrind throughout all testing to ensure no memory leaks or errors were found.

## Integration and System Testing

In the beginning stages of our integration and system testing, we invoked our `server` several times using the provided `player` program to test integration with a client in the following stages:

* supports one player joining, remaining in game until all gold collected
* supports one player joining with a spectator
* supports multiple players joining, remaining in game until all gold collected
* supports multiple players joining with a spectator
* supports multiple players joining with a spectator, some quitting
* supports a spectator joining, then being overridden by a new spectator
* supports MaxPlayers joining, then new player attempting to join
* supports MaxPlayers joining then quitting to trigger end of game

Once the functionality of the `server` was tested as above, we began invoking the `server` using multiple various maps, with and without a seed, repeating tests with each seed to ensure consistency of randomly generated elements between runs. We additionally began invoking the `player` from multiple systems, and once relatively confident in our implementation, we finally used the provided `player`'s bot mode to run several tests over various maps and seeds with different combinations of players and spectators joining and quitting at various points. 


We again made use of valgrind throughout integration and system testing to ensure no memory leaks or errors were found.
