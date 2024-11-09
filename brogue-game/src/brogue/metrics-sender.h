// K8S: Declares the functions to send metrics data to the Portal

#ifndef METRICS_SENDER_H
#define METRICS_SENDER_H

// Define the PlayerMetrics struct
typedef struct {
    // Numeric metrics
    int gold;
    int goldGenerated;
    int depthLevel;
    int deepestLevel;
    int currentHP;
    int maxHP;
    int turns;
    int strength;
    int monsterSpawnFuse;

    int playerTurnNumber;
    int absoluteTurnNumber;
    int milliseconds;
    int xpxpThisTurn;
    int stealthRange;
    int rewardRoomsGenerated;

    // Non-numeric metrics
    boolean wizard;
    boolean disturbed;
    boolean gameInProgress;
    boolean gameHasEnded;
    boolean easyMode;

    int seed;
    int RNG;

    // ring bonuses:
    int clairvoyance;
    int stealthBonus;
    int regenerationBonus;
    int lightMultiplier;
    int awarenessBonus;
    int transference;
    int wisdomBonus;
    int reaping;

    int regenPerTurn;
    int weaknessAmount;
    int poisonAmount; 

    // Equipped items
    item weapon;
    item armor;
    item ringLeft;
    item ringRight;

} Metrics;

// Declaration of the function to send metrics data to the Portal
void send_metrics_to_portal(const Metrics *metrics);
extern void update_metrics();

// Declaration of utility functions
const itemTable *getWeaponDetails(short kind);
const itemTable *getArmorDetails(short kind);
const itemTable *getRingDetails(short kind);
extern const char *getItemCategoryName(unsigned short category);
const char *getItemCategory(unsigned short category);

void extract_inventory_json(char *buffer, size_t buffer_size);

#endif // METRICS_SENDER_H
