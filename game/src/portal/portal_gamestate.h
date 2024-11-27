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
} GameStateData;

// Function declarations
extern void update_gamestate(void);
extern void generate_gamestate_json(const GameStateData *gamestate, char *buffer, size_t size);
extern bool is_gamestate_changed(void);

#endif // PORTAL_GAMESTATE_H
