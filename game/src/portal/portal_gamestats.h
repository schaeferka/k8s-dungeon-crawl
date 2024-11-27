#ifndef PORTAL_GAMESTATS_H
#define PORTAL_GAMESTATS_H

#include "Rogue.h"
#include "stdbool.h"
#include "MainMenu.h"

// Structure to hold game stats metrics
extern struct {
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
    int fewestTurnsWin;
    unsigned long cumulativeTurns;
    int longestWinStreak;
    int longestMasteryStreak;
    int currentWinStreak;
    int currentMasteryStreak;
} gameStatsPortal;

extern void update_gamestats(void);
extern void extractGameStatsJSON(const gameStats *stats, char *buffer, size_t buffer_size);
extern bool is_gamestats_changed(const gameStats *current_stats);

#endif // PORTAL_GAMESTATS_H
