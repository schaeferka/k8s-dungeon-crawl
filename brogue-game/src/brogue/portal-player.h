// portal-player.h

#ifndef PORTAL_PLAYER_H
#define PORTAL_PLAYER_H

#include "Rogue.h"
#include "stdbool.h"

// Structure to hold player-related metrics
typedef struct {
    int gold;
    int depthLevel;
    int deepestLevel;
    int currentHP;
    int maxHP;
    int strength;
    int playerTurnNumber;
    int xpxpThisTurn;
    int stealthRange;
    bool disturbed;
    int regenPerTurn;
    int weaknessAmount;
    int poisonAmount;

    // Ring modifiers
    int clairvoyance;
    int stealthBonus;
    int regenerationBonus;
    int lightMultiplier;
    int awarenessBonus;
    int transference;
    int wisdomBonus;
    int reaping;
} PlayerMetrics;

// Function declarations
void update_player(void);
void send_player_metrics_to_portal(const PlayerMetrics *metrics);

#endif // PORTAL_PLAYER_H
