#ifndef MAIN_MENU_H
#define MAIN_MENU_H

#include <pthread.h>  // Include pthread for the mutex
#include "portal-player.h"
#include "portal-monster.h"
#include "Rogue.h"

// Declaration of the function to start the metrics thread if needed
void start_metrics_thread_if_needed(void);

// Declaration of the function to sleep for a specified number of microseconds
void sleep_for_microseconds(int microseconds);

// Optionally, declare any shared variables like the mutex if it's needed outside this file
extern pthread_mutex_t metrics_thread_mutex;  // Make the mutex accessible if needed for other functions

void *metrics_update_loop(void *arg);

// Declare the metrics thread
static pthread_t metrics_thread;

extern void cleanup_game_resources();

typedef struct gameStats {
    int games;
    int escaped;
    int mastered;
    int won;
    float winRate;
    int deepestLevel;
    int cumulativeLevels;
    int highestScore;
    unsigned long cumulativeScore;
    int mostGold;
    unsigned long cumulativeGold;
    int mostLumenstones;
    int cumulativeLumenstones;
    int fewestTurnsWin; // zero means never won
    unsigned long cumulativeTurns;
    int longestWinStreak;
    int longestMasteryStreak;
    int currentWinStreak;
    int currentMasteryStreak;
} gameStats;

extern void addRuntoGameStats(rogueRun *run, gameStats *stats);

#endif // MAIN_MENU_H