// portal-gamestate.h

#ifndef PORTAL_GAMESTATE_H
#define PORTAL_GAMESTATE_H

#include "Rogue.h"
#include "stdbool.h"

// Structure to hold game state-related metrics
typedef struct {
    bool wizard;
    int rewardRoomsGenerated;
    int goldGenerated;
    bool gameInProgress;
    bool gameHasEnded;
    bool easyMode;
    int seed;
    int RNG;
    int absoluteTurnNumber;
    int milliseconds;
    int monsterSpawnFuse;
    int turns;
} GameStateMetrics;

// Function declarations
void update_gamestate(void);
void send_gamestate_metrics_to_portal(const GameStateMetrics *gamestate);

#endif // PORTAL_GAMESTATE_H
