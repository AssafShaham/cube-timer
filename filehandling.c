#include "filehandling.h"
#include "gui.h"
#include <stdio.h>
#include <ncurses.h>
#include <stdio.h>

const int TOP_STAT_LINE_CT = 8;

FILE* _getFilePtr(const int session_id, const char* mode) {
    char file_name[64];
    snprintf(file_name, sizeof(file_name), "session%d.txt", session_id);
    return fopen(file_name, mode);
}

bool testForFile(const int session_id) {
    // Get pointer to file
    FILE* fptr = _getFilePtr(session_id, "r");

    // If file is valid, return true
    if (fptr) {
        fclose(fptr);
        return true;
    }

    // Otherwise, create file and set it up
    char file_name[64];
    snprintf(file_name, sizeof(file_name), "session%d.txt", session_id);
    fptr = fopen(file_name, "a");
    if (fptr) {
        fputs("solves: 0\n", fptr);
        fputs("single: 99999\n", fptr);
        fputs("mo3: 99999\n", fptr);
        fputs("ao5: 99999\n", fptr);
        fputs("ao12: 99999\n", fptr);
        fputs("ao100: 999999\n", fptr);
        fputs("ao1000: 99999\n", fptr);
        fputs("savg: 99999\n", fptr);
        fputs("\n", fptr);
        fclose(fptr);
        return true;
    }

    // If file fails to be created, return false
    return false;
}

long getSolveCount(const int session_id) {
    // Get file pointer, give error if nullptr
    FILE* fptr = _getFilePtr(session_id, "r");
    if (!fptr) return -1;

    // Declare vars
    char buf[65536];
    int count = 0;

    // Primary for-loop, from Mike Simokin on Stack Overflow
    for (;;) {
        // Get a chunk of the file, ensure it's valid
        size_t res = fread(buf, 1, 65536, fptr);
        if (ferror(fptr)) return -1;

        // Count number of newline characters in chunk
        for (int i = 0; i < res; i++) {
            if (buf[i] == '\n') {
                count++;
            }
        }

        // If at end of file, end for-loop
        if (feof(fptr)) break;
    }

    fclose(fptr);
    return (count - TOP_STAT_LINE_CT);
}

bool writeNewTime(const int session_id, const float time) {
    // Get pointer to file
    FILE* fptr = _getFilePtr(session_id, "a");

    // If pointer is invalid, return false
    if (!fptr) return false;

    // Add solve entry to last line
    char entry[64];
    if (time != -1 && time != -2) snprintf(entry, sizeof(entry), "%.3f\n", time);
    else snprintf(entry, sizeof(entry), "DNF\n");
    fputs(entry, fptr);
    fclose(fptr);
    return true;
}

float getTime(const int session_id, const int i) {
    // Get pointer to file, handle errors
    FILE* fptr = _getFilePtr(session_id, "r");
    if (!fptr || i > getSolveCount(session_id)) return -1;

    // Iterate to i-th to last line in file
    fseek(fptr, 0, SEEK_END);
    long pos = ftell(fptr);
    int newline_ct = 0;
    while (pos > 0 && newline_ct < i+1) {
        pos--;
        fseek(fptr, pos, SEEK_SET);
        int val = fgetc(fptr);

        if (val == '\n') newline_ct++;
    }

    // Extract "word" after i-th newline character (the time)
    char val[64];
    fseek(fptr, pos + 1, SEEK_SET);
    fgets(val, sizeof(val), fptr);
    fclose(fptr);

    // If it's a DNF, return -2
    if (strcmp(val, "DNF\n") == 0) return -2;

    // Return the float value; if atof returns 0.0, there was an error
    float return_val = (float)atof(val);
    if (return_val == 0.0) return -1;
    else return return_val;
}

float getAvg(const int session_id, const int i) {
    // Error handling: if i > # of solves, return -1
    if (i > getSolveCount(session_id)) return -1;
    float sum = 0;
    int dnf_ct = 0;
    // In case of session average, get number of solves to iterate through
    if (i == 0) {
        long solve_ct = getSolveCount(session_id);
        for (int j = 1; j <= solve_ct; j++) {
            float val = getTime(session_id, j);
            // Case 1: error in getting time
            if (val == -1) return -1;
            // Case 2: not a DNF
            else if (val != -2) sum += val;
            // Case 3: DNF
            else dnf_ct++;
        }
        // For session average, skip over DNF solves entirely
        return sum / (solve_ct - dnf_ct);
    }
    else if (i == 1) {
        return getTime(session_id, getSolveCount(session_id));
    }
    else if (i == 3 || i == 5 || i == 12 || i == 50 || i == 100 || i == 1000) {
        float smallest = 999999;
        float largest = 0;
        for (int j = 1; j <= i; j++) {
            float val = getTime(session_id, j);
            // Case 1: error in getting time
            if (val == -1) return -1;
            // Case 2: DNF
            else if (val == -2) {
                dnf_ct++;
                if (dnf_ct > 1) return -2;
            }
            // Case 3: time is fine
            else sum += val;

            // Handle smallest/largest tracking if not mo3
            if (i != -3 && val > 0) {
                if (val < smallest) smallest = val;
                if (val > largest) largest = val;
            }

            // printToLineCenter(3, -1, "time: %.3f", val);
            // printToLineCenter(4, -1, "smallest: %.3f", smallest);
            // printToLineCenter(5, -1, "largest: %.3f", largest);
            // refresh();
            // delay_output(1000);
        }

        // If not mo3, subtract smallest, largest times from sum
        if (i != 3) {
            // If there's 1 DNF, only subtract smallest time
            if (dnf_ct == 1) sum -= smallest;
            else sum -= (smallest + largest);
            // Return case 1: isn't mo3 (no solves removed)
            return sum / (i-2);
        }
        // Return case 2: is mo3 (2 solves removed)
        else {
            // Case 1: No DNFs
            if (dnf_ct == 0) return sum / i;
            // Case 2: there's a DNF (mo3 becomes DNF)
            else return -2;
        }
    }
    else return -1;
}

