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
} PlayerData;

// Function declarations
extern void update_player(void);
void send_player_data_to_portal(const char *player_data);
char *generate_player_json(const PlayerData *player_data);

#endif // PORTAL_PLAYER_H
