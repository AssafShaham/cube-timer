#ifndef GUI
#define GUI

#include "filehandling.h"
#include <time.h>

// Prints to the center of a given line
void printToLineCenter(const int y, const int color_pair_num, const char* str, ...);

// Prints to the center of the screen
void printToCenter(const int color_pair_num, const char* str, ...);

// Clears the contents of a given line
void clearLine(const int row);

// Performs the timer sequence, returning the solve time (handles +2's, DNF's)
float runTimer(const bool hide_timer_during_solve);

// Prints a time to the lefthand side
void printTime(const int session_id, const int index, const int solve_num, const int line_num, const bool print_above);

// Prints all times that there's space for to the lefthand side
void printAllTimes(const int session_id);

// Prints the current and best single, mo3,..., ao1000, and the session avg
void printAvgs(const int session_id);

#endif
