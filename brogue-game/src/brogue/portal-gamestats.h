// portal-gamestate.h

#ifndef PORTAL_GAMESTATS_H
#define PORTAL_GAMESTATS_H

#include "Rogue.h"
#include "stdbool.h"

// Structure to hold gamestats-related metrics

typedef struct {
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
} GameStatsMetrics;

// Function declarations
void update_gamestats(void);
void send_gamestats_metrics_to_portal(const GameStatsMetrics *gamestats);

#endif // PORTAL_GAMESTATS_H
