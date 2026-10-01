#include "filehandling.h"
#include "gui.h"
#include <ncurses.h>

const bool hide_timer_during_solve = false;

/*
- +2, DNFs
- Generate scrambles
   - If making myself, need to create data structure to store cube, make moves, etc
   - Pipeline: Do random sequence of n moves -> run Kociemba -> return reversed solve sequence
- Display scrambled cube
- Ability to delete solves
- Explore various configs
- When resizing window, check to ensure it isn't too small
- Add help screen for providing shortcut actions
*/


int main(void) {
    initscr(); // Initialize the screen ("taking over the terminal")
    start_color(); // Enables color functionality
    use_default_colors(); // Makes -1 a valid background color (terminal background)
    noecho(); // Hides typed characters
    curs_set(0); //Hides the cursor
    timeout(1000); // Sets the max wait time for getch()

    init_pair(1, COLOR_RED, -1);
    init_pair(2, COLOR_MAGENTA, -1);
    init_pair(3, COLOR_YELLOW, -1);

    float timer = 0;
    int running = 1;
    const int avg_vals[] = {1, 3, 5, 12, 100, 100, 1000, 0};
    float times[7]; // Single, mo3, ao5, ao12, ao100, ao1000, savg

    if (!testForFile(1)) {
        printToCenter(1, "ERROR: Couldn't obtain session file, solve data won't be recorded.");
        refresh();
        delay_output(1000);
    }


    while (running) {
        clear(); // Clear the screen
        printToLineCenter(getmaxy(stdscr)/2 - 2, -1, "Cube Timer");

        // If there's a time to display, do so
        if (timer == -2) printToCenter(-1, "DNF");
        else if (timer != 0) printToCenter(-1, "%.3f", timer);
        else { printToCenter(-1, "0.000"); }

        printToLineCenter(getmaxy(stdscr)/2 + 2, -1, "Press 'q' to quit");
        printAllTimes(1);
        printAvgs(1);
        refresh();

        // Get input
        int ch = getch();

        // SPACE = Timer
        if (ch == ' ') {
            timer = runTimer(hide_timer_during_solve);
            if (timer >= 0) timer /= 1000;
            writeNewTime(1, timer);

            // Update averages display
            // printAvgs(1);
            // for (int i = 0; i < 7; i++) {
            //     float avg = getAvg(1, avg_vals[i]);
            // }
            handleBestTimes(1);
        }
        else if (ch == ERR) {
            // Do nothing
        }
        else if (ch == 'q') {
            running = 0;
        }
        else if (ch == 'r') {
            clearLine(getmaxy(stdscr)/2);
            float test = getTime(1, 3);
            if (test == -2) printToCenter(-2, "DNF");
            else printToCenter(-1, "%.3f", getTime(1, 3));
            refresh();
            delay_output(1000);
        }
        else if (ch == 't') {
            float tests[6];
            // tests[0] = getAvg(1, 3);
            tests[1] = getAvg(1, 5);
            printToLineCenter(0, -1, "%.3f", tests[1]);
            refresh();
            delay_output(1000);
            // tests[2] = getAvg(1, 12);
            // tests[3] = getAvg(1, 100);
            // tests[4] = getAvg(1, 1000);
            // tests[5] = getAvg(1, 0);
            // for (int i = 0; i < 6; i++) {
            //     printToLineCenter(0, -1, "%.3f", tests[i]);
            //     refresh();
            //     delay_output(1000);
            // }
        }
        else if (ch == 'c') {
            setBestTime(1, 5, 9.09);
        }
    }

    endwin(); // Closes program, returns to terminal
    return 0;
}