// Moves cursor to space just before a time (for use in reaching best times)
// location variable gets moved to align with the new location
FILE* _moveToLineSpace(const int session_id, const int line, FILE* fptr, int* location) {
    int newline_ct = 0;
    *location = 0;
    while (newline_ct < line-1) {
        fseek(fptr, *location, SEEK_SET);
        if (fgetc(fptr) == '\n') newline_ct++;
        (*location)++;
    }

    // After finding the newline character, move forward until space
    while (fgetc(fptr) != ' ') {
        fseek(fptr, (*location), SEEK_SET);
        (*location)++;
    }

    return fptr;
}

float _getTimeFromLineEnd(const int session_id, const int line) {
    // Get pointer, handle errors
    FILE* fptr = _getFilePtr(session_id, "r");
    if (!fptr) return -1;

    // Move pointer to (i-1)-st newline character
    int location = 0;
    fptr = _moveToLineSpace(session_id, line, fptr, &location);

    // Read contents until newline
    char time_str[32];
    int idx = 0;
    while (fgetc(fptr) != '\n' && idx < 31) {
        fseek(fptr, location, SEEK_SET);
        time_str[idx] = fgetc(fptr);
        location++;
        idx++;
    }
    time_str[idx] = '\0';

    // Convert contents to float
    fclose(fptr);
    float return_val = (float)atof(time_str);
    if (return_val == 0.0) return -1;
    else return return_val;
}

int getLineNum(const int i) {
    if (i == 1) return 1;
    else if (i == 3) return 2;
    else if (i == 5) return 3;
    else if (i == 12) return 4;
    else if (i == 100) return 5;
    else if (i == 1000) return 6;
    else if (i == 0) return 7;
    else return -1;
}

float getBestTime(const int session_id, const int i) {
    int line_num = getLineNum( i);
    if (line_num != -1) return _getTimeFromLineEnd(session_id, line_num);
    else return -1;
}

bool setBestTime(const int session_id, const int i, const float solve) {
    // Get pointers, handle errors
    FILE* fptr = _getFilePtr(session_id, "r");
    FILE* temp = fopen("temp.txt", "w");
    if (!fptr || !temp) return false;

    // Identify best type
    char stat_name[64];
    if (i == 1) snprintf(stat_name, sizeof(stat_name), "single: ");
    else if (i == 3) snprintf(stat_name, sizeof(stat_name), "mo3: ");
    else if (i == 5) snprintf(stat_name, sizeof(stat_name), "ao5: ");
    else if (i == 12) snprintf(stat_name, sizeof(stat_name), "ao12: ");
    else if (i == 100) snprintf(stat_name, sizeof(stat_name), "ao100: ");
    else if (i == 1000) snprintf(stat_name, sizeof(stat_name), "ao1000: ");
    else if (i == 0) snprintf(stat_name, sizeof(stat_name), "savg: ");
    else snprintf(stat_name, sizeof(stat_name), "ERROR: ");

    // Copy all lines of the file except the one with the new best
    int line_idx = getLineNum( i);
    int current_line = 1;
    char line[64];
    while(fgets(line, sizeof(line), fptr)) {
        if (current_line == line_idx) fprintf(temp, "%s%.3f\n", stat_name, solve);
        else fputs(line, temp);
        current_line++;
    }

    // Delete the old file, rename the temp one
    fclose(fptr);
    fclose(temp);
    char file_name[64];
    snprintf(file_name, sizeof(file_name), "session%d.txt", session_id);
    remove(file_name);
    rename("temp.txt", file_name);
    return true;
}

int _lineNumToType(const int i) {
    if (i == 1) return 1;
    else if (i == 2) return 3;
    else if (i == 3) return 5;
    else if (i == 4) return 12;
    else if (i == 5) return 100;
    else if (i == 6) return 1000;
    else if (i == 7) return 0;
    else return -1;
}

void handleBestTimes(const int session_id) {
    for (int i = 1; i <= 7; i++) {
        float current_best = _getTimeFromLineEnd(session_id, i);
        float current_val = getAvg(session_id, _lineNumToType(i));
        if (current_val < current_best) setBestTime(session_id, _lineNumToType(i), current_val);
    }
    // // Check best single
    // float current_best = getBestTime(session_id, 1);
    // if (current_best > solve) setBestTime(session_id, 1, solve);
}
