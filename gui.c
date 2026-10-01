#include "gui.h"
#include "filehandling.h"
#include <ncurses.h>

int _getDigitCount(float num) {
    int digit_ct = 0;
    char buf[32];
    snprintf(buf, sizeof(buf), "%.3f", num);
    for (int i = 0; buf[i] != '\0'; i++) {
        if (buf[i] >= '0' && buf[i] <= '9') {
            digit_ct++;
        }
    }

    return digit_ct;
}

void _vPrintToCenter(const int y, const int color_pair_num, const char* str, va_list args) {
    char buf[128];
    vsnprintf(buf, sizeof(buf), str, args);

    int max_x = getmaxx(stdscr);
    int x = (max_x - strlen(buf)) / 2;

    if (color_pair_num > 0) {
        attron(COLOR_PAIR(color_pair_num));
        mvprintw(y, x, "%s", buf);
        attroff(COLOR_PAIR(color_pair_num));
    }
    else {
        mvprintw(y, x, "%s", buf);
    }
}

void printToLineCenter(const int y, const int color_pair_num, const char* str, ...) {
    char buf[128];
    va_list args;
    va_start(args, str);
    _vPrintToCenter(y, color_pair_num, str, args);
    va_end(args);
}

void printToCenter(const int color_pair_num, const char* str, ...) {
    int y = getmaxy(stdscr) / 2;

    char buf[128];
    va_list args;
    va_start(args, str);
    _vPrintToCenter(y, color_pair_num, str, args);
    va_end(args);
}

void clearLine(const int row) {
    move(row, 0);
    clrtoeol();
    refresh();
}

float runTimer(bool hide_timer_during_solve) {
    timeout(50);
    bool is_timer_running = false;
    bool is_inspection_running = true;
    bool is_plus_two = false;
    bool is_dnf = false;
    long timer_ms = 0;
    int inspection_timer = 15;
    struct timespec start, current;
    clock_gettime(CLOCK_MONOTONIC, &start);

    while (is_inspection_running) {
        // Update inspection timer
        clock_gettime(CLOCK_MONOTONIC, &current);
        inspection_timer = 15 - (current.tv_sec - start.tv_sec);

        // Print inspection time
        clear();
        if (inspection_timer > 0) {
            // Give 8 and 12 second warnings via color changes
            if (inspection_timer <= 3) printToCenter(2, "%d", inspection_timer);
            else if (inspection_timer <= 7) printToCenter(1, "%d", inspection_timer);
            else printToCenter(-1, "%d", inspection_timer);
        }
        else if (inspection_timer < -2) {
            is_dnf = true;
            printToCenter(1, "DNF");
        }
        else {
            is_plus_two = true;
            printToCenter(3, "+2");
        }
        refresh();

        // When spacebar pressed, update bools
        char ch = getch();
        if (ch == ' ') {
            is_inspection_running = false;
            is_timer_running = true;
        }
    }

    // If timer is hidden, simply show text saying "solve" while timer runs
    if (hide_timer_during_solve) {
        clear();
        printToCenter(-1, "Solve");
        refresh();
    }

    clock_gettime(CLOCK_MONOTONIC, &start);
    while (is_timer_running) {
        // Update timer time
        clock_gettime(CLOCK_MONOTONIC, &current);
        timer_ms = ((current.tv_sec - start.tv_sec) * 1000) + ((current.tv_nsec - start.tv_nsec) / 1000000);

        // Print timer time if preference is set as such
        if (!hide_timer_during_solve) {
            clear();
            printToCenter(-1, "%ld.%03ld", timer_ms / 1000, timer_ms % 1000);
            refresh();
        }

        // When spacebar pressed, update variables
        char ch = getch();
        if (ch == ' ') is_timer_running = false;
    }

    // Handle +2, DNF cases
    if (is_dnf) timer_ms = -2;
    else if (is_plus_two) timer_ms += 2000;

    return timer_ms;
}

void _printNTimes(const int y, const int x, const int n, const char c) {
    for (int i = 0; i < n; i++) {
        mvprintw(y, x + i, "%c", c);
    }
}

