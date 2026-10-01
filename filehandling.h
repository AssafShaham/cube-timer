#ifndef FILEHANDLING
#define FILEHANDLING

#include <stdio.h>
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>

// Ensure a file exists for writing current session data
bool testForFile(const int session_id);

// Returns the number of solves in a session
long getSolveCount(const int session_id);

// Write a new time to the session data
bool writeNewTime(const int session_id, const float time);

// Return i-th most recent time in session and its stats
float getTime(const int session_id, const int i);

// Returns the current average of n solves
float getAvg(const int session_id, const int i);

float _getTimeFromLineEnd(const int session_id, const int line);

int getLineNum(const int i);

// Returns the best-of-i solves time (i.e. 1 = best single, 5 = best ao5, etc)
float getBestTime(const int session_id, const int i);

// Writes to the file the best-of-i solves time (i.e. 1 = best single, 5 = best ao5, etc). Returns false if failed to write time to file
bool setBestTime(const int session_id, const int i, const float solve);

// Given a time, checks if it's a new best of any type and updates the file as needed
void handleBestTimes(const int session_id);

#endif
