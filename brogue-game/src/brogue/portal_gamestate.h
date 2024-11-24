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
    int currentDepth;
    int deepestLevel;
} GameStateMetrics;

// Function declarations
extern void update_gamestate(void);
extern void generate_gamestate_json(const GameStateMetrics *gamestate, char *buffer, size_t size);

#endif // PORTAL_GAMESTATE_H