void printTime(const int session_id, const int index, const int solve_num, const int line_num, const bool print_above) {
    // Get time
    float time = getTime(session_id, index);

    // Get number of digits (for centering)
    int time_digit_ct = _getDigitCount(time);
    int solve_num_digit_ct = _getDigitCount(solve_num);

    // Prepare spacing for line
    char line[64] = "|";
    for (int i = 0; i < solve_num_digit_ct + 2; i++) line[i+1] = ' ';
    line[solve_num_digit_ct + 3] = '|';
    for (int i = 0; i < 11; i++) line[solve_num_digit_ct + i + 4] = ' ';
    line[solve_num_digit_ct + 15] = '|';

    // Print line table formatting stuff
    mvprintw(line_num, 0, "%s", line);
    _printNTimes(line_num + 1, 0, solve_num_digit_ct + 16, '-');
    if (print_above) _printNTimes(line_num - 1, 0, solve_num_digit_ct + 16, '-');

    // Print the solve number
    mvprintw(line_num, 3, "%d", solve_num);

    // Printing the time
    if (time == -2) mvprintw(line_num, solve_num_digit_ct + 8, "DNF");
    else mvprintw(line_num, solve_num_digit_ct + 4 + (11 - time_digit_ct) / 2, "%.3f", time);
}

void printAllTimes(const int session_id) {
    // Print info line
    mvprintw(1, 0, "-----------------");
    mvprintw(2, 0, "| Number | Time |");
    mvprintw(3, 0, "-----------------");
    // Identify how many times can be printed
    int line_ct = getmaxy(stdscr)/2;
    long solve_ct = getSolveCount(session_id) + 1;

    // Print up to that many times
    printTime(session_id, 1, solve_ct - 1, 6, true);
    for (int i = 2; i < line_ct; i++) {
        // Check if there is a time at that line; if not, end loop
        if (getTime(session_id, i) == -1) break;

        // If there is, print it
        printTime(session_id, i, solve_ct - i, i * 2 + 4, false);
    }
}

void printAvg(const int session_id, const int i, const int line_num) {
    // Get times, digit counts of times
    float avg = getAvg(session_id, i);
    float best_avg = getBestTime(session_id, i);
    int avg_digit_ct;
    if (avg == -1 || avg == -2) avg_digit_ct = 3;
    else avg_digit_ct = _getDigitCount(avg) + 1;
    int best_digit_ct;
    if (best_avg == -1 || best_avg == -2) best_digit_ct = 3;
    else best_digit_ct = _getDigitCount(best_avg) + 1;

    // Print line table formatting stuff
    char line[64] = "|";
    for (int i = 0; i < 11; i++) line[i+1] = ' ';
    line[12] = '|';
    for (int i = 0; i < 11; i++) line[i+13] = ' ';
    line[24] = '|';
    mvprintw(line_num, COLS - 25, "%s", line);
    mvprintw(line_num + 1, COLS - 35, "-----------------------------------");

    // Print label
    if (i == 1) mvprintw(line_num, COLS - 35, "| Single ");
    else if (i == 3) mvprintw(line_num, COLS - 35, "|   mo3   ");
    else if (i == 5) mvprintw(line_num, COLS - 35, "|   ao5   ");
    else if (i == 12) mvprintw(line_num, COLS - 35, "|  ao12   ");
    else if (i == 100) mvprintw(line_num, COLS - 35, "|  ao100  ");
    else if (i == 1000) mvprintw(line_num, COLS - 35, "| ao1000  ");
    else if (i == 0) mvprintw(line_num, COLS - 35, "| Session ");

    // Print best time
    if (best_avg == -2) mvprintw(line_num, COLS - 20, "DNF");
    else if (best_avg == -1) mvprintw(line_num, COLS - 20, "N/A");
    else mvprintw(line_num, COLS - 19 - (best_digit_ct / 2), "%.3f", best_avg);

    // Print current time
    if (avg == -2) mvprintw(line_num, COLS - 8, "DNF");
    else if (avg == -1) mvprintw(line_num, COLS - 8, "N/A");
    else mvprintw(line_num, COLS - 7 - (avg_digit_ct / 2), "%.3f", avg);
}

void printAvgs(const int session_id) {
    // Print info line
    mvprintw(1, COLS - 35, "-----------------------------------");
    mvprintw(2, COLS - 35, "|  Type   |   Best    |  Current  |");
    mvprintw(3, COLS - 35, "-----------------------------------");

    printAvg(session_id, 1, 4);
    printAvg(session_id, 3, 6);
    printAvg(session_id, 5, 8);
    printAvg(session_id, 12, 10);
    printAvg(session_id, 100, 12);
    printAvg(session_id, 1000, 14);
    printAvg(session_id, 0, 16);
}
